/**************************************************
*- Author       : aゞ小波
*- CreateTime   : 2019-12-15 12:37
*- Email        : 465728296@qq.com
*- Filename     : protocol_manager.c
*- Description  :
***************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "protocol_manager.h"
#include "modbus_rtu.h"

PROTO_MGR_HANDLE proto_manager;

PROTO_MGR_HANDLE protocol_init(PROTO_TYPE proto_type)
{
    switch (proto_type) {
        case MODBUS_RTU:
            proto_manager = &modbus_rtu_proto_manager;
            break;
        case DLT645:
            break;
        case DLT698:
            break;
    }
    return proto_manager;
}



