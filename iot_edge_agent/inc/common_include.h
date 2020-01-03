
#ifndef COMMON_INCLUDE_H
#define COMMON_INCLUDE_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#define MAX_PROT_DATA_LEN       1024

#define JSON_STR_MAX_LEN        32
#define TOPIC_STR_MAX_LEN       32

#define TOPIC_SIZE          3
#define DATA_SUB_TOPIC      "modbus/notify/spont/TEMP_HUMI/temp_humi_01"
#define DATA_PUB_TOPIC      "modbus/notify/spont/TEMP_HUMI/temp_humi_01"
#define MODEL_SUB_TOPIC     "/ModelManager/get/reponse/MODBUS/Southmodel"
#define MODEL_PUB_TOPIC     "/MODBUS/get/request/ModelManager/Southmodel"
#define SERIAL_SUB_TOPIC    "/V1/MODBUS_RTU"
#define SERIAL_PUB_TOPIC    "/V1/COMM_INTF"

typedef enum
{
    TYPE_DLT20 = 1,
    TYPE_DLT645,
    TYPE_DLT698
}PROT_TYPE;

typedef struct
{
    int port;
    uint32_t speed;
    char parity;
    uint8_t nbits;
    uint8_t nstop;
}UART_CFG;

typedef struct
{
    uint32_t ipv4;
    uint16_t port;
}NET_CFG;

typedef struct
{
    char port_type[JSON_STR_MAX_LEN];
    UART_CFG uart;
    NET_CFG net;
}PORT_CFG;

typedef struct
{
    int uart_port;
    uint32_t ipv4;
}PORT_RESP;

/*协议模块向通信接口模块发送MQTT消息的结构体*/
typedef struct
{
    uint32_t seq;           //数据帧序列号
    //int port;             //端口号
    PORT_CFG port_info;     //端口
    PROT_TYPE prot_type;    //协议类型
    int recv_len;           //串口需要返回数据的长度
    char resp_topic[TOPIC_STR_MAX_LEN];   //响应消息的TOPIC
    uint32_t send_len;      //发送数据(send_data)的长度
    uint8_t send_data[MAX_PROT_DATA_LEN]; //己封装的协议数据
}PORT_REQUEST;

/*
通信接口模块返回给协议模块MQTT消息的结构体
注意:使用了offsetof(PORT_RESPONSE, recv_data))
recv_data需要放在结构体最后面
*/
typedef struct
{
    uint32_t seq;        //数据帧序列号
    int resp_stat;       //串口返回数据状态:1-成功,0-失败
    PORT_RESP port_info; //端口
    int recv_len;        //接收数据(recv_data)的长度
    uint8_t recv_data[MAX_PROT_DATA_LEN]; //己封装的协议原始数据
}PORT_RESPONSE;


#ifdef __cplusplus
}
#endif

#endif
