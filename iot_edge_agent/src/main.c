#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include <azure_c_shared_utility/xlogging.h>
#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/threadapi.h>
#include <azure_c_shared_utility/buffer_.h>
#include <azure_c_shared_utility/tickcounter.h>

#include "parson.h"
#include "iot_model.h"
#include "modbus_rtu.h"
#include "serial_port.h"
#include "iot_edge_agent.h"
#include "protocol_manager.h"

#define PROGRAM_VERSION "1.0"

#define ADDRESS  "tcp://192.168.199.12:1883"
#define DEVICE   "MODBUS_RTU"
#define USERNAME "test"
#define PASSWORD "hahaha"

int interval = -1;

PROTO_MGR_HANDLE proto_manager;
MODBUS_RTU_MODEL * modbus_model = NULL;

DATA_REQUEST * data_request = NULL;
DATA_RESPONSE * data_response = NULL;

static const struct option long_options[]=
{
    {"username", required_argument, NULL, 'u'},
    {"password", required_argument, NULL, 'p'},
    {"host", required_argument, NULL, 'H'},
    {"help", no_argument,NULL,'?'},
    {"version",no_argument,NULL,'v'},
    {NULL,0,NULL,0}
};

static void usage(void)
{
    fprintf(stderr,
        "  用法  [选项]  [参数]  Provide args for mqtt client run up \n"
        "  -u, --username        Username is used for mqtt login.\n"
        "  -p, --password        Password is used for mqtt login.\n"
        "  -H, --host            Host include host ip and port.\n"
        "  -h, --help            This information.\n"
        "  -v, --version         Display program version.\n"
    );
}

static void Log(const char* message)
{
    printf("%s\r\n", message);
}

