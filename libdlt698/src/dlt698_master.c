#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#include <azure_c_shared_utility/xlogging.h>

#include "dlt698_master.h"
#include "dlt698_common.h"

const uint16_t fcstab[256] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78};


static int dwDlt698DataLen(uint8_t *ucRcvBuf, int dwRcvLen);
static int dwDlt698Len(uint8_t *ucRcvBuf, int *LenBytes);
static int dwGetResponseNormal(uint8_t *pucBuf, COLL_STORE_DATA *pstCollStoreData);
static int dwDlt698RSDLen(uint8_t *ucRcvBuf, int dwRcvLen);
static int dwDlt698DataLenByOAD(OAD stOAD, uint8_t *pucBuf);
static int dwGetOad(uint8_t *pucStr, OAD *pstOad);
static int dwDlt698MsLen(uint8_t *ucRcvBuf, int dwRcvLen);


uint8_t Buffer[MAX_SIZE_OF_BUFFER];

uint16_t pppfcs16(register uint16_t fcs, register unsigned char *cp, register int len)
{
    assert(sizeof(uint16_t) == 2);
    assert(((uint16_t)-1) > 0);
    while (len--)
        fcs = (fcs >> 8) ^ fcstab[(fcs ^ *cp++) & 0xff];
    return (~fcs);
}

DLT698_PACKET * dlt698_packet_build(uint8_t *oad, uint8_t *sa)
{
	DLT698_PACKET * result;

	if (NULL == oad || NULL == sa)
	{
		result = NULL;
	}
	else
	{
		result = malloc(sizeof(DLT698_PACKET));

		if (NULL == result)
		{
			LogError("Error: Malloc DLT698_PACKET Filaure");
		}
		else
		{
			int length = 0;
			for (int i = 0; i < 4; ++i)
				Buffer[i] = DLT698_LEADER_CHAR;

			Buffer[DLT698_HEADER_LOC] = DLT698_HEADER;
			Buffer[DLT698_CON_LOC] = DLT698_C_2_S_DATA;		//控制域C

			length += 3;

			Buffer[DLT698_SA_LOC] = 05;//地址域中服务器地址SA的第一个字节,表示服务器地址类型、逻辑地址与地址长度
			++length;
			
			for(int i = 0; i < 6; ++i)
			{
				Buffer[DLT698_SA_LOC + 1 + i] = *(sa + 5 - i);
			}	//服务器地址
			length += 6;
			
			Buffer[DLT698_CA_LOC] = DLT698_CLIENT_ADDR;
			++length;
			length += 2;
			
			Buffer[DLT698_APDU_LOC] = GET_Request;
			Buffer[DLT698_APDU_LOC+1] = GetRequestNormal;
			Buffer[DLT698_PIID_LOC] = DLT698_PIID;

			for(int i = 0; i < 4; ++i)
				Buffer[DLT698_OAD_LOC + i] = *(oad + i);
			
			Buffer[DLT698_TIME_LOC] = DLT698_TIME_LABER;
			length += 8;
			length += 2;
			//长度域
			Buffer[DLT698_LENGTH_LOC] = length;
			Buffer[DLT698_LENGTH_LOC+1] = 0x00;

			uint16_t uwhcs = pppfcs16(PPPINITFCS16, Buffer + 5, 11);
			Buffer[DLT698_HEAD_CRC_LOC] = (uwhcs & 0x00FF);
			Buffer[DLT698_HEAD_CRC_LOC+1] = (uwhcs >> 8);
			//整帧校验

			uint16_t uwfcs = pppfcs16(PPPINITFCS16, Buffer + 5, length - 2);
			Buffer[DLT698_ALL_CRC_LOC] = (uwfcs & 0x00FF);
			Buffer[DLT698_ALL_CRC_LOC+1] = (uwfcs >> 8);

			Buffer[DLT698_END_LOC] = DLT698_END;

			length += 6;
			memcpy(result->data, Buffer, length);
			result->length = length;
		}
	}
	return result;
}

int dwDlt698DataLenByOAD(OAD stOAD, uint8_t *pucBuf)
{
    uint8_t byOI1 = stOAD.OI_date >> 8;
    if ((byOI1 >> 4) == 0)
        byOI1 = byOI1 & 0xF0;
    return dwDlt698DataLen(pucBuf, 256);
}

