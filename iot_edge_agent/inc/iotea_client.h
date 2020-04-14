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

#ifndef _EDGE_AGENT_H
#define _EDGE_AGENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common_include.h"
#include "iothub_mqtt_client.h"
#include "protocol_manager.h"

//解析通道信息回调函数定义
typedef int (*CHANNEL_MODEL_PARSE_CALLBACK) (const char * json_string);

//解析物模型回调函数定义
typedef int (*THING_MODEL_PARSE_CALLBACK) (const char * json_string);


typedef struct MODEL_PARSE_CALLBACK_TAG
{
    THING_MODEL_PARSE_CALLBACK ThingModelParse;
    CHANNEL_MODEL_PARSE_CALLBACK ChannelModelParse;
} MODEL_PARSE_CALLBACK;


typedef struct IOTEA_CLIENT_TAG* IOTEA_CLIENT_HANDLE;

//边缘物联代理客户端参数选项
typedef struct IOTEA_CLIENT_OPTIONS_TAG
{
    bool cleanSession;

    char* clientId; //mqtt ID
    char* username; //mqtt 用户名
    char* password; //mqtt 密码

    uint16_t keepAliveInterval; //心跳时间

    size_t retryTimeoutInSeconds;   //断线重试时间

} IOTEA_CLIENT_OPTIONS;


IOTHUB_MQTT_CLIENT_HANDLE get_mqtt_client_handle(IOTEA_CLIENT_HANDLE handle);
void iotea_client_register_thing_model_init(IOTEA_CLIENT_HANDLE handle, THING_MODEL_PARSE_CALLBACK callback);
void iotea_client_register_channel_model_init(IOTEA_CLIENT_HANDLE handle, CHANNEL_MODEL_PARSE_CALLBACK callback);
IOTEA_CLIENT_HANDLE iotea_client_init(char *broker, char *name, char *type);
void iotea_client_deinit(IOTEA_CLIENT_HANDLE handle);
int iotea_client_connect(IOTEA_CLIENT_HANDLE handle, const IOTEA_CLIENT_OPTIONS *options);
int iotea_client_dowork(const IOTEA_CLIENT_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif
