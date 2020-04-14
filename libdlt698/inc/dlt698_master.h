#ifndef DLT698_PACKET_H
#define DLT698_PACKET_H
/******************************************************************************/

#include "dlt698_common.h"

/******************************************************************************/
#define DLT698_LEADER_CHAR ((uint8_t)0xFE)//前导符
#define DLT698_HEADER      ((uint8_t)0x68)//帧起始符
#define DLT698_END         ((uint8_t)0x16)

//控制符
#define DLT698_C_2_S_DATA  ((uint8_t)0x43)//表示从客户机到服务器

//客户机地址
#define DLT698_CLIENT_ADDR ((uint8_t)0x00)//00表示不关心客户机地址

//PIID
#define DLT698_PIID        ((uint8_t)0x01)

//时间标签
#define DLT698_TIME_LABER  ((uint8_t)0x00)

/******************************************************************************/
#define DLT698_HEADER_LOC   4
#define DLT698_LENGTH_LOC   5
#define DLT698_CON_LOC      7
#define DLT698_SA_LOC       8
#define DLT698_CA_LOC       15
#define DLT698_HEAD_CRC_LOC 16
#define DLT698_APDU_LOC     18
#define DLT698_PIID_LOC     20
#define DLT698_OAD_LOC      21
#define DLT698_TIME_LOC     25
#define DLT698_ALL_CRC_LOC  26
#define DLT698_END_LOC      28
#define PPPINITFCS16 		0xFFFF

#define MAX_SIZE_OF_PACKET_DATA     (512)
#define MAX_SIZE_OF_BUFFER          (1024)
#define MAX_COLL_OAD_SIZE 	        (127)

typedef struct DLT698_PACKET_TAG {
    uint8_t data[MAX_SIZE_OF_PACKET_DATA];
    uint8_t length;
} DLT698_PACKET;

typedef struct
{
    uint8_t *pStart;        //指向帧头
    uint8_t *pSecurityData; //指向安全数据
    uint16_t uwSA_len;      //SA长度
    uint16_t nHCSPos;
    uint16_t uwFramelen;    //帧长度，不含头尾
} DLT698_FRAME;

typedef struct
{
    OAD stOAD;    // OAD
    uint8_t ucVal;  // store valid mark
    uint16_t uwLen; // Data length
    uint8_t *ucPtr; // Data buffer pointer
} DATA_UNIT;

typedef enum
{
    D_CLASS_NULL = 0,
    D_CLASS_REAL = 1,   // 实时数据
    D_CLASS_CUR_FREEZE, // 秒、分、小时冻结
    D_CLASS_DAY_FREEZE, // 日冻结数据
    D_CLASS_MON_FREEZE, // 月冻结数据
    D_CLASS_EVENT,      // 事件类数据
    D_CLASS_DATA,       // real + cur + day + mon
} DATA_CLASS;

typedef struct
{
    DATA_CLASS eDataCls;                     // 存储的数据类型
    OAD stMainOAD;                           // main OAD of ROAD
    uint8_t ucDataNum;                       // 已存储的数据个数
    DATA_UNIT stDataUnit[MAX_COLL_OAD_SIZE]; // OAD + Len + Data pointer to store
} COLL_STORE_DATA;

typedef COLL_STORE_DATA DLT698_ANALY_RES;

int convert_to_string_float(uint8_t * ptr, char * res, int math);
DLT698_PACKET * dlt698_packet_build(uint8_t *oad, uint8_t *sa);
DLT698_ANALY_RES * dlt698_response_analy(uint8_t * frame, int length);

#endif
