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
#include <stddef.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/tcpsocketconnection_c.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"

#include "serial_io.h"

#define INVALID_SERIAL_FD       -1

// IO state
typedef enum IO_STATE_TAG
{
    IO_STATE_CLOSED,
    IO_STATE_OPENING,
    IO_STATE_OPEN,
    IO_STATE_CLOSING,
    IO_STATE_ERROR
} IO_STATE;

typedef struct PENDING_SERIAL_IO_TAG
{
    unsigned char* bytes;   //send buffer
    size_t size;            //buffer size
    ON_SEND_COMPLETE on_send_complete;  //callback
    void* callback_context;             //context
    SINGLYLINKEDLIST_HANDLE pending_io_list;
} PENDING_SERIAL_IO;

// serial IO object
typedef struct SERIAL_IO_INSTANCE_TAG
{
    int serial_fd;

    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_ERROR on_io_error;
    void* on_bytes_received_context;
    void* on_io_error_context;

    char interfaceName[MAX_SIZE_OF_INTER_NAME + 1];
    int baudRate;
    int dataBits;
    char parity;
    int stopBits;
    struct timeval timeout;

    IO_STATE io_state;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
    unsigned char recv_bytes[MAX_RECEIVE_BYTES];
} SERIAL_IO_INSTANCE;

/*this function will clone an option given by name and value*/
static void* serialio_clone_option(const char *name, const void *value)
{
    // (void)name;
    // (void)value;
    return NULL;
}

/*this function destroys an option previously created*/
static void serialio_destroy_option(const char *name, const void *value)
{
    // (void)name;
    // (void)value;
}

/*************************************************************************
*函数名 : serialio_retrieveoptions 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 获取串口参数选项
*输入参数 : CONCRETE_IO_HANDLE serial_io serial实例
*输出参数 : 
*返回值: OPTIONHANDLER_HANDLE
*调用关系 :
*其它:
*************************************************************************/
static OPTIONHANDLER_HANDLE serialio_retrieveoptions(CONCRETE_IO_HANDLE serial_io)
{
    OPTIONHANDLER_HANDLE result;
    (void)serial_io;
    result = OptionHandler_Create(serialio_clone_option, serialio_destroy_option, serialio_setoption);
    if (result == NULL)
    {
        /*return as is*/
    }
    else
    {
        /*insert here work to add the options to "result" handle*/
    }
    return result;
}


static const IO_INTERFACE_DESCRIPTION serial_io_interface_description =
{
    serialio_retrieveoptions,
    serialio_create,
    serialio_destroy,
    serialio_open,
    serialio_close,
    serialio_send,
    serialio_dowork,
    serialio_setoption
};

/*************************************************************************
*函数名 : indicate_error 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 处理错误
*输入参数 : SERIAL_IO_INSTANCE *serial_io_instance serial实例
*输出参数 : 
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
static void indicate_error(SERIAL_IO_INSTANCE *serial_io_instance)
{
    if (serial_io_instance->on_io_error != NULL)
    {
        serial_io_instance->on_io_error(serial_io_instance->on_io_error_context);
    }
}

/*************************************************************************
*函数名 : add_pending_io 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 添加IO到链表
*输入参数 : SERIAL_IO_INSTANCE *serial_io_instance serial实例
            const unsigned char *buffer 缓冲数据
            size_t size 数据大小
            ON_SEND_COMPLETE on_send_complete 发送后的回调
            void *callback_context  回调函数参数
*输出参数 : 
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
static int add_pending_io(SERIAL_IO_INSTANCE *serial_io_instance, const unsigned char *buffer, 
    size_t size, ON_SEND_COMPLETE on_send_complete, void *callback_context)
{
    int result;

    PENDING_SERIAL_IO *pending_serial_io = (PENDING_SERIAL_IO*)malloc(sizeof(PENDING_SERIAL_IO));
    if (pending_serial_io == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        pending_serial_io->bytes = (unsigned char*)malloc(size);
        if (pending_serial_io->bytes == NULL)
        {
            free(pending_serial_io);
            pending_serial_io = NULL;
            result = __FAILURE__;
        }
        else
        {
            pending_serial_io->size = size;
            pending_serial_io->on_send_complete = on_send_complete;
            pending_serial_io->callback_context = callback_context;
            pending_serial_io->pending_io_list = serial_io_instance->pending_io_list;
            (void)memcpy(pending_serial_io->bytes, buffer, size);

            if (singlylinkedlist_add(serial_io_instance->pending_io_list, pending_serial_io) == NULL)
            {
                free(pending_serial_io->bytes);
                pending_serial_io->bytes = NULL;
                free(pending_serial_io);
                pending_serial_io = NULL;
                result = __FAILURE__;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

/*************************************************************************
*函数名 : serialio_create 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 创建串口
*输入参数 : void *io_create_parameters serial参数
*输出参数 : 
*返回值: CONCRETE_IO_HANDLE
*调用关系 :
*其它:
*************************************************************************/
CONCRETE_IO_HANDLE serialio_create(void *io_create_parameters)
{
    SERIALIO_CONFIG *serial_io_config = io_create_parameters;
    SERIAL_IO_INSTANCE *result;

    if (serial_io_config == NULL)
    {
        result = NULL;
    }
    else
    {
        result = malloc(sizeof(SERIAL_IO_INSTANCE));
        if (result != NULL)
        {
            result->pending_io_list = singlylinkedlist_create();
            if (result->pending_io_list == NULL)
            {
                free(result);
                result = NULL;
            }
            else
            {

                // set options
                strncpy(result->interfaceName, serial_io_config->interfaceName, MAX_SIZE_OF_INTER_NAME);
                result->parity = serial_io_config->parity;
                result->baudRate = serial_io_config->baudRate;
                result->stopBits = serial_io_config->stopBits;
                result->dataBits = serial_io_config->dataBits;
                result->timeout = serial_io_config->timeout;

                result->on_bytes_received = NULL;
                result->on_io_error = NULL;
                result->on_bytes_received_context = NULL;
                result->on_io_error_context = NULL;
                result->io_state = IO_STATE_CLOSED;
                result->serial_fd = INVALID_SERIAL_FD;
            }
        }
    }

    return result;
}

