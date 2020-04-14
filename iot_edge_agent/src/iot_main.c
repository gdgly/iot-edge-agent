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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
// #include <bits/sigaction.h>

#include <azure_c_shared_utility/xlogging.h>
#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/threadapi.h>
#include <azure_c_shared_utility/buffer_.h>
#include <azure_c_shared_utility/tickcounter.h>
#include <azure_c_shared_utility/singlylinkedlist.h>
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/socketio.h"

#include "iot_main.h"
#include "parson.h"
#include "modbus.h"
#include "serial_io.h"
#include "iotea_client.h"
#include "channel_model.h"
#include "protocol_manager.h"
#include "utils.h"

#define VERSION         "1.0"

#define BROKER          "tcp://127.0.0.1:1883"
#define USERNAME        "test"
#define PASSWORD        "hahaha"


#define PROTO_TYPE       "DLT698.45"
#define DEVICE_NAME      "DLT698_Meter"

// #define PROTO_TYPE       "MODBUS"
// #define DEVICE_NAME      "Temp_Humi"

// iot edge agent handle
IOTEA_CLIENT_HANDLE g_iotea_handle = NULL;

// map link list
SINGLYLINKEDLIST_HANDLE g_pm_map_list = NULL;

// thread count
int g_thread_count = 0;

// signal happen flag
bool g_sigint_happen = false;

// is channel parse error
bool g_channel_parse_error = false;

// options
static const struct option long_options[] =
{
    {"devicename", required_argument, NULL, 'd'},
    {"prototype", required_argument, NULL, 'P'},
    {"username", required_argument, NULL, 'u'},
    {"password", required_argument, NULL, 'p'},
    {"host", required_argument, NULL, 'H'},
    {"help", no_argument,NULL,'?'},
    {"version",no_argument,NULL,'v'},
    {NULL, 0, NULL, 0}
};