int dwGetOad(uint8_t *pucStr, OAD *pstOad)
{
    pstOad->OI_date = MAKE_WORD(*pucStr, *(pucStr + 1));
    pucStr += 2;

    pstOad->attr_ID = *pucStr;
    pucStr++;
    pstOad->attr_index = *pucStr;
    pucStr++;

    return 4;
}

int dwGetResponseNormal(uint8_t *pucBuf, COLL_STORE_DATA *pstCollStoreData)
{

	uint8_t i = 0;
    uint8_t ucResultNum = 0;
    int dwLen = 0;
    COLL_STORE_DATA *pstCollData = pstCollStoreData;
    uint8_t *Ptr = pucBuf;
    OAD stOADtmp;

	memset( &stOADtmp, 0 , sizeof(stOADtmp));

    Ptr++; //APDU_Tag

    switch (*Ptr++) //GetResponseNormal
    {
        case GetResponseNormal:
        {
            Ptr++;
            ucResultNum = 1;
        } break;
        case GetResponseNormalList:
        {
            Ptr++;
            if ((ucResultNum = *Ptr++) > 127) //SEQUENCE OF OAD
            {
                return -1;
            }
        }
        break;
        default:
            return -1;
    }

    pstCollData->eDataCls = D_CLASS_REAL;

    for (i = 0; i < ucResultNum; i++)
    {
        Ptr += dwGetOad(Ptr, &stOADtmp);

        if (ERROR_INFO == *Ptr)
        {
            Ptr++; // Choice
            Ptr++; // DAR
        }
        else if (RES_DATA == *Ptr)
        {
            Ptr++; // Choice
            if ((dwLen = dwDlt698DataLenByOAD(stOADtmp, Ptr)) < 0)
            {
                return -1;
            }
#if 1
            //if (stOADtmp.OI_date != 0) //组合有功电能=0
            {
                pstCollData->stDataUnit[pstCollData->ucDataNum].stOAD.OI_date = stOADtmp.OI_date;
                pstCollData->stDataUnit[pstCollData->ucDataNum].stOAD.attr_ID = stOADtmp.attr_ID;
                pstCollData->stDataUnit[pstCollData->ucDataNum].stOAD.attr_index = stOADtmp.attr_index;

                pstCollData->stDataUnit[pstCollData->ucDataNum].uwLen = dwLen;
                pstCollData->stDataUnit[pstCollData->ucDataNum].ucPtr = Ptr;
                pstCollData->stDataUnit[pstCollData->ucDataNum].ucVal = 1;
                pstCollData->ucDataNum++;
            }
#endif
            Ptr += dwLen;
        }
        else
        {
            return -1;
        }
    }

return 1;
}

int dwGet698Apdu(uint8_t *pframe, uint8_t **pAPDU, uint8_t **sa_addr, uint8_t *sa_addr_len)
{
	uint16_t uwframeLen = 0;
    uint32_t nHCSPos = 0;
    uint32_t nFCSPos = 0;
    uint16_t uwhcs = 0;
    uint16_t uwfcs = 0;
    uint16_t SA_Len = 0;
    int dwAPDULen = 0;
	int prem_num = 0;

    //判断接收数据前导码长度
    for(SA_Len = 0; SA_Len < 4; SA_Len++)
    {
        if(0xFE == pframe[SA_Len])
        {
            prem_num++;
        }
    }

    if (pframe[prem_num] != 0x68)
        return -1;

    //帧长占2byte，不含头尾，先传低位再传高位
    uwframeLen = MAKE_WORD(pframe[prem_num+2], pframe[prem_num+1]);

    /*SA地址字节数，0代表1个字节*/
    SA_Len = (pframe[prem_num+4] & 0x0F) + 2;

    *sa_addr = pframe + prem_num + 5; //地址SA
    *sa_addr_len = SA_Len - 1;  //地址SA长度,不包含客户机地址CA

	//帧头除去帧头校验HFS的长度
    nHCSPos += SA_Len + 5;

    //帧头校验hcs,除去起始字符0x68
    uwhcs = PPPINITFCS16;

    uwhcs = pppfcs16(uwhcs, pframe +prem_num + 1, nHCSPos - 1);

    if (uwhcs != MAKE_WORD(pframe[prem_num+nHCSPos + 1], pframe[prem_num+nHCSPos]))
    {
		printf("\r\n帧头校验错误！\n");
        return -1;
    }
    //整帧校验fcs
    nFCSPos = uwframeLen - 1;//除去FCS自身的长度

    uwfcs = PPPINITFCS16;

    uwfcs = pppfcs16(uwfcs, pframe+prem_num+ 1, nFCSPos - 1);

    if (uwfcs != MAKE_WORD(pframe[prem_num+nFCSPos + 1], pframe[prem_num+nFCSPos]))
    {
		printf("\r\n整帧校验错误！\n");
        return -1;
    }

    dwAPDULen = uwframeLen - SA_Len - 8;

    *pAPDU = &pframe[prem_num + nHCSPos + 2];

    return dwAPDULen;
}