/*************************************************************************
*函数名 : serialio_create 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 销毁串口IO
*输入参数 : CONCRETE_IO_HANDLE serial_io serial实例
*输出参数 : 
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
void serialio_destroy(CONCRETE_IO_HANDLE serial_io)
{
    if (serial_io != NULL)
    {
        SERIAL_IO_INSTANCE *serial_io_instance = (SERIAL_IO_INSTANCE*)serial_io;

        if (serial_io_instance->serial_fd != INVALID_SERIAL_FD)
        {
            close(serial_io_instance->serial_fd);
        }

        /* clear all pending IOs */
        LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(serial_io_instance->pending_io_list);
        while (first_pending_io != NULL)
        {
            PENDING_SERIAL_IO *pending_serial_io = 
                (PENDING_SERIAL_IO*)singlylinkedlist_item_get_value(first_pending_io);
            if (pending_serial_io != NULL)
            {
                free(pending_serial_io->bytes);
                pending_serial_io->bytes = NULL;
                free(pending_serial_io);
                pending_serial_io = NULL;
            }

            (void)singlylinkedlist_remove(serial_io_instance->pending_io_list, first_pending_io);
            first_pending_io = singlylinkedlist_get_head_item(serial_io_instance->pending_io_list);
        }

        singlylinkedlist_destroy(serial_io_instance->pending_io_list);
        free(serial_io);
        serial_io = NULL;
    }
}

