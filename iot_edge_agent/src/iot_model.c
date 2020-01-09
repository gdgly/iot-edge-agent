/**************************************************
*- Author       : aゞ小波
*- CreateTime   : 2019-12-18 10:59
*- Email        : 465728296@qq.com
*- Filename     : iot-model.c
*- Description  :
***************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <azure_c_shared_utility/xlogging.h>
#include "azure_c_shared_utility/crt_abstractions.h"

#include "parson.h"
#include "iot_model.h"
#include "modbus_rtu.h"
#include "protocol_manager.h"

#define N               "n"
#define M               "m"
#define DF              "df"
#define FN              "fn"
#define IP              "ip"
#define BIG             "big"
#define DEV             "dev"
#define UPM             "upm"
#define MATH            "math"
#define PORT            "port"
#define PARITY          "parity"
#define DATABIT         "dataBit"
#define STOPBIT         "stopBit"
#define REGGRP          "regGrp"
#define ACQINT          "acqInt"
#define ADDRESS         "addr"
#define TIMEOUT         "timeOut"
#define DEVADDR         "devAddr"
#define RELMODEL        "relmodel"
#define BANDRATE        "baudRate"
#define PORTTYPE        "portType"
#define SERIALID        "serialID"
#define PROTOTYPE       "protoType"
#define MODELTYPE       "modeltype"

int parse_device_info_to_list(JSON_Value * json_value, SINGLYLINKEDLIST_HANDLE device_list, int * device_count)
{
    int result = 0;

    JSON_Object * device_object = json_value_get_object(json_value);

    JSON_Array  * device_body = json_object_dotget_array(device_object, "body");

    DEVICE_CHANNEL_INFO * device_channel;
    DEVICE_PORT_OPTION  * port_option;

    *device_count = json_array_get_count(device_body);

    JSON_Object * array;
    JSON_Object * port;

    for (int i = 0; i < *device_count; i++)
    {
        array = json_array_get_object(device_body, i);

        device_channel = malloc(sizeof(DEVICE_CHANNEL_INFO));

        if (NULL == device_channel)
        {
            LogError("Alloc DEVICE_CHANNEL_INFO failed");
            return __FAILURE__;
        }
        device_channel->acqInt = atoi(json_object_dotget_string(array, ACQINT));
        device_channel->upm = atoi(json_object_dotget_string(array, UPM));
        device_channel->timeOut = atoi(json_object_dotget_string(array, TIMEOUT));
        device_channel->devAddr = atoi(json_object_dotget_string(array, DEVADDR));

        if (mallocAndStrcpy_s(&(device_channel->dev), json_object_dotget_string(array, DEV)) != 0)
        {
            free(device_channel);
            LogError("mallocAndStrcpy_s dev");
            return __FAILURE__;
        }
        if (mallocAndStrcpy_s(&(device_channel->protoType), json_object_dotget_string(array, PROTOTYPE)) != 0)
        {
            free(device_channel);
            LogError("mallocAndStrcpy_s protoType");
            return __FAILURE__;
        }
        if (mallocAndStrcpy_s(&(device_channel->modelType), json_object_dotget_string(array, MODELTYPE)) != 0)
        {
            free(device_channel);
            LogError("mallocAndStrcpy_s modelType");
            return __FAILURE__;
        }

        port = json_object_dotget_object(array, "port");

        port_option = malloc(sizeof(DEVICE_PORT_OPTION));
        if (NULL == port_option)
        {
            free(device_channel);
            LogError("Alloc DEVICE_PORT_OPTION failed");
            return __FAILURE__;
        }
        device_channel->device_port = port_option;

        port_option->baudRate = atoi(json_object_dotget_string(port, BANDRATE));
        port_option->stopBit = atoi(json_object_dotget_string(port, STOPBIT));
        port_option->dataBit = atoi(json_object_dotget_string(port, DATABIT));
        port_option->port = atoi(json_object_dotget_string(port, PORT));

        if (mallocAndStrcpy_s(&(port_option->serialID), json_object_dotget_string(port, SERIALID)) != 0)
        {
            free(port_option);
            free(device_channel);
            LogError("mallocAndStrcpy_s serialID");
            return __FAILURE__;
        }

        if (mallocAndStrcpy_s(&(port_option->portType), json_object_dotget_string(port, PORTTYPE)) != 0)
        {
            free(port_option);
            free(device_channel);
            LogError("mallocAndStrcpy_s portType");
            return __FAILURE__;
        }

        if (mallocAndStrcpy_s(&(port_option->ip), json_object_dotget_string(port, IP)) != 0)
        {
            free(port_option);
            free(device_channel);
            LogError("mallocAndStrcpy_s ip");
            return __FAILURE__;
        }

        if (strcmp("NONE", json_object_dotget_string(port, PARITY)) == 0)
        {
            port_option->parity = 'N';
        }
        else if (strcmp("EVEN", json_object_dotget_string(port, PARITY)) == 0)
        {
            port_option->parity = 'E';
        }
        else if (strcmp("ODD", json_object_dotget_string(port, PARITY)) == 0)
        {
            port_option->parity = 'O';
        }
        else
        {
            free(port_option);
            free(device_channel);
            LogError("parity arg error");
            return __FAILURE__;
        }

        singlylinkedlist_add(device_list, device_channel);
    }

    return result;
}

static JSON_Object * get_first_body_object(JSON_Value * json_value)
{
    JSON_Object * iot_model_object;
    JSON_Array  * body;
    JSON_Object * body_object;

    iot_model_object = json_value_get_object(json_value);

    body = json_object_dotget_array(iot_model_object, "body");

    body_object = json_array_get_object(body, 0);

    return body_object;
}

static JSON_Object * get_second_body_object(JSON_Value * json_value)
{
    JSON_Object * first_body = get_first_body_object(json_value);

    JSON_Array  * body_array = json_object_dotget_array(first_body, "body");

    JSON_Object * body_object = json_array_get_object(body_array, 0);

    return body_object;
}

PROTO_TYPE parse_proto_header(JSON_Value * json_value)
{

    JSON_Object * body_object = get_first_body_object(json_value);

    const char * prototype = json_object_dotget_string(body_object, "prototype");

    if (strcmp(prototype, "MODBUS_RTU") == 0)
    {
        return MODBUS_RTU;
    }
}

MODBUS_RTU_MODEL * modbus_model_parse(JSON_Value * json_value)
{
    MODBUS_RTU_MODEL * result;
    
    if (json_string == NULL) {
        result = NULL;
    }
    else
    {
        result = malloc(sizeof(MODBUS_RTU_MODEL));

        if (NULL  == result)
        {
            printf("Alloc MODBUS_RTU_MODEL failed\n");
        } 
        else 
        {

            JSON_Object * body = get_second_body_object(json_value);

            result->acq_int = (int)json_object_dotget_number(body, ACQINT);

            JSON_Array * reg_grp = json_object_dotget_array(body, REGGRP);

            int reg_grp_len = json_array_get_count(reg_grp);

            result->register_group_len = reg_grp_len;

            REGISTER_GROUP * reg_group = malloc(sizeof(REGISTER_GROUP) * reg_grp_len);

            result->register_group = reg_group;

            REGISTER_GROUP * temp = reg_group;

            for ( int i = 0; i < reg_grp_len; i++) 
            {

                JSON_Object * arr = json_array_get_object(reg_grp, i);

                if (mallocAndStrcpy_s(&(temp->df), json_object_dotget_string(arr, DF)) != 0)
                {
                    LogError("mallocAndStrcpy_s df");
                    free(result);
                    return NULL;
                }

                if (mallocAndStrcpy_s(&(temp->relmodel), json_object_dotget_string(arr, RELMODEL)) != 0)
                {
                    LogError("mallocAndStrcpy_s relmodel");
                    free(result);
                    return NULL;
                }

                if (mallocAndStrcpy_s(&(temp->math), json_object_dotget_string(arr, MATH)) != 0)
                {
                    LogError("mallocAndStrcpy_s math");
                    free(result);
                    return NULL;
                }

                if (mallocAndStrcpy_s(&(temp->address), json_object_dotget_string(arr, ADDRESS)) != 0)
                {
                    LogError("mallocAndStrcpy_s address");
                    free(result);
                    return NULL;
                }

                if (mallocAndStrcpy_s(&(temp->m), json_object_dotget_string(arr, M)) != 0)
                {
                    LogError("mallocAndStrcpy_s m");
                    free(result);
                    return NULL;
                }

                temp->n = (int)json_object_dotget_number(arr, N);
                temp->fn = (int)json_object_dotget_number(arr, FN);
                temp->big = (int)json_object_dotget_number(arr, BIG);

                temp++;
            }
        }
    }
    return result;
}


void iot_model_free(void)
{
    return;
}
