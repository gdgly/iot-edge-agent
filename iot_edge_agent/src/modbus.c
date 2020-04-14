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

#include <lightmodbus/lightmodbus.h>
#include <lightmodbus/master.h>

#include <azure_c_shared_utility/xlogging.h>
#include <azure_c_shared_utility/singlylinkedlist.h>
#include <azure_c_shared_utility/crt_abstractions.h>

#include "common_include.h"
#include "protocol_manager.h"
#include "modbus.h"
#include "utils.h"

// modbus master
ModbusMaster g_modbus_master;

/*************************************************************************
*函数名 : fix_value_by_model 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 获取服务器地址
*输入参数 : uint8_t *val
           int byte_len
           uint8_t fn
           const REGISTER_TAG *register_group
           int group_len
*输出参数 : 
*返回值: double *
*调用关系 :
*其它:
*************************************************************************/
static double *fix_value_by_model(uint8_t *val, int byte_len, uint8_t fn, 
    const REGISTER_TAG *register_group, int group_len)
{
    double *result = malloc(sizeof(double) *group_len);
    if (result == NULL)
    {
        LogError("Alloc Mem failed");
    }
    else
    {
        uint16_t temp_u16;
        int16_t temp_int16;
        uint8_t temp_int8;
        uint8_t *ptr = val;
        for(int i = 0; i < group_len; i++) 
        {
            if (fn == 3 || fn == 4)
            {
                memcpy(&temp_u16, ptr, 2);
                if (register_group[i].big)
                {
                    temp_u16 = reversebytes_uint16(temp_u16);
                }
                if (!(strcmp(register_group[i].df, "u16") == 0)) 
                {
                    temp_int16 = (int16_t)temp_u16;
                    result[i] = temp_int16;
                }
                else
                {
                    result[i] = temp_u16;
                }
                ptr += 2;
            }
            else if (fn == 1 || fn == 2)
            {
                memcpy(&temp_int8, ptr, 1);
                result[i] = temp_int8;
                ptr++;
            }
            result[i] = result[i] / 10;
        }
    }
    return result;
}

/*************************************************************************
*函数名 : construct_payload_by_model 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 根据模型构造payload
*输入参数 : const MODBUS_MODEL_INFO *model modbus模型
            DATA_RESPONSE *data_response 串口返回的数据
*输出参数 : 
*返回值: char *
*调用关系 :
*其它:
*************************************************************************/
static char * construct_payload_by_model(const MODBUS_MODEL_INFO *model, DATA_RESPONSE *data_response)
{
    char *result;

    if (NULL != model && NULL != data_response)
    {
        JSON_Value *payload = json_value_init_object();
        JSON_Value *body_array = json_value_init_array();
        JSON_Object *root = json_object(payload);
        JSON_Array *body = json_array(body_array);

        json_object_set_value(root, "body", body_array);
        json_object_set_string(root, "token", "077de87b3a04");

        char str_time[12] = {0};
        time_t current_time = time(NULL);
        snprintf(str_time, 12, "%ld", current_time);
        json_object_set_string(root, "timestamp", str_time);

        JSON_Value *temp_value;
        JSON_Object *temp_object;

        double *fixed_value = fix_value_by_model((uint8_t *)data_response->data, 4,
         model->fn ,model->mRegister, model->regCount);

        char res_temp[10];

        for (int i = 0; i < model->regCount; i++) 
        {
            temp_value = json_value_init_object();
            temp_object = json_object(temp_value);
            json_object_set_string(temp_object, "name", model->mRegister[i].m);
            snprintf(res_temp, 10, "%.2lf", fixed_value[i]);
            json_object_set_string(temp_object, "val", res_temp);
            json_array_append_value(body, temp_value);
        }
        char *res = json_serialize_to_string(payload);
        
        if (mallocAndStrcpy_s(&result, res) != 0)
        {
            LogError("mallocAndStrcpy_s construct_payload_by_model");
            result = NULL;
        }

        free(fixed_value);
        fixed_value = NULL;
        json_free_serialized_string(res);
        json_value_free(payload);
    } 
    else
    {
        LogError("data parse filed");
        result = NULL;
    }
    return result;
}

