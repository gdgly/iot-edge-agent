#include <azure_c_shared_utility/xlogging.h>
#include <azure_c_shared_utility/uuid.h>
#include <azure_c_shared_utility/strings.h>
#include <azure_c_shared_utility/threadapi.h>
#include "iotea_client.h"
#include "parson.h"

typedef struct IOTEA_CLIENT_TAG
{
    bool subscribed;
    time_t subscribeSentTimestamp;
    char* endpoint;
    char* name;
    IOTHUB_MQTT_CLIENT_HANDLE mqttClient;
    MQTT_CONNECTION_TYPE mqttConnType;
    PROTOCOL_CALLBACK callback;
} IOTEA_CLIENT;

static void ResetIotEaClient(IOTEA_CLIENT_HANDLE handle)
{
    if (NULL != handle)
    {
        handle->subscribed = false;
        handle->subscribeSentTimestamp = 0;
        handle->endpoint = NULL;
        handle->name = NULL;
        handle->mqttClient = NULL;
        handle->callback.modelParse = NULL;
        handle->callback.deviceParse = NULL;
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

static void OnRecvCallbackForModelParse(IOTEA_CLIENT_HANDLE handle, const char * json_string)
{
    (*(handle->callback.modelParse))(json_string);
}

static void OnRecvCallbackForDeviceParse(IOTEA_CLIENT_HANDLE handle, const char * json_string)
{
    (*(handle->callback.deviceParse))(json_string);
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

static int SendDeviceQueryReq(IOTEA_CLIENT_HANDLE handle, bool is_send_for_list)
{
    int result;
    char publishData[1024] = {'\0'};

    if (true == is_send_for_list)
    {
        sprintf(publishData, "{\"token\": \"1245sfvhjkklm\",\"fsn\":\"123\",\"flat\":\"flat\",\"timestamp\": \"2019-07-09T09:30:08.230Z\",\"body\":\"\"}");
        publish_mqtt_message(handle->mqttClient, DEVICE_LIST_PUB, DELIVER_AT_LEAST_ONCE, (const uint8_t*)publishData, strlen(publishData), NULL , NULL);
    }
    else
    {
        sprintf(publishData, "{\"token\": \"1245sfvhjkklm\",\"fsn\":\"123\",\"flat\":\"flat\",\"timestamp\": \"2019-07-09T09:30:08.230Z\",\"body\":[{\"dev\":\"%s\"}]}", handle->name);
        publish_mqtt_message(handle->mqttClient, DEVICE_INFO_PUB, DELIVER_AT_LEAST_ONCE, (const uint8_t*)publishData, strlen(publishData), NULL , NULL);
    }
    
    LogInfo("%s", publishData);
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

    STRING_HANDLE message = STRING_from_byte_array(appMsg->message, appMsg->length);

    if (NULL == message) {
        LOG(AZ_LOG_TRACE, LOG_LINE, "Received Model response: NULL Message"); 
        return;
    }

    // LogInfo("%s", STRING_c_str(message));

    if (strcmp(topic_name, MODEL_SUB_TOPIC) == 0) 
    {
        OnRecvCallbackForModelParse(eaHandle, STRING_c_str(message));
    }
    else if (strcmp(topic_name, DEVICE_INFO_SUB) == 0) 
    {
        OnRecvCallbackForDeviceParse(eaHandle, STRING_c_str(message));
    }
    else if (strcmp(topic_name, DEVICE_LIST_SUB) == 0) 
    {

        JSON_Value * root_value;

        root_value = json_parse_string(STRING_c_str(message));

        if (json_value_get_type(root_value) != JSONObject)
        {
            LogError("Not a JSON Object");
        }
        else
        {
            JSON_Object * device_list;
            JSON_Array  * body_array;
            JSON_Object * body_content;
            
            device_list = json_value_get_object(root_value);
            body_array  = json_object_dotget_array(device_list, "body");
            int body_arr_len = json_array_get_count(body_array);
            bool exist_in_list = false;

            for (int i = 0; i < body_arr_len; i++)
            {
                body_content = json_array_get_object(body_array, i);
                const char * device_name = json_object_dotget_string(body_content, "dev");
                if (strcmp(device_name, eaHandle->name) == 0)
                    exist_in_list = true;
            }
            if (true == exist_in_list)
            {
                SendDeviceQueryReq(eaHandle, false);
            }
            else
            {
                LogError("%s Not Exist", eaHandle->name);
            }
        }
        json_value_free(root_value);
    }
    else 
    {
        LogError("Unknown Topic \n");
    }
    STRING_delete(message);
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
    SendDeviceQueryReq(handle, true);
    // SendModelReq(handle);
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
    subscribe[index++] = DEVICE_INFO_SUB;
    subscribe[index++] = DEVICE_LIST_SUB;

    return index;
}

IOTHUB_MQTT_CLIENT_HANDLE get_mqtt_client_handle(IOTEA_CLIENT_HANDLE handle)
{
    return handle->mqttClient;
}

void iotea_client_register_model_parse(IOTEA_CLIENT_HANDLE handle, MODEL_PARSE_CALLBACK callback)
{
    handle->callback.modelParse = callback;
}

void iotea_client_register_device_parse(IOTEA_CLIENT_HANDLE handle, DEVICE_PARSE_CALLBACK callback)
{
    handle->callback.deviceParse = callback;
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
    // handle->protoManager = protocol_init(MODBUS_RTU);
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