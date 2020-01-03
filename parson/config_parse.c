#include <stdio.h>
#include <stdlib.h>
#include "parson.h"
#include "config_parse.h"

SERIAL_OPTION serial_option;
MQTT_OPTION mqtt_option;
JSON_Value *root_value;

int init_app_config(const char * file_path)
{
    int result = 0;

    JSON_Object *app_config_object;

    JSON_Object *mqtt;
    JSON_Object *serial;

    root_value = json_parse_file(file_path);

    if (json_value_get_type(root_value) != JSONObject) {
        return -1;
    }

    app_config_object = json_value_get_object(root_value);

    mqtt = json_object_dotget_object(app_config_object, "platformConf");
    mqtt_option.mqttHost  = (char*)json_object_dotget_string(mqtt, "mqtt_host");
    mqtt_option.mqttPort  = json_object_dotget_number(mqtt, "mqtt_port");
    mqtt_option.clientId  = (char*)json_object_dotget_string(mqtt, "client_id");
    mqtt_option.username  = (char*)json_object_dotget_string(mqtt, "username");
    mqtt_option.password  = (char*)json_object_dotget_string(mqtt, "password");
    mqtt_option.keepAlive = json_object_dotget_number(mqtt, "keep_alive");

    serial = json_object_dotget_object(app_config_object, "channels");
    serial_option.device = (char*)json_object_dotget_string(serial, "device");
    serial_option.baudrate = json_object_dotget_number(serial, "baudRate");
    serial_option.databits = json_object_dotget_number(serial, "dataBits");
    serial_option.parity = (char*)json_object_dotget_string(serial, "parity");
    serial_option.stopbits = json_object_dotget_number(serial, "stopBits");

    printf("%s\n", mqtt_option.mqttHost);
    printf("%d\n", mqtt_option.mqttPort);
    printf("%s\n", mqtt_option.clientId);
    printf("%s\n", mqtt_option.username);
    printf("%s\n", mqtt_option.password);
    printf("%d\n", mqtt_option.keepAlive);


    printf("%s\n", serial_option.device);
    printf("%d\n", serial_option.baudrate);
    printf("%d\n", serial_option.databits);
    printf("%s\n", serial_option.parity);
    printf("%d\n", serial_option.stopbits);
    return result;
}

void free_app_config()
{
	json_value_free(root_value);
}

int main(int argc, char ** argv){

    const char * file_path = "./app-config.json";

    init_app_config(file_path);

	exit(0);
}



