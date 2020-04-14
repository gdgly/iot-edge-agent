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


#include <azure_c_shared_utility/xlogging.h>
#include <azure_c_shared_utility/singlylinkedlist.h>
#include <azure_c_shared_utility/crt_abstractions.h>

#include "dlt698_master.h"

#include "common_include.h"
#include "protocol_manager.h"
#include "dlt_698.h"
#include "utils.h"

/*************************************************************************
*函数名 : parse_dlt698_model_by_json 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 根据json对象解析模型
*输入参数 : JSON_Object * json_object 物模型json信息
*输出参数 : 
*返回值: static DLT698_MODEL_INFO * 698模型结构体
*调用关系 :
*其它:
*************************************************************************/
static DLT698_MODEL_INFO * parse_dlt698_model_by_json(JSON_Object *json_object)
{
    DLT698_MODEL_INFO *result;

    if (NULL == json_object)
    {
        result = NULL;
    }
    else
    {
        result = malloc(sizeof(DLT698_MODEL_INFO));

        if (NULL == result)
        {
            LogError("Malloc DLT698_MODEL_INFO Failure");
        }
        else
        {
            int error = 0;
            const char *res_str = NULL;

            if (json_object_dotget_number(json_object, ACQINT))
            {
                result->acqInt = (int)json_object_dotget_number(json_object, ACQINT);
            }
            else
            {
                error = __FAILURE__;
            }
            
            res_str = json_object_dotget_string(json_object, "acqMode");
            
            if (NULL == res_str)
                error = __FAILURE__;
            else
                strncpy(result->acqMode, res_str, MAX_SIZE_OF_MODE);
            
            res_str = json_object_dotget_string(json_object, "DBTopic");
            
            if (NULL == res_str)
                error = __FAILURE__;
            else
                strncpy(result->DBTopic, res_str, MAX_SIZE_OF_DBTOPIC);

            JSON_Object *start_time = json_object_dotget_object(json_object, STARTTIME);
            
            if (NULL == start_time)
            {
                error = __FAILURE__;
            }
            else
            {
                result->startTime.InvMons = (int)json_object_dotget_number(start_time, "InvMons");
                result->startTime.InvDays = (int)json_object_dotget_number(start_time, "InvDays");
                result->startTime.InvHous = (int)json_object_dotget_number(start_time, "InvHous");
                result->startTime.InvMins = (int)json_object_dotget_number(start_time, "InvMins");
            }

            JSON_Object *end_time = json_object_dotget_object(json_object, "endTime");
            
            if (NULL == end_time)
            {
                error = __FAILURE__;
            }
            else
            {
                result->endTime.InvMons = (int)json_object_dotget_number(end_time, "InvMons");
                result->endTime.InvDays = (int)json_object_dotget_number(end_time, "InvDays");
                result->endTime.InvHous = (int)json_object_dotget_number(end_time, "InvHous");
                result->endTime.InvMins = (int)json_object_dotget_number(end_time, "InvMins");
            }

            JSON_Object *acq_file = json_object_dotget_object(json_object, ACQFILE);

            if (NULL == acq_file)
            {
                error = __FAILURE__;
            }
            else
            {
                result->acqFile.id =  (int)json_object_dotget_number(acq_file, ID);
                result->acqFile.big =  (int)json_object_dotget_number(acq_file, BIG);
                
    
                res_str = json_object_dotget_string(acq_file, M);
                
                if (NULL == res_str)
                    error = __FAILURE__;
                else
                    strncpy(result->acqFile.m, res_str, MAX_SIZE_OF_M);

                res_str = json_object_dotget_string(acq_file, MATH);
                
                if (NULL == res_str)
                    error = __FAILURE__;
                else
                    strncpy(result->acqFile.math, res_str, MAX_SIZE_OF_MATH);

                const char *oad = json_object_dotget_string(acq_file, "oad");

                if (NULL == oad)
                {
                    error = __FAILURE__;
                }
                else
                {
                    sscanf(oad, "%02hhx%02hhx%02hhx%02hhx", &result->acqFile.oad[0],
                                                            &result->acqFile.oad[1], 
                                                            &result->acqFile.oad[2], 
                                                            &result->acqFile.oad[3]);
                }
                
                const char *addr = json_object_dotget_string(acq_file, "addr");

                if (NULL == addr)
                {
                    error = __FAILURE__;
                }
                else
                {
                    sscanf(addr, "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx", &result->acqFile.addr[0],
                                                                         &result->acqFile.addr[1], 
                                                                         &result->acqFile.addr[2], 
                                                                         &result->acqFile.addr[3],
                                                                         &result->acqFile.addr[4],
                                                                         &result->acqFile.addr[5]);
                }
            }
            
            if (error != 0)
            {
                LogError("parse_dlt698_model_by_json Failure");
                free(result);
                result = NULL;
            }
        }
    }
    return result;
}

