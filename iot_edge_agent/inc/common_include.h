
#ifndef COMMON_INCLUDE_H
#define COMMON_INCLUDE_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define TOPIC_SIZE          3
#define DATA_SUB_TOPIC      "modbus/notify/spont/TEMP_HUMI/temp_humi_01"
#define DATA_PUB_TOPIC      "modbus/notify/spont/TEMP_HUMI/temp_humi_01"
#define MODEL_SUB_TOPIC     "/ModelManager/get/reponse/MODBUS/Southmodel"
#define MODEL_PUB_TOPIC     "/MODBUS/get/request/ModelManager/Southmodel"
#define SERIAL_SUB_TOPIC    "/V1/MODBUS_RTU"
#define SERIAL_PUB_TOPIC    "/V1/COMM_INTF"


#ifdef __cplusplus
}
#endif

#endif
