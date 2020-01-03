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
DATA_REQUEST * data_request;
DATA_RESPONSE * data_response;

DATA_REQUEST * init_modbus(int slave_addr, int index, int count)
{
    modbusMasterInit( &mstatus );
    modbusBuildRequest0304( &mstatus, 3, slave_addr, index, count );
    data_request = malloc(sizeof(DATA_REQUEST));
    data_request->data = mstatus.request.frame;
    data_request->length = mstatus.request.length;
    data_request->predictedResponseLength = mstatus.predictedResponseLength;
    return data_request;
}

DATA_RESPONSE * parser_modbus(uint8_t * data, uint8_t length)
{
    mstatus.response.frame = data;
    mstatus.response.length = length;
    int mec = modbusParseResponse( &mstatus );
    if (mec != 0) {
        printf("parse failed !\n");
    }
    data_response = malloc(sizeof(DATA_RESPONSE));
    switch ( mstatus.data.type )
	{
		case MODBUS_HOLDING_REGISTER:
		case MODBUS_INPUT_REGISTER:
            data_response->data = (uint8_t *)mstatus.data.regs;
            break;

		case MODBUS_COIL:
		case MODBUS_DISCRETE_INPUT:
            data_response->data = mstatus.data.coils;
			break;
	}
    data_response->length = mstatus.data.count;
    return data_response;
}

void free_modbus(void)
{
    free(data_request);
    free(data_response);
}

struct PROTO_MGR_TAG modbus_rtu_proto_manager = {
    .proto_type = MODBUS_RTU,
    .init = init_modbus,
    .parser = parser_modbus,
    .destroy = free_modbus,
};
