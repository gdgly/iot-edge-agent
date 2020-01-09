#ifndef _IOT_MODEL_H
#define _IOT_MODEL_H


#ifdef __cplusplus
extern "C" {
#endif

#include <azure_c_shared_utility/singlylinkedlist.h>
#include "protocol_manager.h"
#include "serial_port.h"

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

typedef struct DEVICE_PORT_TAG {
    char * portType;
    char * serialID;
    int baudRate;
    int dataBit;
    int stopBit;
    char parity;
    char * ip;
    int port;
} DEVICE_PORT_OPTION;

typedef struct DEVICE_CHANNEL_TAG {
    char * dev;
    int acqInt;
    int upm;
    char * protoType;
    char * modelType;
    int timeOut;
    int devAddr;
    DEVICE_PORT_OPTION * device_port;
    SERIAL_PORT_HANDLE serial_handle;
} DEVICE_CHANNEL_INFO;

int parse_device_info_to_list(JSON_Value * json_value, SINGLYLINKEDLIST_HANDLE device_list, int * device_count);
MODBUS_RTU_MODEL * modbus_model_parse(JSON_Value * json_value);
PROTO_TYPE parse_proto_header(JSON_Value * json_value);
void iot_model_free(void);

#ifdef __cplusplus
}
#endif

#endif
