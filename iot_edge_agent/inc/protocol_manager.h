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

#ifndef _PROTO_MANAGER_H
#define _PROTO_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "azure_c_shared_utility/tickcounter.h"
#include <azure_c_shared_utility/singlylinkedlist.h>

#include <stdint.h>
#include "parson.h"

#define MAX_SIZEOF_DBTOPIC (64)
// protocol type
typedef enum
{
    MODBUS_RTU,
    MODBUS_TCP,
    MODBUS_ASCII,
    DLT645,
    DLT698
} PROTO_TYPE;


typedef struct TIME_TO_SEND_TAG
{
    char acqMode[32 + 1];

    // Package send time
    tickcounter_ms_t packetSendTime;

    // Acquisition frequency
    int acqInt;

    // Acquisition start time
    struct
    {
        char Mons[12];     /* 0-11 */
        char Days[32];     /* 1-31 */
        char Hous[24];     /* 0-23 */
        char Mins[60];     /* 0-59 */
    } startTime;

    // Acquisition end time
    struct
    {
        char Mons[12];     /* 0-11 */
        char Days[32];     /* 1-31 */
        char Hous[24];     /* 0-23 */
        char Mins[60];     /* 0-59 */
    } endTime;

    bool freezing;
    bool isLastOne;
} TIME_TO_SEND;

//请求数据
typedef struct DATA_REQUEST_TAG
{
    uint8_t *data;
    uint8_t  length;
    uint8_t  predictedResponseLength;
} DATA_REQUEST;

//相应数据
typedef struct DATA_RESPONSE_TAG
{
    const uint8_t *data;
    uint8_t  length;
} DATA_RESPONSE;

// 上传数据库
typedef struct DATA_PAYLOAD_TAG
{
    char *payload;
    char  DBTopic[MAX_SIZEOF_DBTOPIC + 1];
} DATA_PAYLOAD;

//协议管理数据信息
typedef struct PM_DATA_INFO_TAG
{
    DATA_REQUEST *data_request;
    TIME_TO_SEND *time_to_send;
    void *model_info;
    void *expend_data;
} PM_DATA_INFO;

//数据描述
typedef struct
{
    SINGLYLINKEDLIST_HANDLE dm_info_list;
} DATA_DESCRIPTION;

typedef DATA_DESCRIPTION * (*CONSTRUCT)(JSON_Array *json_array);
typedef DATA_PAYLOAD * (*RESPONSE_PARSE) (const void *model_info, const PM_DATA_INFO *data_info,
    DATA_RESPONSE *data_response);
typedef void (*DESTROY)(DATA_DESCRIPTION *);

typedef struct
{
    CONSTRUCT Construct;
    DESTROY Destroy;
    RESPONSE_PARSE response_parse;
} INTER_DESCRIPTION;

typedef struct PM_INSTANCE_TAG *PM_HANDLE;

PM_HANDLE protocol_init(JSON_Object *json_object);
void protocol_destroy(PM_HANDLE handle);
DATA_REQUEST * get_request_data(PM_HANDLE handle);
TIME_TO_SEND * get_next_send_time(PM_HANDLE handle);
DATA_PAYLOAD * data_response_parse(PM_HANDLE handle, DATA_RESPONSE *response);
const char * get_model_name(PM_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif
