
#ifndef EDGE_AGENT_H
#define EDGE_AGENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "iothub_mqtt_client.h"
#include "common_include.h"
#include "protocol_manager.h"

typedef void (*PROTOCOL_MODBUS_CALLBACK) (IOTHUB_MQTT_CLIENT * mqttClient, PORT_RESPONSE * port_response);
typedef void (*MODEL_PARSE_CALLBACK) (const char * json_string);

typedef struct PROTOCOL_CALLBACK_TAG
{
    PROTOCOL_MODBUS_CALLBACK modbus;
    MODEL_PARSE_CALLBACK modelParse;
} PROTOCOL_CALLBACK;

typedef struct IOTEA_CLIENT_TAG
{
    bool subscribed;
    time_t subscribeSentTimestamp;
    char* endpoint;
    char* name;
    IOTHUB_MQTT_CLIENT_HANDLE mqttClient;
    MQTT_CONNECTION_TYPE mqttConnType;
    PROTOCOL_CALLBACK callback;
    PROTO_TYPE protoType;
    PROTO_MGR_HANDLE protoManager;
} IOTEA_CLIENT;

typedef struct IOTEA_CLIENT_TAG* IOTEA_CLIENT_HANDLE;


typedef struct IOTEA_CLIENT_OPTIONS_TAG
{
    // Indicate whether receiving deliveried message when reconnection.
    bool cleanSession;

    char* clientId;
    char* username;
    char* password;

    // Device keep alive interval with seconds.
    uint16_t keepAliveInterval;

    // Timeout for publishing device request in seconds.
    size_t retryTimeoutInSeconds;

} IOTEA_CLIENT_OPTIONS;

MOCKABLE_FUNCTION(, void, iotea_client_register_model_parse, IOTEA_CLIENT_HANDLE, handle, MODEL_PARSE_CALLBACK, callback);
MOCKABLE_FUNCTION(, void, iotea_client_register_modbus, IOTEA_CLIENT_HANDLE, handle, PROTOCOL_MODBUS_CALLBACK, callback);
MOCKABLE_FUNCTION(, IOTEA_CLIENT_HANDLE, iotea_client_init, char*, broker, char*, name);
MOCKABLE_FUNCTION(, void, iotea_client_deinit, IOTEA_CLIENT_HANDLE, handle);
MOCKABLE_FUNCTION(, int, iotea_client_connect, IOTEA_CLIENT_HANDLE, handle, const IOTEA_CLIENT_OPTIONS*, options);
MOCKABLE_FUNCTION(, int, iotea_client_dowork, const IOTEA_CLIENT_HANDLE, handle);

#ifdef __cplusplus
}
#endif

#endif