/*************************************************************************
*函数名 : dlt698_data_create 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 根据模型结构体创建协议管理数据信息
*输入参数 : DLT698_MODEL_INFO * model_info 698模型结构体
*输出参数 : 
*返回值: static PM_DATA_INFO * 协议管理数据信息
*调用关系 :
*其它:
*************************************************************************/
static PM_DATA_INFO * dlt698_data_create(DLT698_MODEL_INFO *model_info)
{
    
    PM_DATA_INFO *result;

    if (NULL == model_info)
    {
        LogError("Error dlt698_data_create model_info is Null");
        result = NULL;
    }
    else
    {
        result = malloc(sizeof(PM_DATA_INFO));

        if (result == NULL)
        {
            LogError("Error: dlt698_data_create Malloc DLT698_DATA Fialure");
        }
        else
        {
            result->time_to_send = malloc(sizeof(TIME_TO_SEND));

            if (NULL == result->time_to_send)
            {
                LogError("Error: dlt698_data_create Malloc TIME_TO_SEND");
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
                LogError("Error: dlt698_data_create Malloc DATA_REQUEST");
                free(result);
                result = NULL;
            }
            else
            {
                DLT698_PACKET *dlt698_packet = dlt698_packet_build(model_info->acqFile.oad, model_info->acqFile.addr);

                result->data_request->data = dlt698_packet->data;
                result->data_request->length = dlt698_packet->length;
            }
            
        }
    }
    return result;
}

/*************************************************************************
*函数名 : dlt698_model_info_destroy 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 销毁model info
*输入参数 : SINGLYLINKEDLIST_HANDLE dlt698_data_list 链表
*输出参数 :  
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
void dlt698_model_info_destroy(SINGLYLINKEDLIST_HANDLE dlt698_data_list)
{
    if (NULL == dlt698_data_list) return;

    LIST_ITEM_HANDLE dlt698_data_item = singlylinkedlist_get_head_item(dlt698_data_list);

    while (dlt698_data_item != NULL)
    {
        PM_DATA_INFO *pm_data_info = (PM_DATA_INFO*)singlylinkedlist_item_get_value(dlt698_data_item);
        if (pm_data_info != NULL)
        {
            free(pm_data_info->model_info);
            pm_data_info->model_info = NULL;
        }
        singlylinkedlist_remove(dlt698_data_list, dlt698_data_item);
        dlt698_data_item = singlylinkedlist_get_head_item(dlt698_data_list);
    }
    
    singlylinkedlist_destroy(dlt698_data_list);
}

/*************************************************************************
*函数名 : DLT698_Init 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : DLT698协议初始化
*输入参数 : JSON_Array * json_array 698模型信息
*输出参数 :  
*返回值: DATA_DESCRIPTION * 数据描述
*调用关系 :
*其它:
*************************************************************************/
DATA_DESCRIPTION * dlt698_init(JSON_Array *json_array)
{
    if (NULL == json_array) return NULL;
    
    DATA_DESCRIPTION *result;

    result = malloc(sizeof(DATA_DESCRIPTION));

    if (NULL == result)
    {
        LogError("DLT698_Init Malloc DATA_DESCRIPTION Failure");
    }
    else
    {
        SINGLYLINKEDLIST_HANDLE dlt698_data_list = singlylinkedlist_create();

        if (NULL == dlt698_data_list)
        {
            LogError("DLT698_Init dlt698_data_list create Failure");
            free(result);
            result = NULL;
        }
        else
        {
            int body_count = json_array_get_count(json_array);
            
            int err = 0;

            for (int i = 0; i < body_count; i++)
            {

                JSON_Object *dlt698_object = json_array_get_object(json_array, i);

                if (NULL != dlt698_object)
                {
                    
                    DLT698_MODEL_INFO *model_info = parse_dlt698_model_by_json(dlt698_object);

                    if (NULL == model_info)
                    {
                        err = __FAILURE__;
                        break;
                    }
                    else
                    {
                        PM_DATA_INFO *dlt698_data = dlt698_data_create(model_info);

                        if (NULL == dlt698_data)
                        {
                            free(model_info);
                            model_info = NULL;
                            dlt698_data->model_info = NULL;
                            err = -1;
                            break;
                        }
                        else
                        {
                            dlt698_data->model_info = model_info;
                            singlylinkedlist_add(dlt698_data_list, dlt698_data);
                        }
                    }
                }
            }
            if (err == 0)
            {
                result->dm_info_list = dlt698_data_list;
            }
            else
            {
                // 销毁
                LogError("DLT698_Init data_info create Failure");
                dlt698_model_info_destroy(dlt698_data_list);
                free(result);
                result = NULL;
            }
        }
    }

    return result;
}

