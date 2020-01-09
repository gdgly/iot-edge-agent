#ifndef PROTO_MANAGER_H
#define PROTO_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "modbus_rtu.h"

typedef enum {
    MODBUS_RTU,
    DLT645,
    DLT698
} PROTO_TYPE;

typedef struct PROTO_MGR_TAG {
    // DATA_REQUEST * request;
    // DATA_RESPONSE * response;
    PROTO_TYPE proto_type;
    struct DATA_REQUEST_TAG * (*init) (int slave_addr, int index, int count);
    struct DATA_RESPONSE_TAG * (*parser) (uint8_t * data, uint8_t length);
    void (*destroy) (void);
} PROTO_MGR, * PROTO_MGR_HANDLE;

PROTO_MGR_HANDLE protocol_init(PROTO_TYPE proto_type);

#ifdef __cplusplus
}
#endif

#endif
