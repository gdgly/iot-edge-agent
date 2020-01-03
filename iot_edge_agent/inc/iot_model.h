#ifndef _IOT_MODEL_H
#define _IOT_MODEL_H

#include "protocol_manager.h"

typedef struct REGISTER_GROUP_TAG {
    char * m;
    uint8_t n;
    uint8_t fn;
    uint8_t big;

    char * df;
    char * math;
    char * address;
    char * relmodel;
} REGISTER_GROUP;

typedef struct MODBUS_RTU_MODEL_TAG {
    uint8_t salveAddr;
    uint8_t acq_int;
    uint8_t register_group_len;
    REGISTER_GROUP * register_group;
} MODBUS_RTU_MODEL;

MODBUS_RTU_MODEL * modbus_model_parse(const char  * json_string);
PROTO_TYPE parse_proto_header(const char * josn_string);
void iot_model_free(void);


#endif
