
#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IOCTL_GPIOGET 0x8000
#define IOCTL_GPIOSET 0x8001

#define UART_GPIO2_MASK          (1 << 2)
#define UART_GPIO6_MASK          (1 << 6)
#define UART_GPIO10_MASK         (1 << 10)
#define UART_GPIO14_MASK         (1 << 14)

#define UART_GPIO2_VAL           (1 << 18)
#define UART_GPIO6_VAL           (1 << 22)
#define UART_GPIO10_VAL          (1 << 26)
#define UART_GPIO14_VAL          (1 << 30)


typedef struct SERIAL_PORT_TAG* SERIAL_PORT;

typedef enum {
    SERIAL_PORT_ERROR_NONE = 0,
    SERIAL_PORT_ERROR_INVALID_ARGUMENT = 1,
    SERIAL_PORT_ERROR_INVALID_BAUDRATE = 2,
    SERIAL_PORT_ERROR_OPEN_FAILED = 3,
    SERIAL_PORT_ERROR_UNKNOWN = 99
} SerialPortError;

typedef struct SERIAL_MESSAGE_TAG{
    uint8_t * message;
    size_t length;
}SERIAL_MESSAGE, * SERIAL_MESSAGE_HANDLE;

typedef void(*ON_RECV_CALLBACK)(SERIAL_MESSAGE_HANDLE massage_handle, void * context);

SERIAL_MESSAGE_HANDLE SerialPort_message_create(const uint8_t * message, size_t length);

void SerialPort_message_destroy(SERIAL_MESSAGE_HANDLE handle);

SERIAL_PORT SerialPort_create(const char* interfaceName, int baudRate, uint8_t dataBits, char parity, uint8_t stopBits);

void SerialPort_destroy(SERIAL_PORT serialPort);

bool SerialPort_open(SERIAL_PORT serialPort);

void SerialPort_close(SERIAL_PORT serialPort);

int SerialPort_getBaudRate(SERIAL_PORT serialPort);

void SerialPort_setTimeout(SERIAL_PORT serialPort, int timeout);

void SerialPort_discardInBuffer(SERIAL_PORT serialPort);

int SerialPort_recv(SERIAL_PORT serialPort, ON_RECV_CALLBACK on_recv_callback, void * context);

int SerialPort_send(SERIAL_PORT serialPort, const uint8_t * buffer, size_t length);

SerialPortError SerialPort_getLastError(SERIAL_PORT serialPort);

#ifdef __cplusplus
}
#endif


#endif
