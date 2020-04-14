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

#ifndef _DLT_698_H
#define _DLT_698_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "parson.h"
#include "protocol_manager.h"

#define MAX_SIZE_OF_M       (32)
#define MAX_SIZE_OF_DF      (8)
#define MAX_SIZE_OF_OAD     (8)
#define MAX_SIZE_OF_MATH    (32)
#define MAX_SIZE_OF_ADDR    (16)
#define MAX_SIZE_OF_MODE    (32)
#define MAX_SIZE_OF_DBTOPIC (64)

typedef struct
{
    uint8_t id;
    uint8_t big;

    uint8_t oad[4];
    uint8_t addr[6];
    char m[MAX_SIZE_OF_M + 1];
    char math[MAX_SIZE_OF_MATH + 1];
} ACQFILE_TAG;

typedef struct
{
    struct
    {
        int InvMons;
        int InvDays;
        int InvHous;
        int InvMins;
    } startTime;

    struct
    {
        int InvMons;
        int InvDays;
        int InvHous;
        int InvMins;
    } endTime;
    
    char acqMode[MAX_SIZE_OF_MODE + 1];
    char DBTopic[MAX_SIZE_OF_DBTOPIC + 1];
    int acqInt;
    ACQFILE_TAG acqFile;
} DLT698_MODEL_INFO;

const INTER_DESCRIPTION * dlt698_get_interface_description(void);
DATA_DESCRIPTION * dlt698_init(JSON_Array *model_json);
void dlt698_destroy(DATA_DESCRIPTION *handle);


#ifdef __cplusplus
}
#endif

#endif