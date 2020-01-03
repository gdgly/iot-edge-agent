#include <azure_c_shared_utility/xlogging.h>
#include <azure_c_shared_utility/uuid.h>
#include <azure_c_shared_utility/strings.h>
#include <azure_c_shared_utility/threadapi.h>
#include "iot_edge_agent.h"

static void ResetIotEaClient(IOTEA_CLIENT_HANDLE handle)
{
    if (NULL != handle)
    {
        handle->subscribed = false;
        handle->subscribeSentTimestamp = 0;
        handle->endpoint = NULL;
        handle->name = NULL;
        handle->mqttClient = NULL;
        handle->callback.modbus = NULL;
        handle->callback.modelParse = NULL;
    }
}

static char* GetBrokerEndpoint(char* broker, MQTT_CONNECTION_TYPE* mqttConnType)
{
    if ('t' == broker[0] && 'c' == broker[1] && 'p' == broker[2])
    {
        *mqttConnType = MQTT_CONNECTION_TCP;
    }
    else if ('s' == broker[0] && 's' == broker[1] && 'l' == broker[2])
    {
        *mqttConnType = MQTT_CONNECTION_TLS;
    }
    else
    {
        LogError("Failure: the prefix of the broker address is incorrect.");
        return NULL;
    }

    size_t head = 0;
    size_t end = 0;
    for (size_t pos = 0; '\0' != broker[pos]; ++pos)
    {
        if (':' == broker[pos])
        {
            end = head == 0 ? end : pos;
            // For head, should skip the substring "//".
            head = head == 0 ? pos + 3 : head;
        }
    }
    if (head == 0 || end <= head)
    {
        LogError("Failure: cannot get the endpoint from broker address.");
        return NULL;
    }

    size_t length = end - head + 1;
    char* endpoint = malloc(sizeof(char) * length);
    if (NULL == endpoint)
    {
        LogError("Failure: cannot init the memory for endpoint.");
        return NULL;
    }

    endpoint[length - 1] = '\0';
    for (size_t pos = 0; pos < length - 1; ++pos)
    {
        endpoint[pos] = broker[head + pos];
    }

    return endpoint;
}

static void OnRecvCallbackForModbus(IOTEA_CLIENT_HANDLE handle, IOTHUB_MQTT_CLIENT * mqttClient, PORT_RESPONSE * port_response)
{
    (*(handle->callback.modbus))(mqttClient, port_response);
}

static void OnRecvCallbackForModelParse(IOTEA_CLIENT_HANDLE handle, const char * json_string)
{
    (*(handle->callback.modelParse))(json_string);
}

static int SendModelReq(IOTEA_CLIENT_HANDLE handle)
{
    int result;

    char * publishData = "{}";

    int res = publish_mqtt_message(handle->mqttClient, MODEL_PUB_TOPIC, DELIVER_AT_LEAST_ONCE, (const uint8_t*)publishData, strlen(publishData), NULL , NULL);

    if (res == 0) {
        LogInfo("request for iot model");
        result = 0;
    } else {
        result = -1;
    }

    // 定时检查是否回复

    return result;
}

static void OnRecvCallback(MQTT_MESSAGE_HANDLE msgHandle, void* context)
{
    const char* topic_name = mqttmessage_getTopicName(msgHandle);
    const APP_PAYLOAD* appMsg = mqttmessage_getApplicationMsg(msgHandle);

    if (NULL == topic_name || NULL == appMsg)
    {
        LogError("Failure: cannot find topic or appMsg in the message received.");
        return;
    }

    IOTHUB_MQTT_CLIENT_HANDLE mqttClient = (IOTHUB_MQTT_CLIENT_HANDLE)context;
    IOTEA_CLIENT_HANDLE eaHandle = (IOTEA_CLIENT_HANDLE)mqttClient->callbackContext;

    LOG(AZ_LOG_TRACE, LOG_LINE, "Received response: %s", topic_name);

    if (strcmp(topic_name, MODEL_SUB_TOPIC) == 0) {

        STRING_HANDLE message = STRING_from_byte_array(appMsg->message, appMsg->length);

        if (message != NULL) {
            OnRecvCallbackForModelParse(eaHandle, STRING_c_str(message));
            STRING_delete(message);
        } else {
            LOG(AZ_LOG_TRACE, LOG_LINE, "Received Model response: NULL Message");
        }

    } else if (strcmp(topic_name, SERIAL_SUB_TOPIC) == 0) {

        PORT_RESPONSE * port_response = (PORT_RESPONSE *)appMsg->message;
        OnRecvCallbackForModbus(eaHandle, mqttClient, port_response);

    } else if (strcmp(topic_name, DATA_SUB_TOPIC) == 0) {

    } else {
        LogError("other topic \n");
    }
}

