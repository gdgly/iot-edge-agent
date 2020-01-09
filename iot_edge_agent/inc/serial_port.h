
#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SERIAL_PORT_TAG* SERIAL_PORT_HANDLE;

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

typedef void(*ON_RECV_CALLBACK)(SERIAL_MESSAGE_HANDLE handle, void * context);

SERIAL_MESSAGE_HANDLE SerialPort_message_create(const uint8_t * message, size_t length);
void SerialPort_message_destroy(SERIAL_MESSAGE_HANDLE handle);
SERIAL_PORT_HANDLE SerialPort_create(const char* interfaceName, int baudRate, uint8_t dataBits, char parity, uint8_t stopBits);
void SerialPort_destroy(SERIAL_PORT_HANDLE handle);
bool SerialPort_open(SERIAL_PORT_HANDLE handle);
void SerialPort_close(SERIAL_PORT_HANDLE handle);
int SerialPort_getBaudRate(SERIAL_PORT_HANDLE handle);
void SerialPort_setTimeout(SERIAL_PORT_HANDLE handle, int timeout);
void SerialPort_discardInBuffer(SERIAL_PORT_HANDLE handle);
int SerialPort_recv(SERIAL_PORT_HANDLE handle, ON_RECV_CALLBACK on_recv_callback, void * context);
int SerialPort_send(SERIAL_PORT_HANDLE handle, const uint8_t * buffer, size_t length);
SerialPortError SerialPort_getLastError(SERIAL_PORT_HANDLE handle);

#ifdef __cplusplus
}
#endif


#endif
