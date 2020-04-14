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

#ifndef _COMMON_INCLUDE_H
#define _COMMON_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define TOPIC_SIZE          3

#define MODEL_SUB_TOPIC     "/ModelManager/get/reponse/%s/Southmodel"
#define MODEL_PUB_TOPIC     "/%s/get/request/ModelManager/Southmodel"

#define DEVICE_LIST_SUB     "/devMgr/get/response/%s/devlist"
#define DEVICE_LIST_PUB     "/%s/get/request/devMgr/devlist"

#define DEVICE_INFO_SUB     "/devMgr/get/response/%s/dev"
#define DEVICE_INFO_PUB     "/%s/get/request/devMgr/dev"

#define M               "m"
#define DF              "df"
#define FN              "fn"
#define ID              "id"
#define BIG             "big"
#define DEV             "dev"
#define MATH            "math"
#define PORT            "port"
#define HOST            "ip"
#define PARITY          "parity"
#define DATABIT         "dataBit"
#define STOPBIT         "stopBit"
#define REGGRP          "regGrp"
#define ACQINT          "acqInt"
#define ACQFILE         "AcqFile"
#define ADDR            "address"
#define TIMEOUT         "timeOut"
#define DEVADDR         "devAddr"
#define MODETYPE        "modeltype"
#define RELMODEL        "relmodel"
#define BANDRATE        "baudRate"
#define PORTTYPE        "portType"
#define SERIALID        "serialID"
#define PROTOTYPE       "protoType"
#define MODELNAME       "modelNmae"
#define STARTTIME       "startTime"
#define USEINTERVAL     "useInterval"

#ifdef __cplusplus
}
#endif

#endif