int dwAPduAnalyze(uint8_t *pucBuf, uint32_t dwlen, COLL_STORE_DATA *pstCollStoreData)
{
    int Ret = -1;

    COLL_STORE_DATA *pstCollData = pstCollStoreData;

    if (pucBuf != NULL && GET_Response == pucBuf[0])
    {
		if((GetResponseNormal == pucBuf[1]) || (GetResponseNormalList == pucBuf[1]))
		{
			Ret = dwGetResponseNormal(pucBuf, pstCollData);
		}
		else if((GetResponseRecord == pucBuf[1]) || (GetResponseRecordList == pucBuf[1]))
		{
			//Ret = dwGetResponseRecord(pucBuf, pstCollData);
			return -1;
		}
		else
		{
			//分帧响应
			return -1;
		}
    }

  return Ret;
}

typedef struct{
    uint8_t data_type;
    uint8_t data[];
}DLT_DATA_PAYLOAD;

static uint32_t reversebytes_uint32(uint32_t value){
    return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |
        (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;
}

int convert_to_string_float(uint8_t *ptr, char *res, int math){
    DLT_DATA_PAYLOAD *data_payload = NULL;
    int index = 0;

    int32_t temp;

    switch (*ptr) {
        case DL_ARRAY:
            // LogInfo("type array");
            ptr++;
            int arr_len = *ptr;
            ptr++;
            for (int i = 0; i < arr_len; i++)
            {
                data_payload = (DLT_DATA_PAYLOAD *)ptr;
                switch (data_payload->data_type) {
                    case DL_D_LONG:
                        memcpy(&temp, data_payload->data, 4);
                        // 需要先确定机器大小端
                        temp = reversebytes_uint32(temp);
                        if(i == arr_len - 1)
                        {
                            index += sprintf(res + index, "%.2f", temp * pow(10, math));
                        }
                        else
                        {
                            index += sprintf(res + index, "%.2f,", temp * pow(10, math));
                            // index += sprintf(res + index, "%u.%u,", temp/(int)pow(10, math), temp%(int)pow(10, math));
                        }
                        ptr += 5;
                        break;
                    case DL_D_LONG_UNS:
                        memcpy(&temp, data_payload->data, 4);
                        // 需要先确定机器大小端
                        temp = reversebytes_uint32(temp);
                        if(i == arr_len - 1)
                        {
                            index += sprintf(res + index, "%.2f", temp * pow(10, math));
                        }
                        else
                        {
                            index += sprintf(res + index, "%.2f,", temp * pow(10, math));
                            // index += sprintf(res + index, "%u.%u,", temp/(int)pow(10, math), temp%(int)pow(10, math));
                        }
                        ptr += 5;
                        break;
                    default:
                        // LogError("type error");
                        return 0;
                }
            }
            break;
        default:
            // LogError("type error");
            return 0;
    }
    return 0;
}

DLT698_ANALY_RES * dlt698_response_analy(uint8_t * frame, int length)
{
    uint8_t * pframe = frame;
	uint8_t * apdu = NULL;
	uint8_t * dev_addr = NULL;
	uint8_t addr_len = 0;

	COLL_STORE_DATA * coll_data = malloc(sizeof(COLL_STORE_DATA));

    memset(coll_data, 0, sizeof(COLL_STORE_DATA));

	int dwapdulen = dwGet698Apdu(pframe, &apdu, &dev_addr, &addr_len);

	int rec = dwAPduAnalyze(apdu, dwapdulen, coll_data);

    return coll_data;
}

int dwDlt698DataLen(uint8_t *ucRcvBuf, int dwRcvLen)
{
	assert(ucRcvBuf);

    uint16_t Len = 0;
    uint8_t *Ptr = ucRcvBuf;

    int i = 0, j = 0, Res = 0, varLen = 0, varLenB = 0, LenBytes = 0; // 可变长度

    /* data identify */
    Ptr++;//Ptr自加一指向数据类型
    Len++;//Len=1

    switch (ucRcvBuf[0])
    {
    case DL_NULL:
    {
        /* data length */
        /* data contents */
    }
    break;
    case DL_ARRAY:
    {
        // data length
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        // data contents
        for (i = 0; i < varLen; i++)
        {
            if ((Res = dwDlt698DataLen(Ptr, dwRcvLen - Len)) <= 0)
            {
                return -1;
            }

            Ptr += Res;
            Len += Res;
        }
    }
    break;
    case DL_STRUCT:
    {
        // data length
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        // data contents
        for (i = 0; i < varLen; i++)
        {
            if ((Res = dwDlt698DataLen(Ptr, dwRcvLen - Len)) <= 0)
            {
                return -1;
            }

            Ptr += Res;
            Len += Res;
        }
    }
    break;
    case DL_BOOL:
    {
        /* data length */
        /* data contents */
        Ptr++;
        Len++;
    }
    break;
    case DL_BIT_STR:
    {
        /* data length */
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        /* data contents */
        if (0 == (varLen % 8))
        {
            Ptr += varLen / 8;
            Len += varLen / 8;
        }
        else
        {
            Ptr += (varLen / 8) + 1;
            Len += (varLen / 8) + 1;
        }
    }
    break;
    case DL_D_LONG:
    {
        /* data length */
        /* data contents */
        Ptr += 4;
        Len += 4;
    }
    break;
    case DL_D_LONG_UNS:
    {
        /* data length */
        /* data contents */
        Ptr += 4;
        Len += 4;
    }
    break;
    case DL_OCTET_STR:
    {
        /* data length */
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        /* data contents */
        Ptr += varLen;
        Len += varLen;
    }
    break;
    case DL_VISIBLE_STR:
    {
        /* data length */
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        /* data contents */
        Ptr += varLen;
        Len += varLen;
    }
    break;
    case DL_UTF8_STR:
    {
        /* data length */
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        /* data contents */
        Ptr += varLen;
        Len += varLen;
    }
    break;
    case DL_INTEGER:
    {
        /* data length */
        /* data contents */
        Ptr++;
        Len++;
    }
    break;
    case DL_LONG:
    {
        /* data length */
        /* data contents */
        Ptr += 2;
        Len += 2;
    }
    break;
    case DL_CHAR_UNS:
    {
        /* data length */
        /* data contents */
        Ptr += 1;
        Len += 1;
    }
    break;
    case DL_LONG_UNS:
    {
        /* data length */
        /* data contents */
        Ptr += 2;
        Len += 2;
    }
    break;
    case DL_LONG64:
    {
        /* data length */
        /* data contents */
        Ptr += 8;
        Len += 8;
    }
    break;
    case DL_LONG64_UNS:
    {
        /* data length */
        /* data contents */
        Ptr += 8;
        Len += 8;
    }
    break;
    case DL_ENUM:
    {
        /* data length */
        /* data contents */
        Ptr += 1;
        Len += 1;
    }
    break;
    case DL_FLOAT32:
    {
        /* data length */
        /* data contents */
        Ptr += 4;
        Len += 4;
    }
    break;
    case DL_FLOAT64:
    {
        /* data length */
        /* data contents */
        Ptr += 8;
        Len += 8;
    }
    break;
    case DL_DATE_TIME:
    {
        /* data length */
        /* data contents */
        Ptr += 10;
        Len += 10;
    }
    break;
    case DL_DATE:
    {
        /* data length */
        /* data contents */
        Ptr += 5;
        Len += 5;
    }
    break;
    case DL_TIME:
    {
        /* data length */
        /* data contents */
        Ptr += 3;
        Len += 3;
    }
    break;
    case DL_DATE_TIME_S:
    {
        /* data length */
        /* data contents */
        Ptr += 7;
        Len += 7;
    }
    break;
    case DL_OI:
    {
        /* data length */
        /* data contents */
        Ptr += 2;
        Len += 2;
    }
    break;
    case DL_OAD:
    {
        /* data length */
        /* data contents */
        Ptr += 4;
        Len += 4;
    }
    break;
    case DL_ROAD:
    {
        Ptr += 4;
        Len += 4;

        // OAD number
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        // relevance OAD
        for (i = 0; i < varLen; i++)
        {
            Ptr += 4;
            Len += 4;
        }
    }
    break;
    case DL_OMD:
    {
        /* data length */
        /* data contents */
        Ptr += 4;
        Len += 4;
    }
    break;
    case DL_TI:
    {
        /* data length */
        /* data contents */
        Ptr += 3;
        Len += 3;
    }
    break;
    case DL_TSA: // octet-string
    {
        /* data length */
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        /* data contents */
        Ptr += varLen;
        Len += varLen;
    }
    break;
    case DL_MAC: // octet-string
    case DL_RN:  // octet-string
    {
        /* data length */
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        /* data contents */
        Ptr += varLen;
        Len += varLen;
    }
    break;
    case DL_REGION:
    {
        /* data length */
        /* data contents */
        Ptr += 1;
        Len += 1; // ENUMERATED

        if ((Res = dwDlt698DataLen(Ptr, dwRcvLen - Len)) < 0)
        { // start data
            return -1;
        }
        Ptr += Res;
        Len += Res;

        if ((Res = dwDlt698DataLen(Ptr, dwRcvLen - Len)) < 0)
        { // end data
            return -1;
        }
        Ptr += Res;
        Len += Res;
    }
    break;
    case DL_SCALER_UNIT:
    {
        /* data length */
        /* data contents */
        Ptr += 1;
        Len += 1;

        Ptr += 1;
        Len += 1; // ENUMERATED
    }
    break;
    case DL_RSD:
    {
        if ((Res = dwDlt698RSDLen(Ptr, dwRcvLen - Len)) < 0)
        {
            return -1;
        }

        Ptr += Res;
        Len += Res;
    }
    break;
    case DL_CSD:
    {
        if (*Ptr == 0)
        {
            Ptr += 1; // choice
            Len += 1;

            Ptr += 4; // OAD
            Len += 4;
        }
        else if (*Ptr == 1)
        {
            Ptr += 1; // choice
            Len += 1;

            Ptr += 4; // OAD
            Len += 4;

            // relevance OAD number
            if ((varLenB = dwDlt698Len(Ptr, &LenBytes)) < 0)
            {
                return -1;
            }

            Len += LenBytes;
            Ptr += LenBytes;

            // relevance OAD
            for (j = 0; j < varLenB; j++)
            {
                if (Len > dwRcvLen)
                {
                    return -1;
                }

                Ptr += 4;
                Len += 4;
            }
        }
        else
        {
            return -1;
        }
    }
    break;
    case DL_MS:
    {
        if ((Res = dwDlt698MsLen(Ptr, dwRcvLen - Len)) < 0)
        {
            return -1;
        }

        Ptr += Res;
        Len += Res;
    }
    break;
    case DL_SID:
    {
        /* data length */
        /* data contents */
        Ptr += 4;
        Len += 4;

        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        Ptr += varLen;
        Len += varLen;
    }
    break;
    case DL_SID_MAC:
    {
        /* SID */
        Ptr += 4;
        Len += 4;

        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        Ptr += varLen;
        Len += varLen;

        /* MAC */
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        Ptr += varLen;
        Len += varLen;
    }
    break;
    case DL_COMDCB:
    {
        Ptr += 5;
        Len += 5;
    }
    break;
    case DL_RCSD:
    {
        // CSD number
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        // CSD
        for (i = 0; i < varLen; i++)
        {
            if (Len > dwRcvLen)
            {
                return -1;
            }

            if (*Ptr == 0)
            {
                Ptr += 1; // choice
                Len += 1;

                Ptr += 4; // OAD
                Len += 4;
            }
            else if (*Ptr == 1)
            {
                Ptr += 1; // choice
                Len += 1;

                Ptr += 4; // OAD
                Len += 4;

                // relevance OAD number
                if ((varLenB = dwDlt698Len(Ptr, &LenBytes)) < 0)
                {
                    return -1;
                }

                Len += LenBytes;
                Ptr += LenBytes;

                // relevance OAD
                for (j = 0; j < varLenB; j++)
                {
                    if (Len > dwRcvLen)
                    {
                        return -1;
                    }

                    Ptr += 4;
                    Len += 4;
                }
            }
            else
            {
                return -1;
            }
        }
    }
    break;
    default:
        return -1;
    }

    if (Len > dwRcvLen)
    {
        return -1;
    }

    return Len;
}

int dwDlt698Len(uint8_t *ucRcvBuf, int *LenBytes)
{
	int Bytes;

    assert(ucRcvBuf);

    if (0x00 == (ucRcvBuf[0] & 0x80))
    {
        *LenBytes = 1;
        return ucRcvBuf[0];
    }
#if 1
    Bytes = ucRcvBuf[0] & 0x7F;//保留后七位

    if (Bytes == 1)
    {
        *LenBytes = 2;
        return (ucRcvBuf[1]);
    }
    else if (Bytes == 2)
    {
        *LenBytes = 3;
        return ((ucRcvBuf[1] << 8) | (ucRcvBuf[2]));
    }
    else if (Bytes == 3)
    {
        *LenBytes = 4;
        return (((ucRcvBuf[1] << 16) |
                (ucRcvBuf[2] << 8)) & (ucRcvBuf[3]));
    }
    else if (Bytes == 4)
    {
        *LenBytes = 5;
        return ((((ucRcvBuf[1] << 24) |
                (ucRcvBuf[2] << 16)) & (ucRcvBuf[3] << 8)) & (ucRcvBuf[4]));
    }

    return -1;
#endif

}

int dwDlt698RSDLen(uint8_t *ucRcvBuf, int dwRcvLen)
{
	assert(ucRcvBuf);

    uint16_t Len = 0;
    uint8_t *Ptr = ucRcvBuf;

    int i = 0, Res = 0, varLen = 0, LenBytes = 0; // 可变长度

    if (*Ptr == 0)
    {
        Ptr += 1;
        Len += 1; // choice

        //Ptr += 1;
        //Len += 1; // NULL
    }
    else if (*Ptr == 1)
    {
        Ptr += 1;
        Len += 1; // choice

        Ptr += 4;
        Len += 4; // OAD

        if ((Res = dwDlt698DataLen(Ptr, dwRcvLen - Len)) < 0)
        { // data
            return -1;
        }
        Ptr += Res;
        Len += Res;
    }
    else if (*Ptr == 2)
    {
        Ptr += 1;
        Len += 1; // choice

        Ptr += 4;
        Len += 4; // OAD

        if ((Res = dwDlt698DataLen(Ptr, dwRcvLen - Len)) < 0)
        { // start data
            return -1;
        }
        Ptr += Res;
        Len += Res;

        if ((Res = dwDlt698DataLen(Ptr, dwRcvLen - Len)) < 0)
        { // end data
            return -1;
        }
        Ptr += Res;
        Len += Res;

        if ((Res = dwDlt698DataLen(Ptr, dwRcvLen - Len)) < 0)
        { // TI
            return -1;
        }
        Ptr += Res;
        Len += Res;
    }
    else if (*Ptr == 3)
    {
        Ptr += 1;
        Len += 1; // choice

        // relevance OAD number
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        for (i = 0; i < varLen; i++)
        {
            Ptr += 4;
            Len += 4; // OAD

            if ((Res = dwDlt698DataLen(Ptr, dwRcvLen - Len)) < 0)
            { // start data
                return -1;
            }
            Ptr += Res;
            Len += Res;

            if ((Res = dwDlt698DataLen(Ptr, dwRcvLen - Len)) < 0)
            { // end data
                return -1;
            }
            Ptr += Res;
            Len += Res;

            if ((Res = dwDlt698DataLen(Ptr, dwRcvLen - Len)) < 0)
            { // TI
                return -1;
            }
            Ptr += Res;
            Len += Res;
        }
    }
    else if (*Ptr == 4 || *Ptr == 5)
    {
        Ptr += 1;
        Len += 1; // choice

        Ptr += 7;
        Len += 7; // data_time_s

        if ((Res = dwDlt698MsLen(Ptr, dwRcvLen - Len)) < 0)
        {
            return -1;
        }

        Ptr += Res; // MS
        Len += Res;
    }
    else if (*Ptr == 6 || *Ptr == 7 || *Ptr == 8)
    {
        Ptr += 1;
        Len += 1; // choice

        Ptr += 7;
        Len += 7; // data_time_s

        Ptr += 7;
        Len += 7; // data_time_s

        Ptr += 3;
        Len += 3; // TI

        if ((Res = dwDlt698MsLen(Ptr, dwRcvLen - Len)) < 0)
        {
            return -1;
        }

        Ptr += Res; // MS
        Len += Res;
    }
    else if (*Ptr == 9)
    {
        Ptr += 1;
        Len += 1; // choice

        Ptr += 1;
        Len += 1; // last n
    }
    else if (*Ptr == 10)
    {
        Ptr += 1;
        Len += 1; // choice

        Ptr += 1;
        Len += 1; // last n

        if ((Res = dwDlt698MsLen(Ptr, dwRcvLen - Len)) < 0)
        {
            return -1;
        }

        Ptr += Res; // MS
        Len += Res;
    }
    else
    {
        return -1;
    }

    if (Len > dwRcvLen)
    {
        return -1;
    }

    return Len;
}


int dwDlt698MsLen(uint8_t *ucRcvBuf, int dwRcvLen)
{
	assert(ucRcvBuf);

    uint16_t Len = 0;
    uint8_t *Ptr = ucRcvBuf;

    int i = 0, Res = 0, varLen = 0, varLenB = 0, LenBytes = 0; // 可变长度

    if (*Ptr == 0)
    {
        Ptr += 1;
        Len += 1; // choice - no meter
    }
    else if (*Ptr == 1)
    {
        Ptr += 1;
        Len += 1; // choice - all meter
    }
    else if (*Ptr == 2)
    {
        Ptr += 1;
        Len += 1; // choice - sequence of user type

        // number of user type
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        for (i = 0; i < varLen; i++)
        {
            Len += 1;
            Ptr += 1;
        }
    }
    else if (*Ptr == 3)
    {
        Ptr += 1;
        Len += 1; // choice - sequence of tsa

        // number of user type
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        for (i = 0; i < varLen; i++)
        {
            if ((varLenB = dwDlt698Len(Ptr, &LenBytes)) < 0)
            {
                return -1;
            }

            Len += LenBytes;
            Ptr += LenBytes;

            /* data contents */
            Ptr += varLenB;
            Len += varLenB;
        }
    }
    else if (*Ptr == 4)
    {
        Ptr += 1;
        Len += 1; // choice - sequence of configno

        // number of user type
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        for (i = 0; i < varLen; i++)
        {
            Len += 2;
            Ptr += 2;
        }
    }
    else if (*Ptr == 5 || *Ptr == 6 || *Ptr == 7)
    {
        Ptr += 1;
        Len += 1; // choice - sequence of region

        // number of user type
        if ((varLen = dwDlt698Len(Ptr, &LenBytes)) < 0)
        {
            return -1;
        }

        Len += LenBytes;
        Ptr += LenBytes;

        for (i = 0; i < varLen; i++)
        {
            Ptr += 1;
            Len += 1; // ENUMERATED

            if ((Res = dwDlt698DataLen(Ptr, dwRcvLen - Len)) < 0)
            { // start data
                return -1;
            }
            Ptr += Res;
            Len += Res;

            if ((Res = dwDlt698DataLen(Ptr, dwRcvLen - Len)) < 0)
            { // end data
                return -1;
            }
            Ptr += Res;
            Len += Res;
        }
    }
    else
    {
        return -1;
    }

    if (Len > dwRcvLen)
    {
        return -1;
    }

    return Len;
}