/*************************************************************************
*函数名 : usage
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 帮助
*输入参数 : void
*输出参数 :
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static void usage_help(void)
{
    fprintf(stderr,
        "  用法  [选项]  [参数]  Provide args for IOT eage agent run up \n"
        "  -d, --devicename      Input device name for pair device model\n"
        "  -P, --prototype       Select from DLT645 DLT698.45 MODBUS\n"
        "  -u, --username        Username is used for mqtt login.\n"
        "  -p, --password        Password is used for mqtt login.\n"
        "  -H, --host            Host include host ip and port.\n"
        "  -h, --help            Show this help information.\n"
        "  -v, --version         Display program version.\n"
    );
}

/*************************************************************************
*函数名 : on_send_complete
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 发送完成后的回调函数
*输入参数 : void* context 上下文对象
            IO_SEND_RESULT send_result 发送结果
*输出参数 :
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static void on_send_complete(void *context, IO_SEND_RESULT send_result)
{
    (void)context;
    (void)send_result;
}

/*************************************************************************
*函数名 : on_io_open_complete
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : io打开的回调函数
*输入参数 : void* context 上下文对象
            IO_OPEN_RESULT open_result io打开结果
*输出参数 :
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static void on_io_open_complete(void *context, IO_OPEN_RESULT open_result)
{
    (void)context, (void)open_result;

    if (open_result == IO_OPEN_OK)
    {
        LogInfo("Channel Open Succ");
    }
    else
    {
        LogError("Channel Open Failed");
    }
}

/*************************************************************************
*函数名 : on_io_bytes_received
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : io接收后的回调
*输入参数 : void* context 上下文对象
            const unsigned char* buffer 接收到的数据
            size_t size 数据大小
*输出参数 :
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static void on_io_bytes_received(void *context, const unsigned char *buffer, size_t size)
{

    PM_CHANNEL_MAP *map = (PM_CHANNEL_MAP*)context;

    if (map != NULL && buffer != NULL && size > 0)
    {
        map->task_lock = false;
        map->time_out_count = 0;

        PM_HANDLE pm_handle = map->pm_handle;

        if (pm_handle != NULL)
        {
            DATA_RESPONSE *data_response = malloc(sizeof(DATA_RESPONSE));

            if (NULL == data_response)
            {
                LogError("Error: Malloc DATA_RESPONSE Failure");
            }
            else
            {
                data_response->data = buffer;
                data_response->length = size;

                LogBinary("RX: ", data_response->data, data_response->length);

                DATA_PAYLOAD *data_payload = data_response_parse(pm_handle, data_response);

                if (NULL != data_payload)
                {
                    LogInfo("%s", data_payload->payload);
                    
                    publish_mqtt_message(get_mqtt_client_handle(g_iotea_handle),
                            data_payload->DBTopic, DELIVER_AT_LEAST_ONCE,
                            (uint8_t*)data_payload->payload,
                            strlen(data_payload->payload), NULL , NULL);

                    free(data_payload->payload);
                    data_payload->payload = NULL;
                    free(data_payload);
                    data_payload = NULL;
                }

                free(data_response);
                data_response = NULL;
                ThreadAPI_Sleep(200);
            }
        }
    }
}

/*************************************************************************
*函数名 : on_io_error
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : io发生错误的回调
*输入参数 : void* context 上下文对象
*输出参数 :
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static void on_io_error(void *context)
{
    LogError("IO reported an error");

    PM_CHANNEL_MAP *map = (PM_CHANNEL_MAP*)context;

    if (NULL != map)
    {
        map->task_stop = true;
    }
}

/*************************************************************************
*函数名 : map_list_destroy
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : map 链表销毁
*输入参数 : SINGLYLINKEDLIST_HANDLE map_list 链表对象
*输出参数 :
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static void map_list_destroy(SINGLYLINKEDLIST_HANDLE map_list)
{
    if (NULL == map_list) return;

    LIST_ITEM_HANDLE map_item = singlylinkedlist_get_head_item(map_list);

    while (map_item != NULL)
    {
        PM_CHANNEL_MAP *map = (PM_CHANNEL_MAP*)singlylinkedlist_item_get_value(map_item);

        if (map != NULL)
        {
            if (map->task_thread != NULL)
            {
                map->task_stop = true;
                ThreadAPI_Join(map->task_thread, &map->thread_id);
            }
            protocol_destroy(map->pm_handle);
            xio_close(map->xio_handle, NULL, NULL);
            xio_destroy(map->xio_handle);
            free(map);
            map = NULL;
        }
        singlylinkedlist_remove(map_list, map_item);
        map_item = singlylinkedlist_get_head_item(map_list);
    }

    singlylinkedlist_destroy(map_list);
}


/*************************************************************************
*函数名 : send_resquest_thread
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 发送线程
*输入参数 : void * handle 上下文对象
*输出参数 :
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static int send_resquest_thread(void *handle)
{
    int result;

    // protocol manager channel map
    PM_CHANNEL_MAP *map = (PM_CHANNEL_MAP*)handle;

    // IO handle
    XIO_HANDLE xio_handle = (XIO_HANDLE)map->xio_handle;

    // protocol manager handle
    PM_HANDLE pm_handle = map->pm_handle;

    // if error return
    if (NULL == pm_handle || NULL == xio_handle)
    {
        result = __FAILURE__;
        return result;
    }

    // package send time
    TIME_TO_SEND *send_time;

    // package data
    DATA_REQUEST *data_resquest;

    struct tm *time_info;
    time_t current_time;
    time_t temp_time;

    TICK_COUNTER_HANDLE tick_counter_handle = tickcounter_create();
    tickcounter_ms_t current_tick;
    tickcounter_ms_t last_send_tick;

    while (true)
    {
        // thread stop flag
        if (map->task_stop)
        {
            LogInfo("% s thread exit", map->key_word);
            break;
        }
        xio_dowork(xio_handle);   // 处理io数据(读写IO)

        // 获取要发送数据的时间
        send_time = get_next_send_time(pm_handle); // pm_handle 是protocol manager对象

        if (NULL != send_time)
        {

            // 获取当前计数时间
            tickcounter_get_current_ms(tick_counter_handle, &current_tick);

            if (strcmp(send_time->acqMode, "realtime") == 0) // 判断使用哪种方式
            {

                // 定时轮询方式
                if ((current_tick - send_time->packetSendTime) / 1000 > send_time->acqInt)
                {
                    if (!map->task_lock)
                    {

                        // 符合发送条件，获取数据并发送
                        data_resquest = get_request_data(pm_handle);
                        if (xio_send(xio_handle, data_resquest->data, data_resquest->length, on_send_complete,
                         NULL) != 0)
                        {
                            LogError("Send failed");
                        }

                        LogBinary("TX: ", data_resquest->data, data_resquest->length);

                        map->task_lock = true;
                        last_send_tick = current_tick;

                        send_time->packetSendTime = current_tick; //更新发送时间
                    }
                    else
                    {

                        // 如果一秒钟没有回应数据
                        if ((current_tick - last_send_tick) / 1000 > 2)
                        {
                            map->task_lock = false;
                            map->time_out_count += 1;
                            if (map->time_out_count >= 5)
                            {
                                LogError("Error: Send Data Time Out");
                                map->task_stop = true;
                            }
                        }
                    }
                }
            }
            else if(strcmp(send_time->acqMode, "schfrz") == 0)
            {
                if (send_time->freezing)
                {
                    if ((current_tick - send_time->packetSendTime) / 1000 > send_time->acqInt)
                    {
                        data_resquest = get_request_data(pm_handle);
                        if (xio_send(xio_handle, data_resquest->data, data_resquest->length, on_send_complete,
                            NULL) != 0)
                        {
                            LogError("Send failed");
                        }

                        map->task_lock = true;

                        send_time->packetSendTime = current_tick; //更新发送时间
                    }
                }

                time(&current_time);
                temp_time = current_time - current_time % 60;
                if (temp_time >= current_time)
                {
                    time_info = localtime( &temp_time );

                    if (send_time->startTime.Mins[time_info->tm_min] &&
                        send_time->startTime.Hous[time_info->tm_hour] &&
                        send_time->startTime.Days[time_info->tm_mday] &&
                        send_time->startTime.Mons[time_info->tm_mon])
                    {
                        send_time->freezing = true;
                    }
                    if (send_time->endTime.Mins[time_info->tm_min] &&
                        send_time->endTime.Hous[time_info->tm_hour] &&
                        send_time->endTime.Days[time_info->tm_mday] &&
                        send_time->endTime.Mons[time_info->tm_mon])
                    {
                        send_time->freezing = false;
                    }
                }
            }
            else
            {
                // 增加一个判断条件， 解决秒匹配问题
                if ((current_tick - send_time->packetSendTime) / 1000 > 60)
                {
                    time(&current_time);
                    temp_time = current_time - current_time % 60;
                    if (temp_time >= current_time)
                    {
                        time_info = localtime( &temp_time );

                        // 匹配当前月，日，小时，分钟。 没有匹配到秒， 所以需要上面的判断解决一分钟内都符合条件的情况
                        if (send_time->startTime.Mins[time_info->tm_min] &&
                            send_time->startTime.Hous[time_info->tm_hour] &&
                            send_time->startTime.Days[time_info->tm_mday] &&
                            send_time->startTime.Mons[time_info->tm_mon])
                        {

                            data_resquest = get_request_data(pm_handle);
                            if (xio_send(xio_handle, data_resquest->data, data_resquest->length, on_send_complete,
                             NULL) != 0)
                            {
                                LogError("Send failed");
                            }

                            send_time->packetSendTime = current_tick;
                        }
                    }
                }
            }
        }
        ThreadAPI_Sleep(10);
    }
    tickcounter_destroy(tick_counter_handle);
    ThreadAPI_Exit(map->thread_id);

    return result;
}

/*************************************************************************
*函数名 : channel_init_by_list
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 初始化IO通道
*输入参数 : const void* item 链表节点
            const void* action_context  上下文对象
             bool* continue_processing  继续遍历标志
*输出参数 :
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static void channel_init_by_list(const void *item, const void *action_context, bool *continue_processing)
{

    SINGLYLINKEDLIST_HANDLE map_list = (SINGLYLINKEDLIST_HANDLE)action_context;

    CHANNEL_MODEL_INFO *channel_item = (CHANNEL_MODEL_INFO*)(item);

    XIO_HANDLE xio_handle = NULL;

    const IO_INTERFACE_DESCRIPTION *xio_interface;

    if (strcmp(channel_item->portOptions.portType, "serial") == 0)
    {
        xio_interface = serialio_get_interface_description();

        if (NULL != xio_interface)
        {
            SERIALIO_CONFIG serialio_config;
            strncpy(serialio_config.interfaceName, channel_item->portOptions.serialID, MAX_SIZE_OF_INTER_NAME);
            serialio_config.baudRate = channel_item->portOptions.baudRate;
            serialio_config.dataBits = channel_item->portOptions.dataBit;
            serialio_config.parity = channel_item->portOptions.parity;
            serialio_config.stopBits = channel_item->portOptions.stopBit;
            serialio_config.timeout.tv_sec = 0;
            serialio_config.timeout.tv_usec = channel_item->timeOut;
            xio_handle = xio_create(xio_interface, &serialio_config);
        }
    }
    else if(strcmp(channel_item->portOptions.portType, "socket") == 0)
    {
        xio_interface = socketio_get_interface_description();

        if (NULL != xio_interface)
        {
            SOCKETIO_CONFIG socketio_config;
            socketio_config.hostname = channel_item->portOptions.host;
            socketio_config.port = channel_item->portOptions.port;
            xio_handle = xio_create(xio_interface, &socketio_config);
        }
    }
    else
    {
        LogError("Port Type Error");
        g_channel_parse_error = true;
        *continue_processing = false;
        return;
    }

    if (xio_handle == NULL)
    {
        LogError("Error: Create xio_handle Failed.");
    }
    else
    {
        PM_CHANNEL_MAP *map_item = malloc(sizeof(PM_CHANNEL_MAP));

        if (NULL == map_item)
        {
            LogError("Error: Malloc PM_CHANNEL_MAP Failure");
            xio_destroy(xio_handle);
        }
        else
        {
            if (xio_open(xio_handle, on_io_open_complete, xio_handle, on_io_bytes_received, map_item,
                on_io_error, map_item) != 0)
            {
                LogError("Error: opening socket IO.");
                xio_destroy(xio_handle);
                free(map_item);
                map_item = NULL;
            }
            else
            {
                map_item->xio_handle = xio_handle;
                map_item->pm_handle = NULL;
                map_item->task_thread = NULL;
                map_item->task_stop = false;
                map_item->task_lock = false;
                map_item->time_out_count = 0;
                strncpy(map_item->key_word, channel_item->modelNmae, MAX_SIZE_OF_KEY);

                if (singlylinkedlist_add(map_list, map_item) == NULL)
                {
                    LogError("Error: Map_list Add Failure");
                    xio_close(xio_handle, NULL, NULL);
                    xio_destroy(xio_handle);
                    free(map_item);
                    map_item = NULL;
                }
                else
                {
                    *continue_processing = true;
                    return;
                }
            }
        }
    }
    g_channel_parse_error = true;
    *continue_processing = false;
}

/*************************************************************************
*函数名 : map_channel_and_model
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 绑定IO通道和协议模型
*输入参数 : const void* item 链表节点
            const void* action_context  上下文对象
             bool* continue_processing  继续遍历标志
*输出参数 :
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static void map_channel_and_model(const void *item, const void *action_context, bool *continue_processing)
{
    PM_HANDLE instance = (PM_HANDLE)action_context;

    PM_CHANNEL_MAP *map = (PM_CHANNEL_MAP*)item;

    const char *model_name = get_model_name(instance);

    if (strcmp(model_name, map->key_word) == 0)
    {

        if(map->pm_handle != NULL)
        {
            if (map->task_thread != NULL)
            {
                map->task_stop = true;
                ThreadAPI_Join(map->task_thread, &map->thread_id);
                map->task_stop = false;
            }
            protocol_destroy(map->pm_handle);
        }

        map->pm_handle = instance;

        THREAD_HANDLE thread_handle;

        THREADAPI_RESULT rc = ThreadAPI_Create(&thread_handle, send_resquest_thread, map);

        if (rc == THREADAPI_OK)
        {
            g_thread_count++;
            map->task_thread = thread_handle;
            map->thread_id = g_thread_count;
            LogInfo("%s Thread Create Success", map->key_word);
        }
        else
        {
            LogError("Thread Create Failure");
        }
    }

    *continue_processing = true;
}


/*************************************************************************
*函数名 : thing_instance_construct
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 根据json对象初始化物模型数据结构
*输入参数 : const char * json_string json对象
*输出参数 :
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
static int thing_instance_construct(const char *json_string)
{
    int result = 0;

    if (NULL == json_string) return __FAILURE__;

    if (NULL == g_pm_map_list)
    {
        LogError("ThingInstanceConstruct g_pm_map_list Is Null");
        result = __FAILURE__;
    }
    else
    {
        JSON_Value *root_value;

        root_value = json_parse_string(json_string);

        if (json_value_get_type(root_value) != JSONObject)
        {
            LogError("ThingInstanceConstruct Not a JSON Object");
            result = __FAILURE__;
        }
        else
        {
            JSON_Object *json_object = json_value_get_object(root_value);

            if (NULL != json_object )
            {
                JSON_Array *body_array = json_object_dotget_array(json_object, "body");

                if (NULL != body_array)
                {
                    int body_count = json_array_get_count(body_array);

                    for (int i = 0; i < body_count; i++)
                    {
                        JSON_Object *body_object = json_array_get_object(body_array, i);

                        if (NULL == body_object)
                        {
                            continue;
                        }

                        PM_HANDLE pm_handle = protocol_init(body_object);

                        if (NULL == pm_handle)
                        {
                            // 处理错误
                            LogError("ThingInstanceConstruct Protocol_Init Error");
                            continue;
                        }
                        else
                        {
                            g_thread_count = 0;
                            singlylinkedlist_foreach(g_pm_map_list, map_channel_and_model, pm_handle);
                        }
                    }
                }
                else
                {
                    LogError("ThingInstanceConstruct Failure");
                    result = __FAILURE__;
                }
            }
            else
            {
                LogError("ThingInstanceConstruct Failure");
                result = __FAILURE__;
            }

            if ( 0 == g_thread_count)
            {
                // 销毁 Map_list and io
                LogError("No task thread created");
                map_list_destroy(g_pm_map_list);
                g_pm_map_list = NULL;
                result = __FAILURE__;
            }
            json_value_free(root_value);
        }
    }
    return result;
}

/*************************************************************************
*函数名 : channel_instance_construct
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 根据json对象初始化通道模型数据结构
*输入参数 : const char * json_string json对象
*输出参数 :
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
static int channel_instance_construct(const char *json_string)
{
    int result = 0;

    if (NULL == json_string) return __FAILURE__;

    JSON_Value *root_value;

    SINGLYLINKEDLIST_HANDLE channel_list;

    root_value = json_parse_string(json_string);

    if (json_value_get_type(root_value) != JSONObject)
    {
        LogError("Not a JSON Object");
        result = __FAILURE__;
        return result;
    }

    channel_list = channel_info_list_create(root_value);

    if (NULL == channel_list)
    {
        LogError("channel_list Create Failed");
        result = __FAILURE__;
    }
    else
    {
        if (g_pm_map_list != NULL)
        {
            map_list_destroy(g_pm_map_list);
            g_thread_count = 0;
        }

        g_pm_map_list = singlylinkedlist_create();

        if (NULL == g_pm_map_list)
        {
            LogError("g_pm_map_list Create Failed");
            result = __FAILURE__;
        }
        else
        {
            singlylinkedlist_foreach(channel_list, channel_init_by_list, g_pm_map_list);
            if (g_channel_parse_error)
            {
                LogError("channel_init_by_list Failure");
                singlylinkedlist_destroy(g_pm_map_list);
                g_pm_map_list = NULL;
                result = __FAILURE__;
            }
        }
    }

    channel_info_list_destroy(channel_list);
    json_value_free(root_value);
    return result;
}

/*************************************************************************
*函数名 : sigint_handle
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 处理SIGINT信号
*输入参数 : void
*输出参数 :
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static void sigint_handle()
{
    g_sigint_happen = true;
}

/*************************************************************************
*函数名 : iotea_client_run
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 运行mqtt 客户端
*输入参数 : const char * borker  服务端地址
            const char * username   用户名
            const char * password   密码
*输出参数 :
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
int iotea_client_run(char *borker, char *username, char *password,
    char *prototype, char *devicename)
{

    if (0 != platform_init())
    {
        LogError("platform_init failed");
        return __FAILURE__;
    }

    g_iotea_handle = iotea_client_init(borker, devicename, prototype);

    if (NULL == g_iotea_handle)
    {
        platform_deinit();
        LogError("iotea_client_init failed");
        return __FAILURE__;
    }
    else
    {
        LogInfo("iotea client init");
    }

    iotea_client_register_thing_model_init(g_iotea_handle, thing_instance_construct);
    iotea_client_register_channel_model_init(g_iotea_handle, channel_instance_construct);

    IOTEA_CLIENT_OPTIONS options;
    options.cleanSession = true;
    options.clientId = devicename;
    options.username = username;
    options.password = password;
    options.keepAliveInterval = 30;
    options.retryTimeoutInSeconds = 300;

    if (0 != iotea_client_connect(g_iotea_handle, &options))
    {
        iotea_client_deinit(g_iotea_handle);
        platform_deinit();
        LogError("iotea_client_connect failed");
        return __FAILURE__;
    }
    else
    {
        LogInfo("iotea client connected");
    }

    struct sigaction sa;
    sa.sa_handler = sigint_handle;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);

    while (iotea_client_dowork(g_iotea_handle) >= 0 && g_sigint_happen == false)
    {
        ThreadAPI_Sleep(10);
    }
    map_list_destroy(g_pm_map_list);
    iotea_client_deinit(g_iotea_handle);
    platform_deinit();

    return 0;
}

/*************************************************************************
*函数名 : main
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 主函数
*输入参数 : int argc 参数个数
            char * argv[] 参数地址数组
*输出参数 :
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
int main(int argc, char *argv[])
{
    int opt = 0;

    char *borker     = NULL;
    char *username   = NULL;
    char *password   = NULL;
    char *prototype  = NULL;
    char *devicename = NULL;

    while((opt = getopt_long(argc, argv, "u:p:P:d:H:vh?", long_options, NULL)) != EOF)
    {
        switch(opt)
        {
            case 'v':
                LogInfo("Program Version: "VERSION"");
                return 0;
            case 'P':
                prototype = optarg;
                break;
            case 'd':
                devicename = optarg;
                break;
            case 'u':
                username = optarg;
                break;
            case 'p':
                password = optarg;
                break;
            case 'H':
                borker = optarg;
                break;
            case ':':
            case 'h':
            case '?':
                usage_help();
                return 0;
            default:
                usage_help();
                return 0;
        }
    }

    borker     = borker ? borker : BROKER;
    username   = username ? username : USERNAME;
    password   = password ? password : PASSWORD;
    prototype  = prototype ? prototype : PROTO_TYPE;
    devicename = devicename ? devicename : DEVICE_NAME;

    int rc = iotea_client_run(borker, username, password, prototype, devicename);

    if (rc < 0)
    {
        LogError("IOTEA Run Error Exit");
    }

    return 0;
}
