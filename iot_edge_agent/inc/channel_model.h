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

#ifndef _CHANNEL_MODEL_H
#define _CHANNEL_MODEL_H


#ifdef __cplusplus
extern "C" {
#endif

#include <azure_c_shared_utility/singlylinkedlist.h>
#include "protocol_manager.h"
#include "parson.h"

#define MAX_SIZE_OF_HOST        (20)
#define MAX_SIZE_OF_PORT_TYPE   (20)
#define MAX_SIZE_OF_SERIAL_ID   (20)

#define MAX_SIZE_OF_DEVICE_NAME (64)
#define MAX_SIZE_OF_PROTO_TYPE  (64)
#define MAX_SIZE_OF_MODEL_TYPE  (64)


typedef struct DEVICE_PORT_TAG
{
    int baudRate;
    int dataBit;
    int stopBit;
    char parity;
    int port;
    char host[MAX_SIZE_OF_HOST + 1];
    char portType[MAX_SIZE_OF_PORT_TYPE + 1];
    char serialID[MAX_SIZE_OF_SERIAL_ID + 1];
} DEVICE_PORT_OPTION;   // 设备端口选项


typedef struct CHANNEL_MODEL_TAG
{
    int timeOut;
    char device[MAX_SIZE_OF_DEVICE_NAME + 1];
    char protoType[MAX_SIZE_OF_PROTO_TYPE + 1];
    char modelNmae[MAX_SIZE_OF_MODEL_TYPE + 1];
    DEVICE_PORT_OPTION portOptions;
} CHANNEL_MODEL_INFO;   // 通道模型信息

SINGLYLINKEDLIST_HANDLE channel_info_list_create(JSON_Value *channel_json);
void channel_info_list_destroy(SINGLYLINKEDLIST_HANDLE channel_list);

#ifdef __cplusplus
}
#endif

#endif