/*************************************************************************
*函数名 : serialio_open 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 打开串口IO
*输入参数 : CONCRETE_IO_HANDLE serial_io serial实例
            ON_IO_OPEN_COMPLETE on_io_open_complete io打开的回调
            void *on_io_open_complete_context   回调参数
            ON_BYTES_RECEIVED on_bytes_received 接收到数据的回调
            void *on_bytes_received_context 回调参数
            ON_IO_ERROR on_io_error 发生错误的回调
            void* on_io_error_context 回参数
*输出参数 : 
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
int serialio_open(CONCRETE_IO_HANDLE serial_io, ON_IO_OPEN_COMPLETE on_io_open_complete, 
    void *on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, 
    void *on_bytes_received_context, ON_IO_ERROR on_io_error, void *on_io_error_context)
{
    int result;

    SERIAL_IO_INSTANCE *serial_io_instance = (SERIAL_IO_INSTANCE*)serial_io;
    if (serial_io == NULL)
    {
        LogError("Invalid argument: SERIAL_IO_INSTANCE is NULL");
        result = __FAILURE__;
    }
    else
    {
        if (serial_io_instance->io_state != IO_STATE_CLOSED)
        {
            LogError("Failure: serial state is not closed.");
            result = __FAILURE__;
        }
        else if (serial_io_instance->serial_fd != INVALID_SERIAL_FD)
        {
            // Opening an accepted socket
            serial_io_instance->on_bytes_received_context = on_bytes_received_context;
            serial_io_instance->on_bytes_received = on_bytes_received;
            serial_io_instance->on_io_error = on_io_error;
            serial_io_instance->on_io_error_context = on_io_error_context;
            serial_io_instance->io_state = IO_STATE_OPEN;
            result = 0;
        }
        else
        {
            serial_io_instance->serial_fd = open(serial_io_instance->interfaceName, O_RDWR | O_NOCTTY | O_NDELAY);

            if (serial_io_instance->serial_fd < INVALID_SERIAL_FD)
            {
                LogError("Failure: serial create failure %d.", serial_io_instance->serial_fd);
                result = __FAILURE__;
            }
            else
            {

                // 判断是不是终端
                if( !isatty(serial_io_instance->serial_fd) )
                {
                    close(serial_io_instance->serial_fd);
                    serial_io_instance->serial_fd = INVALID_SERIAL_FD;
                    LogError("Failure: Is Not A Tty Device");
                    result = __FAILURE__;
                }
                else
                {

                    // 设置非阻塞
                    if( fcntl(serial_io_instance->serial_fd, F_SETFL, 0) == -1 )
                    {
                        close(serial_io_instance->serial_fd);
                        serial_io_instance->serial_fd = INVALID_SERIAL_FD;
                        LogError("Failure: fcntl Set Error");
                        result = __FAILURE__;
                    }
                    else
                    {
                        struct termios tios;
                        speed_t baudrate;

                        tcgetattr(serial_io_instance->serial_fd, &tios);

                        // set baudRate
                        switch (serial_io_instance->baudRate)
                        {
                        case 110:
                            baudrate = B110;
                            break;
                        case 300:
                            baudrate = B300;
                            break;
                        case 600:
                            baudrate = B600;
                            break;
                        case 1200:
                            baudrate = B1200;
                            break;
                        case 2400:
                            baudrate = B2400;
                            break;
                        case 4800:
                            baudrate = B4800;
                            break;
                        case 9600:
                            baudrate = B9600;
                            break;
                        case 19200:
                            baudrate = B19200;
                            break;
                        case 38400:
                            baudrate = B38400;
                            break;
                        case 57600:
                            baudrate = B57600;
                            break;
                        case 115200:
                            baudrate = B115200;
                            break;
                        default:
                            baudrate = B9600;
                        }

                        if ((cfsetispeed(&tios, baudrate) < 0) || (cfsetospeed(&tios, baudrate) < 0))
                        {
                            close(serial_io_instance->serial_fd);
                            serial_io_instance->serial_fd = INVALID_SERIAL_FD;
                            LogError("Failure: cfsetispeed Error");
                            result = __FAILURE__;
                        }
                        else
                        {
                            tios.c_cflag |= (CREAD | CLOCAL);
                            tios.c_cflag &= ~CSIZE;

                            switch (serial_io_instance->dataBits)
                            {
                            case 5:
                                tios.c_cflag |= CS5;
                                break;
                            case 6:
                                tios.c_cflag |= CS6;
                                break;
                            case 7:
                                tios.c_cflag |= CS7;
                                break;
                            case 8:
                            default:
                                tios.c_cflag |= CS8;
                                break;
                            }

                            if (serial_io_instance->stopBits == 1)
                                tios.c_cflag &= ~CSTOPB;
                            else /* 2 */
                                tios.c_cflag |= CSTOPB;

                            if (serial_io_instance->parity == 'N') 
                            {
                                tios.c_cflag &= ~PARENB;
                            } 
                            else if (serial_io_instance->parity == 'E') 
                            {
                                tios.c_cflag |= PARENB;
                                tios.c_cflag &= ~PARODD;
                            } 
                            else 
                            { /* 'O' */
                                tios.c_cflag |= PARENB;
                                tios.c_cflag |= PARODD;
                            }
                            tios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

                            if (serial_io_instance->parity == 'N') 
                            {
                                tios.c_iflag &= ~INPCK;
                            } 
                            else 
                            {
                                tios.c_iflag |= INPCK;
                            }
                            tios.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL);
                            tios.c_iflag |= IGNBRK; /* Set ignore break to allow 0xff characters */
                            tios.c_iflag |= IGNPAR;
                            tios.c_oflag &= ~OPOST;

                            tios.c_cc[VMIN] = 0;
                            tios.c_cc[VTIME] = 1;

                            tcflush(serial_io_instance->serial_fd, TCIFLUSH);

                            if (tcsetattr(serial_io_instance->serial_fd, TCSANOW, &tios) < 0)
                            {
                                close(serial_io_instance->serial_fd);
                                serial_io_instance->serial_fd = INVALID_SERIAL_FD;
                                LogError("Failure: tcsetattr Error");
                                result = __FAILURE__;
                            }
                            else
                            {
                                serial_io_instance->on_bytes_received = on_bytes_received;
                                serial_io_instance->on_bytes_received_context = on_bytes_received_context;

                                serial_io_instance->on_io_error = on_io_error;
                                serial_io_instance->on_io_error_context = on_io_error_context;

                                serial_io_instance->io_state = IO_STATE_OPEN;

                                result = 0;
                            } 
                        }
                    }
                }
            }
        }
    }
    
    if (on_io_open_complete != NULL)
    {
        on_io_open_complete(on_io_open_complete_context, result == 0 ? IO_OPEN_OK : IO_OPEN_ERROR);
    }

    return result;
}