/*************************************************************************
*函数名 : parse_modbus_model_by_json 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 根据model json对象构造model info结构体
*输入参数 : JSON_Object *json_object model json对象
*输出参数 : 
*返回值: MODBUS_MODEL_INFO *
*调用关系 :
*其它:
*************************************************************************/
static MODBUS_MODEL_INFO * parse_modbus_model_by_json(JSON_Object *json_object)
{
    MODBUS_MODEL_INFO *result;

    JSON_Array *reg_grp_arr = json_object_dotget_array(json_object, REGGRP);

    int reg_grp_len = json_array_get_count(reg_grp_arr);

    if (NULL == reg_grp_arr || 0 == reg_grp_len)
    {
        result = NULL;
    }
    else
    {
        result = malloc(sizeof(MODBUS_MODEL_INFO) + sizeof(REGISTER_TAG)*reg_grp_len);

        if (NULL == result)
        {
            LogError("Malloc MODBUS_MODEL_INFO Failed");
        }
        else
        {
            strncpy(result->acqMode, json_object_dotget_string(json_object, "acqMode"), MAX_SIZE_OF_MODE);
            
            strncpy(result->DBTopic, json_object_dotget_string(json_object, "DBTopic"), MAX_SIZE_OF_DBTOPIC);

            JSON_Object *start_time = json_object_dotget_object(json_object, "startTime");
            
            // 检查
            result->startTime.InvMons = (int)json_object_dotget_number(start_time, "InvMons");
            result->startTime.InvDays = (int)json_object_dotget_number(start_time, "InvDays");
            result->startTime.InvHous = (int)json_object_dotget_number(start_time, "InvHous");
            result->startTime.InvMins = (int)json_object_dotget_number(start_time, "InvMins");

            JSON_Object *end_time = json_object_dotget_object(json_object, "endTime");
            
            result->endTime.InvMons = (int)json_object_dotget_number(end_time, "InvMons");
            result->endTime.InvDays = (int)json_object_dotget_number(end_time, "InvDays");
            result->endTime.InvHous = (int)json_object_dotget_number(end_time, "InvHous");
            result->endTime.InvMins = (int)json_object_dotget_number(end_time, "InvMins");

            result->acqInt = (int)json_object_dotget_number(json_object, ACQINT);
            result->fn = (int)json_object_dotget_number(json_object, FN);
            result->devAddr = (int)json_object_dotget_number(json_object, DEVADDR);
            result->regCount = reg_grp_len;

            const char *res_str;
            JSON_Object *reg_grp_object;
            int rc;
            int error = 0;

            for ( int i = 0; i < reg_grp_len; i++)
            {
                reg_grp_object = json_array_get_object(reg_grp_arr, i);

                result->mRegister[i].id = (int)json_object_dotget_number(reg_grp_object, ID);
                result->mRegister[i].address = (int)json_object_dotget_number(reg_grp_object, ADDR);
                result->mRegister[i].big = (int)json_object_dotget_number(reg_grp_object, BIG);

                res_str = json_object_dotget_string(reg_grp_object, M);

                rc = strncpy_s(result->mRegister[i].m, MAX_SIZE_OF_M, res_str, strlen(res_str));
                
                // 检查rc
                res_str = json_object_dotget_string(reg_grp_object, DF);
                rc = strncpy_s(result->mRegister[i].df, MAX_SIZE_OF_DF, res_str, strlen(res_str));
                
                res_str = json_object_dotget_string(reg_grp_object, MATH);
                rc = strncpy_s(result->mRegister[i].math, MAX_SIZE_OF_MATH, res_str, strlen(res_str));
            }
        }
    }
    return result;
}