/*************************************************************************
*函数名 : dlt698_response_parse 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : DLT698协议解析
*输入参数 : const void * model_info 模型信息
            const PM_DATA_INFO * data_info  协议帧数据信息
            DATA_RESPONSE * data_response   返回的数据帧
*输出参数 : 
*返回值: char * 解析后的数据
*调用关系 :
*其它:
*************************************************************************/
DATA_PAYLOAD * dlt698_response_parse(const void *model_info, const PM_DATA_INFO *data_info, 
    DATA_RESPONSE *data_response)
{
    DATA_PAYLOAD *result;

    if (NULL == model_info || NULL == data_info || NULL == data_response)
    {
        result = NULL;
    }
    else
    {
        DLT698_MODEL_INFO *dlt698_model = (DLT698_MODEL_INFO *)model_info;

        DLT698_ANALY_RES *dlt698_ananly_res = dlt698_response_analy((uint8_t *)data_response->data, 
            data_response->length);

        if (NULL == dlt698_ananly_res)
        {
            LogError("Error: dlt698_response_analy");
            result = NULL;
        }
        else
        {
            JSON_Value *payload = json_value_init_object();
            
            if (NULL == payload)
            {
                LogError("json_value_init_object Failure");
            }
            else
            {
                JSON_Object *root = json_object(payload);
            
                char time_result[32] = {0};
                get_current_json_time(time_result);
                json_object_set_string(root, "timestamp", time_result);

                json_object_set_string(root, "token", "077de87b3a04");

                if (strcmp(dlt698_model->acqMode, "realtime") == 0)
                {
                    json_object_set_string(root, "data_raw", "single");
                }

                JSON_Value *first_body_array = json_value_init_array();
                JSON_Array *first_body = json_array(first_body_array);
                
                json_object_set_value(root, "body", first_body_array);
                
                JSON_Value *temp_value;
                JSON_Object *temp_object;

                // temp_value = json_value_init_object();
                // temp_object = json_object(temp_value);
                
                // get_time_by_model(dlt698_model->startTime.InvMons, dlt698_model->startTime.InvDays, 
                //     dlt698_model->startTime.InvHous, dlt698_model->startTime.InvMins, time_result);
                // json_object_set_string(temp_object, "timestamp", time_result);
                // json_object_set_string(temp_object, "timestartgather", time_result);

                // get_time_by_model(dlt698_model->endTime.InvMons, dlt698_model->endTime.InvDays, 
                //     dlt698_model->endTime.InvHous, dlt698_model->endTime.InvMins, time_result);
                // json_object_set_string(temp_object, "timeendgather", time_result);

                // JSON_Value *second_body_array = json_value_init_array();
                // JSON_Array *second_body = json_array(second_body_array);
                // json_object_set_value(temp_object, "body", second_body_array);
                
                // json_array_append_value(first_body, temp_value);

                int math = atoi(dlt698_model->acqFile.math);
                char res[32] = {0};
                char temp_name[16] = {0};
                char *res_temp = res;
                char *main_name = dlt698_model->acqFile.m;
                int index = 1;
                
                for (int i = 0; i < dlt698_ananly_res->ucDataNum; i++)
                {
                    temp_value = json_value_init_object();
                    temp_object = json_object(temp_value);
                    
                    convert_to_string_float(dlt698_ananly_res->stDataUnit[i].ucPtr, res, math);
                    json_object_set_string(temp_object, "name", main_name);
                    json_object_set_string(temp_object, "val", res_temp);
                    json_array_append_value(first_body, temp_value);

                    // res_temp = strtok(res, ",");
                    // while(res_temp != NULL)
                    // {
                    //     temp_value = json_value_init_object();
                    //     temp_object = json_object(temp_value);

                    //     snprintf(temp_name, 16, "%s_%d", main_name, index);
                    //     json_object_set_string(temp_object, "name", temp_name);
                    //     json_object_set_string(temp_object, "val", res_temp);
                    //     json_array_append_value(first_body, temp_value);
                    //     index++;
                    //     res_temp = strtok(NULL, ",");
                    // }   
                }

                char *json_string = json_serialize_to_string(payload);
                
                if (json_string == NULL)
                {
                    LogError("json_serialize_to_string Failure");
                }
                else
                {
                    result = malloc(sizeof(DATA_PAYLOAD));

                    if (result == NULL)
                    {
                        LogError("Malloc DATA_PAYLOAD Failure");
                    }
                    else
                    {
                        if (mallocAndStrcpy_s(&result->payload, json_string) != 0)
                        {
                            LogError("mallocAndStrcpy_s construct_payload_by_model");
                            free(result);
                            result = NULL;
                        }
                        else
                        {
                            strncpy(result->DBTopic, dlt698_model->DBTopic, 64); 
                        }
                    }
                    json_free_serialized_string(json_string);
                }
                json_value_free(payload);
            }
            free(dlt698_ananly_res);
            dlt698_ananly_res = NULL;
        }
    }
    return result;
}