static uint32_t reversebytes_uint32(uint32_t value){
    return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |
        (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;
}

static uint16_t reversebytes_uint16(uint16_t value){
    return (value & 0x00FFU) << 8 | (value & 0xFF00U) >> 8;
}

static double * fix_value_by_model(uint8_t * val, int byte_len, REGISTER_GROUP * register_group, int group_len)
{
    double * result = malloc(sizeof(double) * group_len);
    if (result == NULL) {
        LogError("Alloc Mem failed");
    } else {
        uint16_t temp_u16;
        int16_t temp_16;
        uint8_t temp_8;
        uint8_t * ptr = val;
        for(int i = 0; i < group_len; i++) 
        {
            if (register_group[i].fn == 3 || register_group[i].fn == 4)
            {
                memcpy(&temp_u16, ptr, 2);
                if (register_group[i].big) {
                    temp_u16 = reversebytes_uint16(temp_u16);
                }
                if (!strcmp(register_group[i].df, "u16") == 0) 
                {
                    temp_16 = (int16_t)temp_u16;
                    result[i] = temp_16;
                } else {
                    result[i] = temp_u16;
                }
                ptr += 2;
            }
            else if (register_group[i].fn == 1 || register_group[i].fn == 2)
            {
                memcpy(&temp_8, ptr, 1);
                result[i] = temp_8;
                ptr++;
            }
            result[i] /= 10;
        }
    }
    return result;
}

static JSON_Value* construct_payload_by_model(MODBUS_RTU_MODEL * model, DATA_RESPONSE * data_response)
{
    if (model && data_response) {

        JSON_Value * payload = json_value_init_object();
        JSON_Value * body_array = json_value_init_array();
        JSON_Object* root = json_object(payload);
        JSON_Array * body = json_array(body_array);

        json_object_set_value(root, "body", body_array);
        json_object_set_string(root, "token", "e4c60ef4-27bb-11ea-ba4f-077de87b3a04");

        char str_time[12] = {'\0'};
        time_t currentTime = time(NULL);
        sprintf(str_time, "%ld", currentTime);
        json_object_set_string(root, "timestamp", str_time);

        JSON_Value * temp_value;
        JSON_Object* temp_object;

        double * val = fix_value_by_model(data_response->data, 4, model->register_group, model->register_group_len);

        for (int i = 0; i < model->register_group_len; i++) {
            temp_value = json_value_init_object();
            temp_object = json_object(temp_value);
            json_object_set_string(temp_object, "name", model->register_group[i].m);
            json_object_set_number(temp_object, "val", val[i]);
            json_array_append_value(body, temp_value);
        }
        return payload;
    } else {
        LogError("data parse filed");
    }

}

static DATA_REQUEST * protocol_init_by_model(MODBUS_RTU_MODEL * model)
{
    interval = model->acq_int;
    proto_manager = protocol_init(MODBUS_RTU);
    data_request = proto_manager->init(1, 0, model->register_group_len);
    printf("Init modbus protocol -> ");
    for (int i = 0; i < data_request->length; i++)
    {
        printf("0x%02hhx ", data_request->data[i]);
    }
    printf("\n");
    return data_request;
}


static void HandleModelParse(const char * json_string)
{
    // printf("%s\n", json_string);

    JSON_Value * root_value;

    root_value = json_parse_string(json_string);

    if (json_value_get_type(root_value) != JSONObject)
    {
        LogError("Not a JSON Object");
        return;
    }

    PROTO_TYPE proto_type = parse_proto_header(root_value);

    switch (proto_type)
    {
        case MODBUS_RTU:

            modbus_model = modbus_model_parse(root_value);

            if (NULL == modbus_model) 
            {
                LogError("modbus model parse failed");
            } 
            else
            {
                data_request = protocol_init_by_model(modbus_model);

                if (NULL == data_request)
                {
                    LogError("protocol_init_by_model failed");
                }
            }

            json_value_free(root_value);
            break;

        default:
            break;
    }
}

void OnSerialRecvCallback(SERIAL_MESSAGE_HANDLE Serial_Msg, void * context)
{
    IOTEA_CLIENT_HANDLE eaHandle = (IOTEA_CLIENT_HANDLE)context;

    data_response = proto_manager->parser(Serial_Msg->message, Serial_Msg->length);

    if (NULL == data_response)
    {
        LogError("proto_manager parser failed");
        SerialPort_message_destroy(Serial_Msg);

    } 
    else 
    {
        uint16_t * data = (uint16_t *)data_response->data;
        LogInfo("温度: %.2f 湿度：%.2f", data[0] / 10.0, data[1] / 10.0);

        JSON_Value * payload = construct_payload_by_model(modbus_model, data_response);

        if (NULL == payload)
        {

            LogError("construct_payload_by_model failed");
            SerialPort_message_destroy(Serial_Msg);

        }
        else
        {
            char* encoded = json_serialize_to_string(payload);

            if (NULL == encoded) 
            {

                LogError("Failue: failed to encode the json.");
                json_value_free(payload);
                SerialPort_message_destroy(Serial_Msg);
                
            }
            else
            {
                printf("%s\n", encoded);

                publish_mqtt_message(get_mqtt_client_handle(eaHandle), DATA_PUB_TOPIC, DELIVER_AT_LEAST_ONCE, (uint8_t*)encoded, strlen(encoded), NULL , NULL);

                json_free_serialized_string(encoded);
                json_value_free(payload);
                SerialPort_message_destroy(Serial_Msg);
            }
        }  
    }
    
}

int iotea_client_run(const char * host, const char * username, const char * password)
{

    // serial init
    SERIAL_PORT serial_port = SerialPort_create("/dev/ttyUSB4", 9600, 8, 'N', 1);
    if (NULL == serial_port)
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "Error: serial port init failed");
        return __FAILURE__;
    }

    bool rc = SerialPort_open(serial_port);
    if (false == rc) 
    {
        SerialPort_destroy(serial_port);
        LOG(AZ_LOG_ERROR, LOG_LINE, "Error: serial port open failed");
        return __FAILURE__;
    }

    if (0 != platform_init())
    {
        SerialPort_close(serial_port);
        SerialPort_destroy(serial_port);
        LogError("platform_init failed");
        return __FAILURE__;
    }

    // 初始化iot edge agent
    IOTEA_CLIENT_HANDLE handle;

    if (host)
    {
        handle = iotea_client_init((char *)host, DEVICE);
    } else {
        handle = iotea_client_init(ADDRESS, DEVICE);
    }

    if (NULL == handle)
    {
        platform_deinit();
        SerialPort_close(serial_port);
        SerialPort_destroy(serial_port);
        LogError("iotea_client_init failed");
        return __FAILURE__;
    }
    else
    {
        LogInfo("iotea client init");
    }

    // register model 
    iotea_client_register_model_parse(handle, HandleModelParse);

    IOTEA_CLIENT_OPTIONS options;
    options.cleanSession = true;
    options.clientId = DEVICE;

    if(username && password) 
    {
        options.username = (char *)username;
        options.password = (char *)password;
    } else {
        options.username = USERNAME;
        options.password = PASSWORD;
    }

    options.keepAliveInterval = 30;
    options.retryTimeoutInSeconds = 300;

    if (0 != iotea_client_connect(handle, &options))
    {
        iotea_client_deinit(handle);
        platform_deinit();
        SerialPort_close(serial_port);
        SerialPort_destroy(serial_port);

        LogError("iotea_client_connect failed");
        return __FAILURE__;
    }
    else
    {
        LogInfo("iotea client connected");
    }
    

    iotea_client_dowork(handle);

    TICK_COUNTER_HANDLE tickCounterHandle = tickcounter_create();
    tickcounter_ms_t currentTime, lastSendTime;
    tickcounter_get_current_ms(tickCounterHandle, &lastSendTime);

    while (iotea_client_dowork(handle) >= 0)
    {
        SerialPort_recv(serial_port, OnSerialRecvCallback, handle);
        tickcounter_get_current_ms(tickCounterHandle, &currentTime);

        if (-1 != interval && (currentTime - lastSendTime) / 1000 > interval)
        {

            if(data_request) 
            {
                int res = SerialPort_send(serial_port, data_request->data, data_request->length);
                printf("send serial data\n");
            }

            lastSendTime = currentTime;
        }

        ThreadAPI_Sleep(10);
    }

    SerialPort_close(serial_port);
    SerialPort_destroy(serial_port);
    tickcounter_destroy(tickCounterHandle);
    iotea_client_deinit(handle);
    platform_deinit();
    free(modbus_model);
    
    return 0;
}

int main(int argc, char * argv[]){
    int opt = 0;

    const char* username = NULL;
    const char* password = NULL;
    const char* host = NULL;

    while((opt=getopt_long(argc,argv,"u:p:H:vh?",long_options,NULL)) != EOF )
    {
        switch(opt)
        {
            case 'v':
                printf("Program Version: "PROGRAM_VERSION"\n");
                exit(0);
            case 'u':
                username = optarg; break;
            case 'p':
                password = optarg; break;
            case 'H':
                host = optarg; break;
            case ':':
            case 'h':
            case '?': usage();return 0;break;
        }
    }

    iotea_client_run(host, username, password);

    exit(0);
}
