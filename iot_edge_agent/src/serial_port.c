#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <azure_c_shared_utility/xlogging.h>

#include "serial_port.h"

int gpio = (UART_GPIO2_VAL | UART_GPIO2_MASK);

#define BUFF_SIZE 256
uint8_t buffer[BUFF_SIZE];

struct SERIAL_PORT_TAG {
    char interfaceName[100];
    int fd;
    int baudRate;
    uint8_t dataBits;
    char parity;
    uint8_t stopBits;
    uint64_t lastSentTime;
    struct timeval timeout;
    SerialPortError lastError;
} SERIAL_PORT_TAG;


SERIAL_PORT SerialPort_create(const char* interfaceName, int baudRate, uint8_t dataBits, char parity, uint8_t stopBits)
{
    SERIAL_PORT serialPort = (SERIAL_PORT) malloc(sizeof(SERIAL_PORT_TAG));

    if (serialPort != NULL) {
        serialPort->fd = -1;
        serialPort->baudRate = baudRate;
        serialPort->dataBits = dataBits;
        serialPort->stopBits = stopBits;
        serialPort->parity = parity;
        serialPort->lastSentTime = 0;
        serialPort->timeout.tv_sec = 0;
        serialPort->timeout.tv_usec = 5000; /* 5 ms */
        strncpy(serialPort->interfaceName, interfaceName, 100);
        serialPort->lastError = SERIAL_PORT_ERROR_NONE;
    }

    return serialPort;
}

void SerialPort_destroy(SERIAL_PORT serialPort)
{
    if (serialPort != NULL) {
        free(serialPort);
    }
}

bool SerialPort_open(SERIAL_PORT serialPort)
{
    serialPort->fd = open(serialPort->interfaceName, O_RDWR | O_NOCTTY | O_NDELAY);

    if (serialPort->fd == -1) {
        serialPort->lastError = SERIAL_PORT_ERROR_OPEN_FAILED;
        return false;
    }

    if( ! isatty(serialPort->fd) ){
        return false;
    }

    if( fcntl(serialPort->fd, F_SETFL, 0) == -1 ){
        return false;
    }

    struct termios tios;
    speed_t baudrate;

    tcgetattr(serialPort->fd, &tios);

    switch (serialPort->baudRate) {
    case 110:
        baudrate = B110;
        break;
    case 300:
        baudrate = B300;
        break;
    case 600:
        baudrate = B600;
        break;
    case 1200:
        baudrate = B1200;
        break;
    case 2400:
        baudrate = B2400;
        break;
    case 4800:
        baudrate = B4800;
        break;
    case 9600:
        baudrate = B9600;
        break;
    case 19200:
        baudrate = B19200;
        break;
    case 38400:
        baudrate = B38400;
        break;
    case 57600:
        baudrate = B57600;
        break;
    case 115200:
        baudrate = B115200;
        break;
    default:
        baudrate = B9600;
        serialPort->lastError = SERIAL_PORT_ERROR_INVALID_BAUDRATE;
    }

    /* Set baud rate */
    if ((cfsetispeed(&tios, baudrate) < 0) || (cfsetospeed(&tios, baudrate) < 0)) {
        close(serialPort->fd);
        serialPort->fd = -1;
        serialPort->lastError = SERIAL_PORT_ERROR_INVALID_BAUDRATE;
        return false;
    }

    tios.c_cflag |= (CREAD | CLOCAL);
    tios.c_cflag &= ~CSIZE;

    switch (serialPort->dataBits) {
    case 5:
        tios.c_cflag |= CS5;
        break;
    case 6:
        tios.c_cflag |= CS6;
        break;
    case 7:
        tios.c_cflag |= CS7;
        break;
    case 8:
    default:
        tios.c_cflag |= CS8;
        break;
    }

    /* Set stop bits (1/2) */
    if (serialPort->stopBits == 1)
        tios.c_cflag &=~ CSTOPB;
    else /* 2 */
        tios.c_cflag |= CSTOPB;

    if (serialPort->parity == 'N') {
        tios.c_cflag &=~ PARENB;
    } else if (serialPort->parity == 'E') {
        tios.c_cflag |= PARENB;
        tios.c_cflag &=~ PARODD;
    } else { /* 'O' */
        tios.c_cflag |= PARENB;
        tios.c_cflag |= PARODD;
    }

    tios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    if (serialPort->parity == 'N') {
        tios.c_iflag &= ~INPCK;
    } else {
        tios.c_iflag |= INPCK;
    }

    tios.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL);
    tios.c_iflag |= IGNBRK; /* Set ignore break to allow 0xff characters */
    tios.c_iflag |= IGNPAR;
    tios.c_oflag &=~ OPOST;

    tios.c_cc[VMIN] = 0;
    tios.c_cc[VTIME] = 1;

    tcflush(serialPort->fd, TCIFLUSH);

    if (tcsetattr(serialPort->fd, TCSANOW, &tios) < 0) {
        close(serialPort->fd);
        serialPort->fd = -1;
        serialPort->lastError = SERIAL_PORT_ERROR_INVALID_ARGUMENT;
        
        return false;
    }

    gpio &= 0xffff;
    ioctl(serialPort->fd, IOCTL_GPIOSET, &gpio);
    
    return true;
}

