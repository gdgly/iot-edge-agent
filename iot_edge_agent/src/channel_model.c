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
#include "azure_c_shared_utility/crt_abstractions.h"

#include "common_include.h"
#include "parson.h"
#include "channel_model.h"

/*************************************************************************
*函数名 : channel_info_list_create
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 根据json对象初始化通道信息链表
*输入参数 : JSON_Value * json_value 通道信息json数据
*输出参数 :
*返回值: SINGLYLINKEDLIST_HANDLE 通道信息链表
*调用关系 :
*其它:
*************************************************************************/
SINGLYLINKEDLIST_HANDLE channel_info_list_create(JSON_Value *channel_json)
{
    SINGLYLINKEDLIST_HANDLE result;

    if (NULL == channel_json)
    {
        result = NULL;
        return result;
    }

    JSON_Object *channel_object = json_value_get_object(channel_json);

    if (NULL == channel_object)
    {
        result = NULL;
        return result;
    }

    //  不要忘了检查token

    JSON_Array  *channel_body = json_object_dotget_array(channel_object, "body");

    if (NULL == channel_body)
    {
        result = NULL;
        return result;
    }

    result = singlylinkedlist_create();

    if (NULL == result)
    {
        LogError("Channel_Info singlylinkedlist_create Failure");
    }
    else
    {
        CHANNEL_MODEL_INFO *channel_info;

        int device_count = json_array_get_count(channel_body);

        JSON_Object *array;
        JSON_Object *port;

        int error = 0;
        const char *res_str = NULL;

        for (int i = 0; i < device_count; i++)
        {
            array = json_array_get_object(channel_body, i);

            channel_info = malloc(sizeof(CHANNEL_MODEL_INFO));

            if (NULL == channel_info)
            {
                LogError("Alloc DEVICE_CHANNEL_INFO failed");
                error = -1;
            }
            else
            {
                res_str = json_object_dotget_string(array, TIMEOUT);
                if (NULL == res_str)
                    error = -1;
                else
                    channel_info->timeOut = atoi(res_str);

                res_str = json_object_dotget_string(array, DEV);
                if (NULL == res_str)
                    error = -1;
                else
                    strncpy(channel_info->device, res_str, MAX_SIZE_OF_DEVICE_NAME);

                res_str = json_object_dotget_string(array, PROTOTYPE);
                if (NULL == res_str)
                    error = -1;
                else
                    strncpy(channel_info->protoType, res_str, MAX_SIZE_OF_PROTO_TYPE);

                res_str = json_object_dotget_string(array, MODETYPE);
                if (NULL == res_str)
                    error = -1;
                else
                    strncpy(channel_info->modelNmae, res_str, MAX_SIZE_OF_MODEL_TYPE);

                port = json_object_dotget_object(array, "port");

                if (NULL == port)
                {
                    LogError("Get Port failed");
                    error = -1;
                }
                else
                {
                    res_str = json_object_dotget_string(port, BANDRATE);
                    if (NULL == res_str)
                        error = -1;
                    else
                        channel_info->portOptions.baudRate = atoi(res_str);

                    res_str = json_object_dotget_string(port, STOPBIT);
                    if (NULL == res_str)
                        error = -1;
                    else
                        channel_info->portOptions.stopBit = atoi(res_str);

                    res_str = json_object_dotget_string(port, DATABIT);
                    if (NULL == res_str)
                        error = -1;
                    else
                        channel_info->portOptions.dataBit = atoi(res_str);

                    res_str = json_object_dotget_string(port, PORT);
                    if (NULL == res_str)
                        error = -1;
                    else
                        channel_info->portOptions.port = atoi(res_str);

                    res_str = json_object_dotget_string(port, HOST);
                    if (NULL == res_str)
                        error = -1;
                    else
                        strncpy(channel_info->portOptions.host, res_str, MAX_SIZE_OF_HOST);

                    res_str = json_object_dotget_string(port, SERIALID);
                    if (NULL == res_str)
                        error = -1;
                    else
                        strncpy(channel_info->portOptions.serialID, res_str, MAX_SIZE_OF_SERIAL_ID);

                    res_str = json_object_dotget_string(port, PORTTYPE);
                    if (NULL == res_str)
                        error = -1;
                    else
                        strncpy(channel_info->portOptions.portType, res_str, MAX_SIZE_OF_PORT_TYPE);

                    if (strcmp("NONE", json_object_dotget_string(port, PARITY)) == 0)
                    {
                        channel_info->portOptions.parity = 'N';
                    }
                    else if (strcmp("EVEN", json_object_dotget_string(port, PARITY)) == 0)
                    {
                        channel_info->portOptions.parity = 'E';
                    }
                    else if (strcmp("ODD", json_object_dotget_string(port, PARITY)) == 0)
                    {
                        channel_info->portOptions.parity = 'O';
                    }
                    else
                    {
                        LogError("Error: Parity Arg error");
                        error = -1;
                    }
                }
            }

            if (error != 0 || singlylinkedlist_add(result, channel_info) == NULL)
            {
                free(channel_info);
                channel_info = NULL;
                channel_info_list_destroy(result);
                result = NULL;
                break;
            }
        }
    }
    return result;
}

/*************************************************************************
*函数名 : channel_info_list_destroy
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 销毁通道信息链表
*输入参数 : SINGLYLINKEDLIST_HANDLE channel_list 通道信息链表
*输出参数 :
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
void channel_info_list_destroy(SINGLYLINKEDLIST_HANDLE channel_list)
{
    if (NULL == channel_list) return;

    LIST_ITEM_HANDLE channel_item = singlylinkedlist_get_head_item(channel_list);

    while (channel_item != NULL)
    {
        CHANNEL_MODEL_INFO *channel_info = (CHANNEL_MODEL_INFO*)singlylinkedlist_item_get_value(channel_item);
        if (channel_info != NULL)
        {
            free(channel_info);
            channel_info = NULL;
        }
        singlylinkedlist_remove(channel_list, channel_item);
        channel_item = singlylinkedlist_get_head_item(channel_list);
    }

    singlylinkedlist_destroy(channel_list);
}