/*************************************************************************
*函数名 : serialio_close 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 关闭串口
*输入参数 : CONCRETE_IO_HANDLE serial_io serial实例
            ON_IO_CLOSE_COMPLETE on_io_close_complete io关闭的回调
            void *callback_context   回调参数
*输出参数 : 
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
int serialio_close(CONCRETE_IO_HANDLE serial_io, ON_IO_CLOSE_COMPLETE on_io_close_complete,
    void *callback_context)
{
    int result = 0;

    if (serial_io == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        SERIAL_IO_INSTANCE *serial_io_instance = (SERIAL_IO_INSTANCE*)serial_io;

        if ((serial_io_instance->io_state != IO_STATE_CLOSED) && (serial_io_instance->io_state != IO_STATE_CLOSING))
        {
            close(serial_io_instance->serial_fd);
            serial_io_instance->serial_fd = INVALID_SERIAL_FD;
            serial_io_instance->io_state = IO_STATE_CLOSED;
        }

        if (on_io_close_complete != NULL)
        {
            on_io_close_complete(callback_context);
        }

        result = 0;
    }

    return result;
}

/*************************************************************************
*函数名 : serialio_send 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 添加IO到链表
*输入参数 : CONCRETE_IO_HANDLE serial_io serial实例
            ON_IO_CLOSE_COMPLETE on_io_close_complete io关闭的回调
            void *callback_context   回调参数
*输出参数 : 
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
int serialio_send(CONCRETE_IO_HANDLE serial_io, const void *buffer, size_t size, 
    ON_SEND_COMPLETE on_send_complete, void *callback_context)
{
    int result;

    if ((serial_io == NULL) ||
        (buffer == NULL) ||
        (size == 0))
    {
        /* Invalid arguments */
        result = __FAILURE__;
    }
    else
    {
        SERIAL_IO_INSTANCE *serial_io_instance = (SERIAL_IO_INSTANCE*)serial_io;
        if (serial_io_instance->io_state != IO_STATE_OPEN)
        {
            LogError("Failure: serial state is not opened.");
            result = __FAILURE__;
        }
        else
        {
            LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(serial_io_instance->pending_io_list);
            if (first_pending_io != NULL)
            {
                if (add_pending_io(serial_io_instance, buffer, size, on_send_complete, 
                    callback_context) != 0)
                {
                    LogError("Failure: add_pending_io failed.");
                    result = __FAILURE__;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                int send_result = write(serial_io_instance->serial_fd, buffer, size);
                if (send_result != size)
                {
                    if (send_result == INVALID_SERIAL_FD)
                    {
                        if (errno == EAGAIN) 

                        /*send says "come back later" with EAGAIN - likely the socket buffer cannot accept more data*/
                        {
                            /*do nothing*/
                            result = 0;
                        }
                        else
                        {
                            LogError("Failure: sending socket failed. errno=%d (%s).", errno, strerror(errno));
                            result = __FAILURE__;
                        }
                    }
                    else
                    {
                        /* queue data */
                        if (add_pending_io(serial_io_instance, buffer + send_result, 
                            size - send_result, on_send_complete, callback_context) != 0)
                        {
                            LogError("Failure: add_pending_io failed.");
                            result = __FAILURE__;
                        }
                        else
                        {
                            result = 0;
                        }
                    }
                }
                else
                {
                    if (on_send_complete != NULL)
                    {
                        on_send_complete(callback_context, IO_SEND_OK);
                    }

                    result = 0;
                }
            }
        }
    }

    return result;
}

