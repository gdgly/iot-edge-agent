/**************************************************
*- Author       : aゞ小波
*- CreateTime   : 2019-12-15 13:28
*- Email        : 465728296@qq.com
*- Filename     : modbus_rtu.c
*- Description  :
***************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "lightmodbus/lightmodbus.h"
#include "lightmodbus/master.h"
#include "modbus_rtu.h"


ModbusMaster mstatus;
DATA_REQUEST _data_request;
DATA_RESPONSE _data_response;

DATA_REQUEST * init_modbus(int slave_addr, int index, int count)
{
    modbusMasterInit( &mstatus );
    modbusBuildRequest0304( &mstatus, 3, slave_addr, index, count );
    _data_request.data = mstatus.request.frame;
    _data_request.length = mstatus.request.length;
    _data_request.predictedResponseLength = mstatus.predictedResponseLength;
    return &_data_request;
}

DATA_RESPONSE * parser_modbus(uint8_t * data, uint8_t length)
{
    mstatus.response.frame = data;
    mstatus.response.length = length;
    int mec = modbusParseResponse( &mstatus );
    if (mec != 0) {
        printf("parse failed !\n");
    }

    switch ( mstatus.data.type )
	{
		case MODBUS_HOLDING_REGISTER:
		case MODBUS_INPUT_REGISTER:
            _data_response.data = (uint8_t *)mstatus.data.regs;
            break;

		case MODBUS_COIL:
		case MODBUS_DISCRETE_INPUT:
            _data_response.data = mstatus.data.coils;
			break;
	}
    _data_response.length = mstatus.data.count;
    return &_data_response;
}

void free_modbus(void){
    return;
}

struct PROTO_MGR_TAG modbus_rtu_proto_manager = {
    .proto_type = MODBUS_RTU,
    .init = init_modbus,
    .parser = parser_modbus,
    .destroy = free_modbus,
};