/*************************************************************************
*函数名 : modbus_data_info_create 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 创建modbus PM 数据信息
*输入参数 : MODBUS_MODEL_INFO *model_info 模型信息
*输出参数 : 
*返回值: PM_DATA_INFO *
*调用关系 :
*其它:
*************************************************************************/
static PM_DATA_INFO * modbus_data_info_create(MODBUS_MODEL_INFO *model_info)
{
    PM_DATA_INFO *result;

    if (NULL == model_info)
    {
        result = NULL;
    }
    else
    {
        result = malloc(sizeof(PM_DATA_INFO));

        if (NULL == result)
        {
            LogError("Error: Malloc MODBUS_DATA");
        }
        else
        {
            result->time_to_send = malloc(sizeof(TIME_TO_SEND));

            if (NULL == result->time_to_send)
            {
                LogError("Error: Malloc TIME_TO_SEND");
                free(result);
                result = NULL;
            }
            else
            {
                result->time_to_send->freezing = false;

                result->time_to_send->acqInt = model_info->acqInt;

                strncpy(result->time_to_send->acqMode, model_info->acqMode, MAX_SIZE_OF_MODE);

                if (model_info->startTime.InvMons == -1)
                {
                    memset(result->time_to_send->startTime.Mons, 1, 12);
                }
                else
                {
                    result->time_to_send->startTime.Mons[model_info->startTime.InvMons] = 1;
                }

                if (model_info->startTime.InvDays == -1)
                {
                    memset(result->time_to_send->startTime.Days, 1, 32);
                }
                else
                {
                    result->time_to_send->startTime.Days[model_info->startTime.InvDays] = 1;
                }
                
                if (model_info->startTime.InvHous == -1)
                {
                    memset(result->time_to_send->startTime.Hous, 1, 24);
                }
                else
                {
                    result->time_to_send->startTime.Hous[model_info->startTime.InvHous] = 1;
                }

                result->time_to_send->startTime.Mins[model_info->startTime.InvMins] = 1;

                if (model_info->endTime.InvMons == -1)
                {
                    memset(result->time_to_send->endTime.Mons, 1, 12);
                }
                else
                {
                    result->time_to_send->endTime.Mons[model_info->endTime.InvMons] = 1;
                }

                if (model_info->endTime.InvDays == -1)
                {
                    memset(result->time_to_send->endTime.Days, 1, 32);
                }
                else
                {
                    result->time_to_send->endTime.Days[model_info->endTime.InvDays] = 1;
                }
                
                if (model_info->endTime.InvHous == -1)
                {
                    memset(result->time_to_send->endTime.Hous, 1, 24);
                }
                else
                {
                    result->time_to_send->endTime.Hous[model_info->endTime.InvHous] = 1;
                }

                result->time_to_send->endTime.Mins[model_info->endTime.InvMins] = 1;
            }

            result->time_to_send->packetSendTime = 0;

            result->data_request = malloc(sizeof(DATA_REQUEST));

            if (NULL == result->data_request)
            {
                LogError("Error: Malloc DATA_REQUEST");
                free(result->time_to_send);
                result->time_to_send = NULL;
                free(result);
                result = NULL;
            }
            else
            {
                switch (model_info->fn)
                {
                case 0x03:
                case 0x04:
                    modbusBuildRequest0304(&g_modbus_master, model_info->fn, model_info->devAddr, 0, 
                        model_info->regCount);
                    break;
                default:
                    break;
                }

                result->data_request->data = g_modbus_master.request.frame;
                g_modbus_master.request.frame = NULL;
                result->data_request->length = g_modbus_master.request.length;
                result->data_request->predictedResponseLength = g_modbus_master.predictedResponseLength;
            }
        }
    }
    return result;
}

/*************************************************************************
*函数名 : modbus_init 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : modbus协议初始化
*输入参数 : JSON_Array *json_array modbus物模型
*输出参数 : 
*返回值: DATA_DESCRIPTION * 数据描述
*调用关系 :
*其它:
*************************************************************************/
DATA_DESCRIPTION * modbus_init(JSON_Array *json_array)
{
    
    if (NULL == json_array) return NULL;
    
    DATA_DESCRIPTION *result;
    
    result = malloc(sizeof(DATA_DESCRIPTION));

    if (NULL == result)
    {
        LogError("Modbus_Init Malloc DATA_DESCRIPTION Failure");
    }
    else
    {
        ModbusError rc = modbusMasterInit(&g_modbus_master);

        if (rc != MODBUS_OK)
        {
            LogError("Error: modbusMasterInit");
            free(result);
            result = NULL;
        }else
        {
            SINGLYLINKEDLIST_HANDLE modbus_list = singlylinkedlist_create();

            if (NULL == modbus_list)
            {
                LogError("Modbus_Init modbus_list create Failure");
                modbusMasterEnd(&g_modbus_master);
                free(result);
                result = NULL;
            }
            else
            {
                int body_count = json_array_get_count(json_array);

                int err = 0;
                JSON_Object *modbus_object;
                MODBUS_MODEL_INFO *model_info;
                PM_DATA_INFO *data_info;

                for (int i = 0; i < body_count; i++)
                {
                    modbus_object = json_array_get_object(json_array, i);

                    if (NULL != modbus_object)
                    {

                        model_info = parse_modbus_model_by_json(modbus_object);

                        if (NULL == model_info)
                        {
                            err = -1;
                            break;
                        }
                        else
                        {
                            data_info = modbus_data_info_create(model_info);
                            
                            if (NULL == data_info)
                            {
                                free(model_info);
                                model_info = NULL;
                                err = -1;
                                break;
                            }
                            else
                            {
                                data_info->model_info = model_info;
                                singlylinkedlist_add(modbus_list, data_info);
                            }
                        }
                    }
                }

                if (err == 0)
                {
                    result->dm_info_list = modbus_list;
                }
                else
                {
                    // 销毁
                    LogError("Modbus_Init data_info create Failure");
                    modbus_destroy(result);
                    result = NULL;
                }
            }
        }  
    }
    return result;
}