/*************************************************************************
*函数名 : serialio_dowork 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : IO读写
*输入参数 : CONCRETE_IO_HANDLE serial_io serial实例
*输出参数 : 
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
void serialio_dowork(CONCRETE_IO_HANDLE serial_io)
{
    if (serial_io != NULL)
    {
        SERIAL_IO_INSTANCE *serial_io_instance = (SERIAL_IO_INSTANCE*)serial_io;
        if (serial_io_instance->io_state == IO_STATE_OPEN)
        {     
            int received = 1;

            // get queue
            LIST_ITEM_HANDLE first_pending_io = 
                singlylinkedlist_get_head_item(serial_io_instance->pending_io_list);
            while (first_pending_io != NULL)
            {
                PENDING_SERIAL_IO *pending_serial_io = 
                    (PENDING_SERIAL_IO*)singlylinkedlist_item_get_value(first_pending_io);
                if (pending_serial_io == NULL)
                {
                    serial_io_instance->io_state = IO_STATE_ERROR;
                    indicate_error(serial_io_instance);
                    LogError("Failure: retrieving serial from list");
                    break;
                }

                int send_result = write(serial_io_instance->serial_fd, 
                    (const char*)pending_serial_io->bytes, pending_serial_io->size);
                if (send_result != pending_serial_io->size)
                {
                    if (send_result < 0)
                    {
                        if (send_result < INVALID_SERIAL_FD)
                        {
                            // Bad error.  Indicate as much.
                            serial_io_instance->io_state = IO_STATE_ERROR;
                            indicate_error(serial_io_instance);
                        }
                        break;
                    }
                    else
                    {
                        /* send something, wait for the rest */
                        (void)memmove(pending_serial_io->bytes, pending_serial_io->bytes + send_result, 
                            pending_serial_io->size - send_result);
                    }
                }
                else
                {
                    if (pending_serial_io->on_send_complete != NULL)
                    {
                        pending_serial_io->on_send_complete(pending_serial_io->callback_context, 
                            IO_SEND_OK);
                    }

                    free(pending_serial_io->bytes);
                    pending_serial_io->bytes = NULL;
                    free(pending_serial_io);
                    pending_serial_io = NULL;
                    if (singlylinkedlist_remove(serial_io_instance->pending_io_list, first_pending_io) != 0)
                    {
                        serial_io_instance->io_state = IO_STATE_ERROR;
                        indicate_error(serial_io_instance);
                        LogError("Failure: unable to remove serial from list");
                    }
                }

                first_pending_io = singlylinkedlist_get_head_item(serial_io_instance->pending_io_list);
            }

            if (serial_io_instance->io_state == IO_STATE_OPEN)
            {
                fd_set set;
                FD_ZERO(&set);
                FD_SET(serial_io_instance->serial_fd, &set);

                // select
                int ret = select(serial_io_instance->serial_fd + 1, &set, NULL, NULL, &serial_io_instance->timeout);
                
                if (ret == -1)
                {
                    indicate_error(serial_io_instance);
                }
                else if (ret > 0)
                {   
                    unsigned char *temp_buf = serial_io_instance->recv_bytes;
                    int nbytes = 0;
                    int left = MAX_RECEIVE_BYTES;
                    int received = 0;

                    do
                    {
                        nbytes = read(serial_io_instance->serial_fd, temp_buf, MAX_RECEIVE_BYTES);

                        if (nbytes > 0)
                        {
                            temp_buf += nbytes;
                            received += nbytes;
                            left -= nbytes;
                            continue;
                        }
                        else if (nbytes == 0)
                        {
                            break;
                        }
                        else if (nbytes < 0 && errno != EAGAIN)
                        {
                            LogError("Serialio_Failure: Receiving data from endpoint: errno=%d.", errno);
                            indicate_error(serial_io_instance);
                        }

                    } while (nbytes > 0 && left > 0 && serial_io_instance->io_state == IO_STATE_OPEN);

                    if (serial_io_instance->on_bytes_received != NULL && received != 0)
                    {
                        /* Explicitly ignoring here the result of the callback */
                        (void)serial_io_instance->on_bytes_received(serial_io_instance->on_bytes_received_context, 
                            serial_io_instance->recv_bytes, received);
                    }
                }
            }
        }
    }
}

/*************************************************************************
*函数名 : serialio_setoption 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 设置串口参数
*输入参数 : CONCRETE_IO_HANDLE serial_io serial实例
            const char *optionName 选项名字
            const void *value 值
*输出参数 : 
*返回值: int
*调用关系 :
*其它:
*************************************************************************/
int serialio_setoption(CONCRETE_IO_HANDLE serial_io, const char *optionName, const void *value)
{
    /* Not implementing any options */
    return __FAILURE__;
}

/*************************************************************************
*函数名 : serialio_get_interface_description 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 设置串口参数
*输入参数 : void
*输出参数 : 
*返回值: const IO_INTERFACE_DESCRIPTION* 接口描述
*调用关系 :
*其它:
*************************************************************************/
const IO_INTERFACE_DESCRIPTION* serialio_get_interface_description(void)
{
    return &serial_io_interface_description;
}

