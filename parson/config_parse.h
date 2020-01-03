#ifndef _CONFIG_PARSE_
#define _CONFIG_PARSE_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct MQTT_OPTION_TAG{
    char* mqttHost;
    char* username;
    char* password;
    char* clientId;
    int mqttPort;
    int keepAlive;
}MQTT_OPTION;

typedef struct SERIAL_OPTION_TAG{
    char* device;
    int baudrate;
    int databits;
    int stopbits;
    char* parity;
}SERIAL_OPTION;

#ifdef __cplusplus
}
#endif

#endif
