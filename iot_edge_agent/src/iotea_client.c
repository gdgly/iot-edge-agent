/*
* Copyright(c) 2020, Works Systems, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
* 3. Neither the name of the vendors nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <azure_c_shared_utility/xlogging.h>
#include <azure_c_shared_utility/uuid.h>
#include <azure_c_shared_utility/strings.h>
#include <azure_c_shared_utility/threadapi.h>
#include "iotea_client.h"
#include "parson.h"

typedef struct IOTEA_CLIENT_TAG
{
    bool subscribed;                // 是否订阅
    time_t subscribeSentTimestamp;  // 订阅时间

    char *endpoint;                 // 服务器地址
    char *devname;                  // 设备名字
    char *proType;                  // 协议类型

    bool reqAns;                    // 是否响应
    size_t reqCount;                // 请求次数

    tickcounter_ms_t lastReqSendTime;

    TICK_COUNTER_HANDLE reqTickCounter;
    //  mqtt client 实例
    IOTHUB_MQTT_CLIENT_HANDLE mqttClient;

    // mqtt 连接状态
    MQTT_CONNECTION_TYPE mqttConnType;

    // 模型解析的回调函数
    MODEL_PARSE_CALLBACK callback;
} IOTEA_CLIENT;

/*************************************************************************
*函数名 : get_broker_endpiont 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 获取服务器地址
*输入参数 : char* broker 服务器地址
           MQTT_CONNECTION_TYPE* mqttConnType   连接类型
*输出参数 : 
*返回值: char *
*调用关系 :
*其它:
*************************************************************************/
static char* get_broker_endpiont(char *broker, MQTT_CONNECTION_TYPE *mqttConnType)
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
    char *endpoint = malloc(sizeof(char) * length);
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

/*************************************************************************
*函数名 : generate_topic 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 生成topic
*输入参数 : const char* format 格式
           const char* device 设备
*输出参数 : 
*返回值: char *
*调用关系 :
*其它:
*************************************************************************/
static char* generate_topic(const char *format, const char *device)
{
    if (NULL == format || NULL == device)
    {
        LogError("Failure: both format and device should not be NULL.");
        return NULL;
    }
    size_t size = strlen(format) + strlen(device) + 1;
    char *topic = malloc(sizeof(char) * size);
    if (NULL == topic) return NULL;
    if (sprintf_s(topic, size, format, device) < 0)
    {
        free(topic);
        topic = NULL;
    }

    return topic;
}