static int OnSubAckCallback(QOS_VALUE* qosReturn, size_t qosCount, void *context)
{
    for (int i = 0; i < qosCount; ++i)
    {
        if (qosReturn[i] == DELIVER_FAILURE)
        {
            LogError("Failed to subscribe");
            return 0;
        }
    }
    IOTEA_CLIENT_HANDLE handle = context;
    handle->subscribed = true;
    LogInfo("Subscribed topics");
    SendModelReq(handle);
    return 0;
}

static void InitIotHubClient(IOTEA_CLIENT_HANDLE handle, const IOTEA_CLIENT_OPTIONS* options) {

    MQTT_CLIENT_OPTIONS mqttClientOptions;
    mqttClientOptions.clientId = options->clientId == NULL ? handle->name : options->clientId;
    mqttClientOptions.willTopic = NULL;
    mqttClientOptions.willMessage = NULL;
    mqttClientOptions.username = options->username;
    mqttClientOptions.password = options->password;
    mqttClientOptions.keepAliveInterval = options->keepAliveInterval > 0 ? options->keepAliveInterval : 10;
    mqttClientOptions.messageRetain = false;
    mqttClientOptions.useCleanSession = options->cleanSession;
    mqttClientOptions.qualityOfServiceValue = DELIVER_AT_LEAST_ONCE;

    handle->mqttClient = initialize_mqtt_client_handle(
        &mqttClientOptions,
        handle->endpoint,
        handle->mqttConnType,
        OnRecvCallback,
        IOTHUB_CLIENT_RETRY_EXPONENTIAL_BACKOFF_WITH_JITTER,
        options->retryTimeoutInSeconds < 300 ? 300 : options->retryTimeoutInSeconds);
}

static void ResetSubscription(char** subscribe, size_t length)
{
    if (NULL == subscribe) return;
    for (size_t index = 0; index < length; ++index)
    {
        subscribe[index] = NULL;
    }
}

static void ReleaseSubscription(char** subscribe, size_t length)
{
    if (NULL == subscribe) return;
    for (size_t index = 0; index < length; ++index)
    {
        if (NULL != subscribe[index])
        {
            free(subscribe[index]);
            subscribe[index] = NULL;
        }
    }
}

static int GetSubscription(IOTEA_CLIENT_HANDLE handle, char** subscribe, size_t length)
{
    if (NULL == subscribe) return 0;

    size_t index = 0;
    ResetSubscription(subscribe, length);

    subscribe[index++] = DATA_SUB_TOPIC;
    subscribe[index++] = MODEL_SUB_TOPIC;
    subscribe[index++] = SERIAL_SUB_TOPIC;

    return index;
}

void iotea_client_register_modbus(IOTEA_CLIENT_HANDLE handle, PROTOCOL_MODBUS_CALLBACK callback)
{
    handle->callback.modbus = callback;
}

void iotea_client_register_model_parse(IOTEA_CLIENT_HANDLE handle, MODEL_PARSE_CALLBACK callback)
{
    handle->callback.modelParse = callback;
}

IOTEA_CLIENT_HANDLE iotea_client_init(char* broker, char* name)
{
    if (NULL == broker || NULL == name)
    {
        LogError("Failure: parameters broker and name should not be NULL.");
        return NULL;
    }

    IOTEA_CLIENT_HANDLE handle = malloc(sizeof(IOTEA_CLIENT));
    if (NULL == handle)
    {
        LogError("Failure: init memory for iotea client.");
        return NULL;
    }

    ResetIotEaClient(handle);
    handle->endpoint = GetBrokerEndpoint(broker, &(handle->mqttConnType));
    if (NULL == handle->endpoint)
    {
        LogError("Failure: get the endpoint from broker address.");
        free(handle);
        return NULL;
    }
    handle->name = name;
    handle->protoManager = protocol_init(MODBUS_RTU);
    return handle;
}

