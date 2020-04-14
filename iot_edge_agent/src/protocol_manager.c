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

#include <azure_c_shared_utility/singlylinkedlist.h>
#include <azure_c_shared_utility/xlogging.h>
#include <azure_c_shared_utility/crt_abstractions.h>

#include "modbus.h"
#include "dlt_698.h"
#include "protocol_manager.h"

#define MAX_SIZE_OF_MNAME   (32)

// 协议管理
typedef struct PM_INSTANCE_TAG
{
    char ModelName[MAX_SIZE_OF_MNAME + 1];    // 用于匹配通道
    DATA_DESCRIPTION  *data_desc;             // 数据描述
    const INTER_DESCRIPTION *interface;       // 接口描述
    LIST_ITEM_HANDLE next_data_info;
    LIST_ITEM_HANDLE prev_data_info;
    LIST_ITEM_HANDLE curr_data_info;
} PM_INSTANCE;

/*************************************************************************
*函数名 : protocol_init
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 初始化协议管理对象
*输入参数 : JSON_Object * json_object 物模型json数据
*输出参数 :
*返回值: PM_HANDLE 协议管理实例
*调用关系 :
*其它:
*************************************************************************/
PM_HANDLE protocol_init(JSON_Object *json_object)
{

    PM_HANDLE result;

    const char *proto_type = json_object_dotget_string(json_object, "protoType");
    const char *model_name = json_object_dotget_string(json_object, "name");

    if (NULL == proto_type || NULL == model_name)
    {
        result = NULL;
    }
    else
    {
        result = (PM_HANDLE)malloc(sizeof(PM_INSTANCE));

        if (NULL == result)
        {
            LogError("Error: Malloc PM_INSTANCE");
        }
        else
        {
            if (strcmp(proto_type, "MODBUS") == 0)
            {
                result->interface = modbus_get_interface_description();
            }
            else if(strcmp(proto_type, "DLT698.45") == 0)
            {
                result->interface = dlt698_get_interface_description();
            }
            else
            {
                LogError("Prorocol Not Support");
                free(result);
                result = NULL;
            }

            if (result != NULL)
            {
                JSON_Array *body_array = json_object_dotget_array(json_object, "body");

                if (NULL == body_array)
                {
                    LogError("Get body_array Error");
                    free(result);
                    result = NULL;
                }
                else
                {
                    int rc = strncpy_s(result->ModelName, MAX_SIZE_OF_MNAME, model_name, strlen(model_name));
                    result->data_desc = result->interface->Construct(body_array);

                    if (NULL == result->data_desc || rc != 0)
                    {
                        LogError("Error Construct");
                        free(result);
                        result = NULL;
                    }
                    else
                    {
                        result->next_data_info = singlylinkedlist_get_head_item(result->data_desc->dm_info_list);
                        result->curr_data_info = result->prev_data_info = NULL;
                    }
                }
            }
        }
    }

    return result;
}

/*************************************************************************
*函数名 : get_model_name
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 获取模型名字
*输入参数 : PM_HANDLE handle 协议管理实例
*输出参数 :
*返回值: const char * 模型名字
*调用关系 :
*其它:
*************************************************************************/
const char * get_model_name(PM_HANDLE handle)
{
    const char *result;

    if (NULL == handle)
    {
        result = NULL;
    }
    else
    {
        result = handle->ModelName;
    }
    return result;
}

/*************************************************************************
*函数名 : get_next_send_time
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 获取下一个数据发送时间
*输入参数 : PM_HANDLE handle 协议管理实例
*输出参数 :
*返回值: TIME_TO_SEND * 需要发送数据的时间
*调用关系 :
*其它:
*************************************************************************/
TIME_TO_SEND * get_next_send_time(PM_HANDLE handle)
{
    TIME_TO_SEND *result;

    if (NULL == handle)
    {
        result = NULL;
    }
    else
    {
        const PM_DATA_INFO *pm_data_info = singlylinkedlist_item_get_value(handle->next_data_info);

        if (NULL == pm_data_info || NULL == pm_data_info->time_to_send)
        {
            result = NULL;
        }
        else
        {
            result = pm_data_info->time_to_send;

            handle->prev_data_info = handle->next_data_info;
            if (NULL == singlylinkedlist_get_next_item(handle->next_data_info))
            {
                handle->next_data_info = singlylinkedlist_get_head_item(handle->data_desc->dm_info_list);
                result->isLastOne = true;
            }
            else
            {
                handle->next_data_info = singlylinkedlist_get_next_item(handle->next_data_info);
                result->isLastOne = false;
            }
        }
    }
    return result;
}

/*************************************************************************
*函数名 : get_request_data
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 获取请求数据
*输入参数 : PM_HANDLE handle 协议管理实例
*输出参数 :
*返回值: DATA_REQUEST * 需要发送的数据
*调用关系 :
*其它:
*************************************************************************/
DATA_REQUEST * get_request_data(PM_HANDLE handle)
{
    DATA_REQUEST *result;

    if (NULL == handle)
    {
        result = NULL;
    }
    else
    {
        const PM_DATA_INFO *data_info = singlylinkedlist_item_get_value(handle->prev_data_info);

        if (NULL == data_info || NULL == data_info->data_request)
        {
            result = NULL;
        }
        else
        {
            handle->curr_data_info = handle->prev_data_info;
            result = data_info->data_request;
        }
    }
    return result;
}

/*************************************************************************
*函数名 : data_response_parse
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 解析返回的数据
*输入参数 : PM_HANDLE handle 协议管理实例
            DATA_RESPONSE * response 串口返回的数据
*输出参数 :
*返回值: char * 解析后的数据
*调用关系 :
*其它:
*************************************************************************/
DATA_PAYLOAD * data_response_parse(PM_HANDLE handle, DATA_RESPONSE *response)
{
    DATA_PAYLOAD *result;

    if (NULL == handle || NULL == response)
    {
        result = NULL;
    }
    else
    {
        DATA_DESCRIPTION *data_desc = handle->data_desc;

        if (NULL != data_desc)
        {
            const PM_DATA_INFO *data_info = singlylinkedlist_item_get_value(handle->curr_data_info);

            const void *model_info = data_info->model_info;

            if (NULL != data_info && NULL != model_info)
            {
                result = handle->interface->response_parse(model_info, data_info, response);
            }
        }
    }
    return result;
}

/*************************************************************************
*函数名 : protocol_destroy
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 销毁协议管理对象
*输入参数 : PM_HANDLE handle 协议管理实例
*输出参数 :
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
void protocol_destroy(PM_HANDLE handle)
{
    if (NULL == handle) return;
    handle->interface->Destroy(handle->data_desc);
    handle->data_desc = NULL;
    handle->next_data_info = NULL;
    handle->prev_data_info = NULL;
    handle->curr_data_info = NULL;
    free(handle);
    handle = NULL;
}