/*************************************************************************
*函数名 : callback_for_thing_model_parse 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 绑定物模型解析回调函数
*输入参数 : IOTEA_CLIENT_HANDLE handle 
           const char * json_string
*输出参数 : 
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static int callback_for_thing_model_parse(IOTEA_CLIENT_HANDLE handle, const char *json_string)
{
    return (*(handle->callback.ThingModelParse))(json_string);
}

/*************************************************************************
*函数名 : callback_for_channel_model_parse 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 绑定通道模型回调函数
*输入参数 : IOTEA_CLIENT_HANDLE handle 
           const char * json_string
*输出参数 : 
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static int callback_for_channel_model_parse(IOTEA_CLIENT_HANDLE handle, const char *json_string)
{
    return (*(handle->callback.ChannelModelParse))(json_string);
}

/*************************************************************************
*函数名 : send_model_query_req 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 模型请求
*输入参数 : IOTEA_CLIENT_HANDLE handle 
*输出参数 : 
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
static int send_model_query_req(IOTEA_CLIENT_HANDLE handle)
{
    int result;

    char *publish_data = "{}";
    char temp_topic[64 + 1] = {0};
    snprintf(temp_topic, 64, MODEL_PUB_TOPIC, handle->proType);

    int res = publish_mqtt_message(handle->mqttClient, temp_topic, DELIVER_AT_LEAST_ONCE, 
        (const uint8_t*)publish_data, strlen(publish_data), NULL , NULL);

    if (res != 0)
    {
        result = __FAILURE__;
    }
    else 
    {
        result = 0;
    }

    return result;
}

/*************************************************************************
*函数名 : send_device_query_req 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 模型请求
*输入参数 : IOTEA_CLIENT_HANDLE handle bool is_send_for_list
*输出参数 : 
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
static int send_device_query_req(IOTEA_CLIENT_HANDLE handle, bool is_send_for_list)
{
    int result = 0;

    char publish_data[1024] = {0};
    char *pub_topic;

    if (true == is_send_for_list)
    {
        snprintf(publish_data, 1024, "{\"token\": \"1245sfvhjkklm\",\"fsn\": \"123\",\
            \"flat\": \"123\",\"timestamp\": \"2019-07-09T09:30:08.230Z\",\"body\":\"\"}");
        pub_topic = generate_topic(DEVICE_LIST_PUB, handle->proType);
        publish_mqtt_message(handle->mqttClient, pub_topic, DELIVER_AT_LEAST_ONCE, 
            (const uint8_t*)publish_data, strlen(publish_data), NULL , NULL);
    }
    else
    {
        snprintf(publish_data, 1024, "{\"token\": \"1245sfvhjkklm\",\"fsn\": \"123\",\
            \"flat\": \"123\",\"timestamp\": \"2019-07-09T09:30:08.230Z\",\
            \"body\":[{\"dev\":\"%s\"}]}", handle->devname);
        pub_topic = generate_topic(DEVICE_INFO_PUB, handle->proType);
        publish_mqtt_message(handle->mqttClient, pub_topic, DELIVER_AT_LEAST_ONCE, 
            (const uint8_t*)publish_data, strlen(publish_data), NULL , NULL);
    }

    free(pub_topic);
    pub_topic = NULL;

    // LogInfo("%s", publish_data);
    return result;
}

/*************************************************************************
*函数名 : on_recv_callback 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 接收到数据的回调
*输入参数 : MQTT_MESSAGE_HANDLE msgHandle 消息句柄
            void* context 上下文对象
*输出参数 : 
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static void on_recv_callback(MQTT_MESSAGE_HANDLE msg_handle, void* context)
{
    const char *topic_name = mqttmessage_getTopicName(msg_handle);
    const APP_PAYLOAD *app_msg = mqttmessage_getApplicationMsg(msg_handle);

    if (NULL == topic_name || NULL == app_msg)
    {
        LogError("Failure: cannot find topic or app_msg in the message received.");
        return;
    }

    IOTHUB_MQTT_CLIENT_HANDLE mqtt_client = (IOTHUB_MQTT_CLIENT_HANDLE)context;
    IOTEA_CLIENT_HANDLE ea_handle = (IOTEA_CLIENT_HANDLE)mqtt_client->callbackContext;

    // LOG(AZ_LOG_TRACE, LOG_LINE, "Received response: %s", topic_name);

    STRING_HANDLE message = STRING_from_byte_array(app_msg->message, app_msg->length);

    if (NULL == message)
    {
        LOG(AZ_LOG_TRACE, LOG_LINE, "Received Model response: NULL Message");
        return;
    }

    // LogInfo("%s", STRING_c_str(message));
    
    if (topic_name[strlen(topic_name) - 1] == 'l')
    {
        int res = callback_for_thing_model_parse(ea_handle, STRING_c_str(message));

        if (res != 0)
        {
            LogError("Thing Model Parse Failure");
        }
    }
    else if (topic_name[strlen(topic_name) - 1] == 'v')
    {
        int res = callback_for_channel_model_parse(ea_handle, STRING_c_str(message));

        if (res == 0)
        {
            LogInfo("request for iot model");
            send_model_query_req(ea_handle);
        }
        else
        {
            LogError("Channel Model Parse Failure");
        }
    }
    else if (topic_name[strlen(topic_name) - 1] == 't')
    {

        JSON_Value *root_value;

        root_value = json_parse_string(STRING_c_str(message));

        if (json_value_get_type(root_value) != JSONObject)
        {
            LogError("Not a JSON Object");
        }
        else
        {
            JSON_Object *device_list;
            JSON_Array  *body_array;
            JSON_Object *body_content;

            device_list = json_value_get_object(root_value);
            body_array  = json_object_dotget_array(device_list, "body");
            int body_arr_len = json_array_get_count(body_array);
            bool exist_in_list = false;

            for (int i = 0; i < body_arr_len; i++)
            {
                body_content = json_array_get_object(body_array, i);
                const char *device_name = json_object_dotget_string(body_content, "dev");
                if (strcmp(device_name, ea_handle->devname) == 0)
                    exist_in_list = true;
            }

            
            if (true == exist_in_list)
            {
                send_device_query_req(ea_handle, false);
            }
            else
            {
                LogError("%s Not Exist", ea_handle->devname);
            }
        }
        json_value_free(root_value);
    }
    else
    {
        // LogError("Unknown Topic \n");
    }

    ea_handle->reqAns = true;

    STRING_delete(message);
}

/*************************************************************************
*函数名 : on_sub_ack_callback 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 订阅topic的回调
*输入参数 : QOS_VALUE* qos_return qos信息
            size_t qosCount     qos数
            void* context 上下文对象
*输出参数 : 
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static int on_sub_ack_callback(QOS_VALUE *qos_return, size_t qosCount, void *context)
{
    for (int i = 0; i < qosCount; ++i)
    {
        if (qos_return[i] == DELIVER_FAILURE)
        {
            LogError("Failed to subscribe");
            return 0;
        }
    }
    IOTEA_CLIENT_HANDLE handle = context;
    handle->subscribed = true;
    LogInfo("Subscribed topics");

    LogInfo("request for iot channel");
    tickcounter_get_current_ms(handle->reqTickCounter, &handle->lastReqSendTime);
    send_device_query_req(handle, true);
    return 0;
}


/*************************************************************************
*函数名 : reset_subscription 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 重置需要订阅的topic
*输入参数 : char** subscribe topic
            size_t length     topic数
*输出参数 : 
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static void reset_subscription(char **subscribe, size_t length)
{
    if (NULL == subscribe) return;
    for (size_t index = 0; index < length; ++index)
    {
        subscribe[index] = NULL;
    }
}

/*************************************************************************
*函数名 : release_subscription 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 释放订阅的topic
*输入参数 : char** subscribe topic
            size_t length     topic数
*输出参数 : 
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static void release_subscription(char **subscribe, size_t length)
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

/*************************************************************************
*函数名 : get_subscription 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 获取订阅topic
*输入参数 : IOTEA_CLIENT_HANDLE handle
           char** subscribe    topic
           size_t length     topic数
*输出参数 : 
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
static int get_subscription(IOTEA_CLIENT_HANDLE handle, char **subscribe, size_t length)
{
    if (NULL == subscribe) return 0;

    size_t index = 0;
    reset_subscription(subscribe, length);

    char *sub_topic;
    
    sub_topic = generate_topic(MODEL_SUB_TOPIC, handle->proType);
    subscribe[index++] = sub_topic;
    sub_topic = generate_topic(DEVICE_INFO_SUB, handle->proType);
    subscribe[index++] = sub_topic;
    sub_topic = generate_topic(DEVICE_LIST_SUB, handle->proType);
    subscribe[index++] = sub_topic;

    return index;
}

/*************************************************************************
*函数名 : get_mqtt_client_handle 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 获取mqtt handle
*输入参数 : IOTEA_CLIENT_HANDLE handle
*输出参数 : 
*返回值: IOTHUB_MQTT_CLIENT_HANDLE
*调用关系 :
*其它:
*************************************************************************/
IOTHUB_MQTT_CLIENT_HANDLE get_mqtt_client_handle(IOTEA_CLIENT_HANDLE handle)
{
    return handle->mqttClient;
}