/*************************************************************************
*函数名 : mosbus_response_parse 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 解析modbus响应
*输入参数 : const void *model_info 模型信息
            const PM_DATA_INFO *data_info PM数据信息
            DATA_RESPONSE *data_response 返回的数据
*输出参数 : 
*返回值: char * 解析后的数据
*调用关系 :
*其它:
*************************************************************************/
DATA_PAYLOAD * mosbus_response_parse(const void *model_info, const PM_DATA_INFO *data_info, 
    DATA_RESPONSE *data_response)
{
    DATA_PAYLOAD *result;

    MODBUS_MODEL_INFO *modbus_model_info = (MODBUS_MODEL_INFO *)model_info;

    if (NULL == modbus_model_info || NULL == data_info || NULL == data_response)
    {
        result = NULL;
    }
    else
    {
        ModbusMaster *modbus_master = &g_modbus_master;
        
        if (NULL == modbus_master)
        {
            LogError("Parse Error modbus_master is Null");
            result = NULL;
        }
        else
        {
            modbus_master->request.frame = data_info->data_request->data;
            modbus_master->request.length = data_info->data_request->length;

            modbus_master->response.frame = data_response->data;
            modbus_master->response.length = data_response->length;

            ModbusError rc = modbusParseResponse(modbus_master);

            if (rc != MODBUS_OK)
            {
                LogError("Modbus Response Parse Failure");
                result = NULL;
            }
            else
            {
                const uint8_t *res;
                int length;

                length = modbus_master->data.count;

                switch (modbus_master->data.type)
                {
                    case MODBUS_HOLDING_REGISTER:
                    case MODBUS_INPUT_REGISTER:
                        res = (uint8_t *)modbus_master->data.regs;
                        break;

                    case MODBUS_COIL:
                    case MODBUS_DISCRETE_INPUT:
                        res = modbus_master->data.coils;
                        break;
                    default:
                        break;
                }

                data_response->data = res;
                data_response->length = length;

                result = malloc(sizeof(DATA_PAYLOAD));
                if (result == NULL)
                {
                    LogError("Malloc DATA_PAYLOAD Failure");
                }
                else
                {
                    result->payload = construct_payload_by_model(modbus_model_info, data_response);
                    strncpy(result->DBTopic, modbus_model_info->DBTopic, 64); 
                }
            }
        } 
    }
    return result;
}

/*************************************************************************
*函数名 : modbus_destroy 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : modbus 协议销毁
*输入参数 : DATA_DESCRIPTION *handle modbus实例
*输出参数 : 
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
void modbus_destroy(DATA_DESCRIPTION *handle)
{
    if (NULL == handle) return;
    
    DATA_DESCRIPTION *data_desc = (DATA_DESCRIPTION *)handle;

    LIST_ITEM_HANDLE modbus_data_item = singlylinkedlist_get_head_item(data_desc->dm_info_list);
    
    while (modbus_data_item != NULL)
    {
        PM_DATA_INFO *modbus_data = (PM_DATA_INFO*)singlylinkedlist_item_get_value(modbus_data_item);
        if (modbus_data != NULL)
        {
            free(modbus_data->data_request->data);
            modbus_data->data_request->data = NULL;
            free(modbus_data->data_request);
            modbus_data->data_request = NULL;
            free(modbus_data->time_to_send);
            modbus_data->time_to_send = NULL;
            free(modbus_data->model_info);
            modbus_data->model_info = NULL;
            free(modbus_data);
            modbus_data = NULL;
        }
        singlylinkedlist_remove(data_desc->dm_info_list, modbus_data_item);
        modbus_data_item = singlylinkedlist_get_head_item(data_desc->dm_info_list);
    }
    singlylinkedlist_destroy(data_desc->dm_info_list);

    free(data_desc);
    data_desc = NULL;

    // 避免内存重复释放
    g_modbus_master.request.frame = NULL;
    modbusMasterEnd(&g_modbus_master);
}

INTER_DESCRIPTION modbus_interface_description = {
    .Construct = modbus_init,
    .Destroy   = modbus_destroy,
    .response_parse = mosbus_response_parse,
};

/*************************************************************************
*函数名 : modbus_get_interface_description 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 获取modbus接口描述
*输入参数 : void
*输出参数 : 
*返回值: INTER_DESCRIPTION* 接口描述
*调用关系 :
*其它:
*************************************************************************/
const INTER_DESCRIPTION* modbus_get_interface_description(void)
{
    return &modbus_interface_description;
}