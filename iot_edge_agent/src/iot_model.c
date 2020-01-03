/**************************************************
*- Author       : aゞ小波
*- CreateTime   : 2019-12-18 10:59
*- Email        : 465728296@qq.com
*- Filename     : iot-model.c
*- Description  :
***************************************************/
#include <azure_c_shared_utility/xlogging.h>
#include "azure_c_shared_utility/crt_abstractions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parson.h"
#include "iot_model.h"
#include "modbus_rtu.h"
#include "protocol_manager.h"

#define N               "n"
#define M               "m"
#define DF              "df"
#define FN              "fn"
#define BIG             "big"
#define MATH            "math"
#define REGGRP          "regGrp"
#define ACQINT          "acqInt"
#define ADDRESS         "addr"
#define RELMODEL        "relmodel"

static JSON_Value * root_value;

static JSON_Object * get_first_body_object(const char * josn_string)
{
    JSON_Object * iot_model_object;

    JSON_Array * body;
    JSON_Object * body_object;
    // JSON_Array * regGrp;

    root_value = json_parse_string(josn_string);

    if (json_value_get_type(root_value) != JSONObject) {
        printf("is not a json object\n");
        return NULL;
    }

    iot_model_object = json_value_get_object(root_value);

    body = json_object_dotget_array(iot_model_object, "body");

    body_object = json_array_get_object(body, 0);

    return body_object;
}

static JSON_Object * get_second_body_object(const char * josn_string)
{
    JSON_Object * first_body = get_first_body_object(josn_string);

    JSON_Array * body_array = json_object_dotget_array(first_body, "body");

    JSON_Object * body_object = json_array_get_object(body_array, 0);

    return body_object;
}

PROTO_TYPE parse_proto_header(const char * json_string)
{

    JSON_Object * body_object = get_first_body_object(json_string);

    const char * prototype = json_object_dotget_string(body_object, "prototype");

    if (strcmp(prototype, "MODBUS_RTU") == 0) {
        return MODBUS_RTU;
    }
}

MODBUS_RTU_MODEL * modbus_model_parse(const char  * json_string)
{
    MODBUS_RTU_MODEL * result;
    
    if (json_string == NULL) {

        result = NULL;

    } else {
        result = malloc(sizeof(MODBUS_RTU_MODEL));

        if (result == NULL) {

            printf("Alloc MODBUS_RTU_MODEL failed\n");

        } else {

            JSON_Object * body = get_second_body_object(json_string);

            result->acq_int = (int)json_object_dotget_number(body, ACQINT);

            JSON_Array * reg_grp = json_object_dotget_array(body, REGGRP);

            int reg_grp_len = json_array_get_count(reg_grp);

            result->register_group_len = reg_grp_len;

            REGISTER_GROUP * reg_group = malloc(sizeof(REGISTER_GROUP) * reg_grp_len);

            result->register_group = reg_group;

            REGISTER_GROUP * temp = reg_group;

            for ( int i = 0; i < reg_grp_len; i++) {
                JSON_Object * arr = json_array_get_object(reg_grp, i);

                if (mallocAndStrcpy_s(&(temp->df), json_object_dotget_string(arr, DF)) != 0) {
                    LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s df");
                    free(result);
                    return NULL;
                }

                if (mallocAndStrcpy_s(&(temp->relmodel), json_object_dotget_string(arr, RELMODEL)) != 0) {
                    LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s relmodel");
                    free(result);
                    return NULL;
                }

                if (mallocAndStrcpy_s(&(temp->math), json_object_dotget_string(arr, MATH)) != 0) {
                    LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s math");
                    free(result);
                    return NULL;
                }

                if (mallocAndStrcpy_s(&(temp->address), json_object_dotget_string(arr, ADDRESS)) != 0) {
                    LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s address");
                    free(result);
                    return NULL;
                }

                if (mallocAndStrcpy_s(&(temp->m), json_object_dotget_string(arr, M)) != 0) {
                    LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s m");
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
    json_value_free(root_value);
}