/*************************************************************************
*函数名 : init_iotea_client 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 初始化iotea客户端
*输入参数 : IOTEA_CLIENT_HANDLE handle
           const IOTEA_CLIENT_OPTIONS* options 初始化参数
*输出参数 : 
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
static void init_iotea_client(IOTEA_CLIENT_HANDLE handle, const IOTEA_CLIENT_OPTIONS *options)
{

    MQTT_CLIENT_OPTIONS mqtt_client_options;
    mqtt_client_options.clientId = (options->clientId == NULL) ? handle->devname : options->clientId;
    mqtt_client_options.willTopic = NULL;
    mqtt_client_options.willMessage = NULL;
    mqtt_client_options.username = options->username;
    mqtt_client_options.password = options->password;
    mqtt_client_options.keepAliveInterval = options->keepAliveInterval > 0 ? options->keepAliveInterval : 10;
    mqtt_client_options.messageRetain = false;
    mqtt_client_options.useCleanSession = options->cleanSession;
    mqtt_client_options.qualityOfServiceValue = DELIVER_AT_LEAST_ONCE;

    handle->mqttClient = initialize_mqtt_client_handle(
        &mqtt_client_options,
        handle->endpoint,
        handle->mqttConnType,
        on_recv_callback,
        IOTHUB_CLIENT_RETRY_EXPONENTIAL_BACKOFF_WITH_JITTER,
        options->retryTimeoutInSeconds < 300 ? 300 : options->retryTimeoutInSeconds);
}

/*************************************************************************
*函数名 : reset_iotea_client 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 重置IOTEA客户端
*输入参数 : IOTEA_CLIENT_HANDLE handle
*输出参数 : 
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static void reset_iotea_client(IOTEA_CLIENT_HANDLE handle)
{
    if (NULL != handle)
    {
        handle->subscribed = false;
        handle->subscribeSentTimestamp = 0;
        handle->endpoint = NULL;
        handle->devname = NULL;
        handle->proType = NULL;
        handle->reqAns = false;
        handle->lastReqSendTime = 0;
        handle->reqCount = 0;
        handle->mqttClient = NULL;
        handle->callback.ThingModelParse = NULL;
        handle->callback.ChannelModelParse = NULL;
    }
}

/*************************************************************************
*函数名 : iotea_client_init 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 初始化iotea客户端
*输入参数 : char* broker 服务器地址
           char* name 设备名字
*输出参数 : 
*返回值: IOTEA_CLIENT_HANDLE
*调用关系 :
*其它:
*************************************************************************/
IOTEA_CLIENT_HANDLE iotea_client_init(char *broker, char *name, char *type)
{
    if (NULL == broker || NULL == name || NULL == type)
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

    reset_iotea_client(handle);
    handle->endpoint = get_broker_endpiont(broker, &(handle->mqttConnType));
    if (NULL == handle->endpoint)
    {
        LogError("Failure: get the endpoint from broker address.");
        free(handle);
        handle = NULL;
        return handle;
    }

    if ((handle->reqTickCounter = tickcounter_create()) == NULL)
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "fail to create tickCounter");
        free(handle);
        handle = NULL;
        return handle;
    }

    handle->devname = name;
    handle->proType = type;

    return handle;
}

