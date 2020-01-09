#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "protocol_manager.h"

extern struct PROTO_MGR_TAG modbus_rtu_proto_manager;

typedef struct DATA_REQUEST_TAG {
    uint8_t * data;
    uint8_t length;
    uint8_t predictedResponseLength;
} DATA_REQUEST;

typedef struct DATA_RESPONSE_TAG {
    uint8_t * data;
    uint8_t length;
} DATA_RESPONSE;


DATA_REQUEST * init_modbus(int slave_addr, int index, int count);
DATA_RESPONSE * parser_modbus(uint8_t * data, uint8_t length);
void free_modbus(void);

#ifdef __cplusplus
}
#endif

#endif
