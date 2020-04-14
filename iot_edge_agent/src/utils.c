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
#include <sys/time.h>

#include "utils.h"

/*************************************************************************
*函数名 : reversebytes_uint32 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 反转uint32字节序
*输入参数 : uint32_t value
*输出参数 : 
*返回值: uint32_t
*调用关系 :
*其它:
*************************************************************************/
uint32_t reversebytes_uint32(uint32_t value)
{
    return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |
        (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;
}

/*************************************************************************
*函数名 : reversebytes_uint16 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 反转uint16字节序
*输入参数 : uint16_t value
*输出参数 : 
*返回值: uint16_t
*调用关系 :
*其它:
*************************************************************************/
uint16_t reversebytes_uint16(uint16_t value)
{
    return (value & 0x00FFU) << 8 | (value & 0xFF00U) >> 8;
}

/*************************************************************************
*函数名 : get_log_time 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 获取当前LOG时间
*输入参数 : 
*输出参数 : char* timeResult 返回时间字符串
            size_t len 字符串长度
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
void get_log_time(char *time_result, size_t len)
{
    if (time_result != NULL)
    {
        time_t agent_time = get_time(NULL);
        if (agent_time == (time_t) - 1)
        {
            time_result[0] = '\0';
        }
        else
        {
            struct tm *tm_info = localtime(&agent_time);
            if (tm_info == NULL)
            {
                time_result[0] = '\0';
            }
            else
            {
                if (strftime(time_result, len, "%H:%M:%S", tm_info) == 0)
                {
                    time_result[0] = '\0';
                }
            }
        }
    }
}

/*************************************************************************
*函数名 : get_current_json_time 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 获取当前时间
*输入参数 : 
*输出参数 : char* pJsonTime 输出时间字符串
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
void get_current_json_time(char *time_result)
{
    time_t current_time = time(NULL);

    snprintf(time_result, 12, "%ld", current_time);

	// char tmp[32];
    // char ms[32];
	// struct timeval tv;
	// struct tm *now;

	// gettimeofday(&tv, NULL);

	// now = localtime(&tv.tv_sec);

	// strftime(tmp, sizeof(tmp), "%Y-%m-%dT%H:%M:%S",now);
	// snprintf(ms, 32, ".%03ld", tv.tv_usec / 1000);
	// strncat(tmp, ms, strlen(ms));
	// strncat(tmp, "+0800", strlen("+0800"));

	// if(NULL != time_result)
	// {
	// 	strncpy(time_result, tmp, 32);
	// }

	return;
}

/*************************************************************************
*函数名 : get_time_by_model 
*负责人 : 闵波bmin
*创建日期 : 2019年12月18日
*函数功能 : 根据模型获取当前时间
*输入参数 : int mons, int days, int hour, int mins
*输出参数 : char* time_result 输出时间字符串
*返回值: void
*调用关系 :
*其它:
*************************************************************************/
void get_time_by_model(int mons, int days, int hour, int mins, char* time_result)
{
	char tmp[32];
    char ms[32];
	struct timeval tv;
	struct tm *time_info;

	gettimeofday(&tv, NULL);

	time_info = localtime(&tv.tv_sec);
    time_info->tm_mon = mons;
    time_info->tm_mday = days;
    time_info->tm_hour = hour;
    time_info->tm_min = mins;
    time_info->tm_sec = 0;
    
	strftime(tmp, sizeof(tmp), "%Y-%m-%dT%H:%M:%S",time_info);
	snprintf(ms, 32, ".%03ld", tv.tv_usec / 1000);
	strncat(tmp, ms, strlen(ms));
	strncat(tmp, "+0800", strlen("+0800"));

	if(NULL != time_result)
	{
		strncpy(time_result, tmp, 32);
	}

	return;
}