/*************************************************************************
*函数名 : dlt698_destroy 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : DLT698协议销毁
*输入参数 : DATA_DESCRIPTION * handle 数据描述实例
*输出参数 : 
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
void dlt698_destroy(DATA_DESCRIPTION *handle)
{
    if (NULL == handle) return;
    
    DATA_DESCRIPTION *data_desc = (DATA_DESCRIPTION *)handle;

    LIST_ITEM_HANDLE dlt698_data_item = singlylinkedlist_get_head_item(data_desc->dm_info_list);
    
    while (dlt698_data_item != NULL)
    {
        PM_DATA_INFO *dlt698_data = (PM_DATA_INFO*)singlylinkedlist_item_get_value(dlt698_data_item);
        
        if (dlt698_data != NULL)
        {
            free(dlt698_data->data_request->data);
            dlt698_data->data_request->data = NULL;
            free(dlt698_data->data_request);
            dlt698_data->data_request = NULL;
            free(dlt698_data->time_to_send);
            dlt698_data->time_to_send = NULL;
            free(dlt698_data->model_info);
            dlt698_data->model_info = NULL;
            free(dlt698_data);
            dlt698_data = NULL;
        }
        singlylinkedlist_remove(data_desc->dm_info_list, dlt698_data_item);
        dlt698_data_item = singlylinkedlist_get_head_item(data_desc->dm_info_list);
    }
    singlylinkedlist_destroy(data_desc->dm_info_list);
    
    free(data_desc);
    data_desc = NULL;
}

INTER_DESCRIPTION dlt698_interface_description = {
    .Construct = dlt698_init,
    .Destroy   = dlt698_destroy,
    .response_parse = dlt698_response_parse
};

/*************************************************************************
*函数名 : dlt698_get_interface_description 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 获取698接口描述
*输入参数 : void
*输出参数 : 
*返回值: INTER_DESCRIPTION* 接口描述
*调用关系 :
*其它:
*************************************************************************/
const INTER_DESCRIPTION* dlt698_get_interface_description(void)
{
    return &dlt698_interface_description;
}