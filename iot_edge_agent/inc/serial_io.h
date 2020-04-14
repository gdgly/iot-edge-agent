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

#ifndef _SERIALIO_H
#define _SERIALIO_H

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <sys/time.h>
#include <stddef.h>
#endif /* __cplusplus */

#define MAX_SIZE_OF_INTER_NAME  (64)

// 串口参数
typedef struct SERIALIO_CONFIG_TAG
{
    char interfaceName[MAX_SIZE_OF_INTER_NAME + 1];
    int baudRate;
    int dataBits;
    char parity;
    int stopBits;
    struct timeval timeout;
} SERIALIO_CONFIG;

#define MAX_RECEIVE_BYTES     (256)

CONCRETE_IO_HANDLE serialio_create(void *io_create_parameters);

void serialio_destroy(CONCRETE_IO_HANDLE serial_io);

int serialio_open(CONCRETE_IO_HANDLE serial_io, ON_IO_OPEN_COMPLETE on_io_open_complete, 
	void *on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, 
	void *on_bytes_received_context, ON_IO_ERROR on_io_error, void *on_io_error_context);

int serialio_close(CONCRETE_IO_HANDLE serial_io, ON_IO_CLOSE_COMPLETE on_io_close_complete,
 	void *callback_context);

int serialio_send(CONCRETE_IO_HANDLE serial_io, const void *buffer, size_t size,
 	ON_SEND_COMPLETE on_send_complete, void *callback_context);

void serialio_dowork(CONCRETE_IO_HANDLE serial_io);
int serialio_setoption(CONCRETE_IO_HANDLE serial_io, const char *optionName, const void *value);

const IO_INTERFACE_DESCRIPTION * serialio_get_interface_description(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SOCKETIO_H */