void iotea_client_deinit(IOTEA_CLIENT_HANDLE handle)
{
    if (NULL != handle)
    {
        size_t topicSize = TOPIC_SIZE;
        char** topics = malloc(topicSize * sizeof(char*));
        if (topics == NULL)
        {
            LogError("Failure: failed to alloc");
            return;
        }
        int amount = GetSubscription(handle, topics, topicSize);
        if (amount < 0)
        {
            LogError("Failure: failed to get the subscribing topics.");
        }
        else if (amount > 0)
        {
            unsubscribe_mqtt_topics(handle->mqttClient, (const char**) topics, amount);
            ReleaseSubscription(topics, topicSize);
        }
        free(topics);

        iothub_mqtt_destroy(handle->mqttClient);
        if (NULL != handle->endpoint)
        {
            free(handle->endpoint);
        }

        ResetIotEaClient(handle);
        free(handle);
    }
}

int iotea_client_connect(IOTEA_CLIENT_HANDLE handle, const IOTEA_CLIENT_OPTIONS *options)
{
    if (NULL == handle)
    {
        LogError("IOTEA_CLIENT_HANDLE handle should not be NULL.");
        return __FAILURE__;
    }
    if (NULL == options || NULL == options->username || NULL == options->password)
    {
        LogError("Failure: the username and password in options should not be NULL.");
        return __FAILURE__;
    }

    InitIotHubClient(handle, options);
    if (NULL == handle->mqttClient)
    {
        LogError("Failure: cannot initialize the mqtt connection.");
        return __FAILURE__;
    }

    handle->mqttClient->callbackContext = handle;
    do
    {
        iothub_mqtt_dowork(handle->mqttClient);
        ThreadAPI_Sleep(10);
    } while (MQTT_CLIENT_STATUS_CONNECTED != handle->mqttClient->mqttClientStatus && handle->mqttClient->isRecoverableError);

    handle->mqttClient->isDestroyCalled = false;
    handle->mqttClient->isDisconnectCalled = false;
    return 0;
}

int iotea_client_dowork(const IOTEA_CLIENT_HANDLE handle)
{
    if (handle->mqttClient->isDestroyCalled || handle->mqttClient->isDisconnectCalled)
    {
        return -1;
    }

    if (handle->mqttClient->isConnectionLost)
    {
        handle->subscribed = false;
        handle->subscribeSentTimestamp = 0;
    }

    if (handle->mqttClient->mqttClientStatus == MQTT_CLIENT_STATUS_CONNECTED && !(handle->subscribed))
    {
        time_t current = time(NULL);

        double elipsed = difftime(current, handle->subscribeSentTimestamp);
        if (elipsed > 10) {
            size_t topicSize = TOPIC_SIZE;
            char** topics = malloc(topicSize * sizeof(char*));
            if (topics == NULL)
            {
                LogError("Failure: failed to alloc");
                return __FAILURE__;
            }

            int amount = GetSubscription(handle, topics, topicSize);
            if (amount < 0)
            {
                LogError("Failure: failed to get the subscribing topics.");
                free(topics);
                return __FAILURE__;
            }
            else if (amount > 0)
            {
                SUBSCRIBE_PAYLOAD* subscribe = malloc(topicSize * sizeof(SUBSCRIBE_PAYLOAD));
                for (size_t index = 0; index < (size_t)amount; ++index)
                {
                    subscribe[index].subscribeTopic = topics[index];
                    subscribe[index].qosReturn = DELIVER_AT_LEAST_ONCE;
                }
                int result = subscribe_mqtt_topics(handle->mqttClient, subscribe, amount, OnSubAckCallback, handle);
                if (result == 0)
                {
                    handle->subscribeSentTimestamp = time(NULL);
                }
                // ReleaseSubscription(topics, topicSize);
                free(subscribe);
                free(topics);
                if (0 != result)
                {
                    LogError("Failure: failed to subscribe the topics.");
                    return __FAILURE__;
                }
            }
        }
    }

    iothub_mqtt_dowork(handle->mqttClient);

    return 0;
}