/*************************************************************************
*函数名 : iotea_client_init 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 初始化iotea客户端
*输入参数 : IOTEA_CLIENT_HANDLE handle
           const IOTEA_CLIENT_OPTIONS* options 初始化参数
*输出参数 : 
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
void iotea_client_deinit(IOTEA_CLIENT_HANDLE handle)
{
    if (NULL != handle)
    {
        size_t topic_size = TOPIC_SIZE;
        char **topics = malloc(topic_size * sizeof(char*));
        if (topics == NULL)
        {
            LogError("Failure: failed to alloc");
            return;
        }
        int amount = get_subscription(handle, topics, topic_size);
        if (amount < 0)
        {
            LogError("Failure: failed to get the subscribing topics.");
        }
        else if (amount > 0)
        {
            unsubscribe_mqtt_topics(handle->mqttClient, (const char**) topics, amount);
            release_subscription(topics, topic_size);
        }
        free(topics);
        topics = NULL;

        iothub_mqtt_destroy(handle->mqttClient);
        if (NULL != handle->endpoint)
        {
            free(handle->endpoint);
            handle->endpoint = NULL;
        }

        tickcounter_destroy(handle->reqTickCounter);
        reset_iotea_client(handle);
        free(handle);
        handle = NULL;
    }
}

/*************************************************************************
*函数名 : iotea_client_connect 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 连接mqtt服务端
*输入参数 : IOTEA_CLIENT_HANDLE handle
           IOTEA_CLIENT_OPTIONS *options
*输出参数 : 
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
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

    init_iotea_client(handle, options);
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
    } while (MQTT_CLIENT_STATUS_CONNECTED != handle->mqttClient->mqttClientStatus 
        && handle->mqttClient->isRecoverableError);

    handle->mqttClient->isDestroyCalled = false;
    handle->mqttClient->isDisconnectCalled = false;
    return 0;
}

/*************************************************************************
*函数名 : iotea_client_dowork 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 处理mqtt IO数据
*输入参数 : const IOTEA_CLIENT_HANDLE handle
*输出参数 : 
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
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
        if (elipsed > 10)
        {
            size_t topic_size = TOPIC_SIZE;
            char **topics = malloc(topic_size * sizeof(char*));
            if (topics == NULL)
            {
                LogError("Failure: failed to alloc");
                return __FAILURE__;
            }

            int amount = get_subscription(handle, topics, topic_size);
            if (amount < 0)
            {
                LogError("Failure: failed to get the subscribing topics.");
                free(topics);
                topics = NULL;
                return __FAILURE__;
            }
            else if (amount > 0)
            {
                SUBSCRIBE_PAYLOAD *subscribe = malloc(topic_size * sizeof(SUBSCRIBE_PAYLOAD));
                for (size_t index = 0; index < (size_t)amount; ++index)
                {
                    subscribe[index].subscribeTopic = topics[index];
                    subscribe[index].qosReturn = DELIVER_AT_LEAST_ONCE;
                }
                int result = subscribe_mqtt_topics(handle->mqttClient, subscribe, amount, on_sub_ack_callback, handle);
                if (result == 0)
                {
                    handle->subscribeSentTimestamp = time(NULL);
                }

                release_subscription(topics, topic_size);
                free(subscribe);
                subscribe = NULL;
                free(topics);
                topics = NULL;
                if (0 != result)
                {
                    LogError("Failure: failed to subscribe the topics.");
                    return __FAILURE__;
                }
            }
        }
    }

    tickcounter_ms_t currentTime; 
    tickcounter_get_current_ms(handle->reqTickCounter, &currentTime);
    
    if ((currentTime - handle->lastReqSendTime) / 1000 > 3 && handle->reqAns == false && 
        handle->reqCount < 3)
    {
        LogInfo("request for iot channel again");
        send_device_query_req(handle, true);
        handle->lastReqSendTime = currentTime;
        handle->reqCount++;
    }

    iothub_mqtt_dowork(handle->mqttClient);

    return 0;
}

/*************************************************************************
*函数名 : iotea_client_register_thing_model_init 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 注册物模型解析函数的回调
*输入参数 : IOTEA_CLIENT_HANDLE handle iotea 实例
            THING_MODEL_PARSE_CALLBACK callback 物模型解析回调函数指针
*输出参数 : 
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
void iotea_client_register_thing_model_init(IOTEA_CLIENT_HANDLE handle, THING_MODEL_PARSE_CALLBACK callback)
{
    handle->callback.ThingModelParse = callback;
}

/*************************************************************************
*函数名 : iotea_client_register_channel_model_init 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 注册通道模型解析函数的回调
*输入参数 : IOTEA_CLIENT_HANDLE handle iotea 实例
            THING_MODEL_PARSE_CALLBACK callback 通道模型解析回调函数指针
*输出参数 : 
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
void iotea_client_register_channel_model_init(IOTEA_CLIENT_HANDLE handle, CHANNEL_MODEL_PARSE_CALLBACK callback)
{
    handle->callback.ChannelModelParse = callback;
}