void SerialPort_close(SERIAL_PORT serialPort)
{
    if (serialPort->fd != -1) {
        close(serialPort->fd);
        serialPort->fd = 0;
    }
}

int SerialPort_getBaudRate(SERIAL_PORT serialPort)
{
    return serialPort->baudRate;
}

void SerialPort_discardInBuffer(SERIAL_PORT serialPort)
{
    tcflush(serialPort->fd, TCIOFLUSH);
}

void SerialPort_setTimeout(SERIAL_PORT serialPort, int timeout)
{
    serialPort->timeout.tv_sec = timeout / 1000;
    serialPort->timeout.tv_usec = (timeout % 1000) * 1000;
}

SerialPortError SerialPort_getLastError(SERIAL_PORT serialPort)
{
    return serialPort->lastError;
}

SERIAL_MESSAGE_HANDLE SerialPort_message_create(const uint8_t * message, size_t length){

    SERIAL_MESSAGE_HANDLE result;

    if (message == NULL) {
        LogError("args error\n");
        result = NULL;
    } else {
        result = malloc(sizeof(SERIAL_MESSAGE));
        if (result != NULL) {
            memset(result, 0, sizeof(SERIAL_MESSAGE));
            result->length = length;
            if(result->length > 0){
                result->message = malloc(length);
                if(result->message == NULL){
                    LogError("Failure allocating message value of %uz", length);
                    free(result);
                    result = NULL;
                } else {
                    memcpy(result->message, message, length);
                }
            } else {
                result->message = NULL;
            }
        }
    }
    return result;
}

void SerialPort_message_destroy(SERIAL_MESSAGE_HANDLE handle)
{
    SERIAL_MESSAGE * msgInfo = (SERIAL_MESSAGE *)handle;

    if(msgInfo != NULL){
        free(msgInfo->message);
        free(msgInfo);
    }
}

int SerialPort_recv(SERIAL_PORT serialPort, ON_RECV_CALLBACK on_recv_callback, void * context)
{
    fd_set set;
    uint8_t * temp_buf = buffer;
    size_t left = BUFF_SIZE;
    size_t recv_len = 0;
    int nbyte = -1;

    serialPort->lastError = SERIAL_PORT_ERROR_NONE;

    FD_ZERO(&set);
    FD_SET(serialPort->fd, &set);

    int ret = select(serialPort->fd + 1, &set, NULL, NULL, &(serialPort->timeout));

    if (ret == -1) {
        serialPort->lastError = SERIAL_PORT_ERROR_UNKNOWN;
        return -1;
    } else if (ret == 0)
        return -1;
    else {

        while(nbyte && left) {
            nbyte = read(serialPort->fd, temp_buf, left);
            
            if(nbyte == -1) {
                return -1;
            }
            if(nbyte > 0){
                temp_buf += nbyte;
                recv_len += nbyte;
                left -= nbyte;
                continue;
            } else {
                break;
            }
        }

        if(recv_len){
            SERIAL_MESSAGE * message_handle = SerialPort_message_create((const uint8_t *)buffer, (size_t)recv_len);
            if(message_handle){
                on_recv_callback(message_handle, context);
            }

            printf("RX: ");
            for(int i = 0; i < recv_len; i++){
                printf("%02hhx ", buffer[i]);
            }
            printf("\n");

        }
    }
    
    return 1;
}

int SerialPort_send(SERIAL_PORT serialPort, const uint8_t * send_data, size_t data_len){

    int result = 0;
    if (serialPort->fd != -1) {

        printf("TX: ", (int)data_len);
        for (int i = 0; i < data_len; i++) {
            printf("%02hhx ", send_data[i]);
        }
        printf("\n");

        int nbyte = write(serialPort->fd, send_data, data_len);

        // tcdrain(serialPort->fd);

        if (nbyte == -1) {
            return -1;
        }
    }
    
    return result;
}
