#ifndef _DLT698_45_H_
#define _DLT698_45_H_

#include <stdint.h>
/********************************************************************************************/

#define MAX_APDU_LENGTH 2048              //最长apdu长度
#define MAX_AssociateOAD_NUM_OF_ROAD 32   //最大关联对象属性的个数
#define MAX_TSA_SIZE 16                   //目标服务器地址长度最大字节数
#define MAX_MAC_SIZE 100                  //MAC最大长度（MAC消息鉴别码 Message Authentication Code）
#define MAX_RN_SIZE 100                   //最大随机数长度
#define SID_ADDITIONAL_DATA_SIZE 16       //安全标识最大长度
#define MAX_RCSD_CSD_NUM 20               //RCSD中CSD的最大个数
#define MAX_OAD_NUM_OF_GET_REQUEST 10     //读取请求中最多请求读取对象属性的个数
#define MAX_LEN_OF_DATA 100               //数据最长长度
#define MAX_NUM_OF_ROW 20                 //最多记录数据的个数
#define DLT698_OAD_LEN  4

#define MAKE_WORD(hbyte, lbyte) (((uint8_t)(lbyte)) | (((uint8_t)(hbyte)) << 8))

/***********************************DL/T698.45协议基础数据类型标识***************************
*********************************************************************************************
*********************************************************************************************/
typedef enum
{
    DL_NULL = 0,
    DL_ARRAY,
    DL_STRUCT,
    DL_BOOL,
    DL_BIT_STR,
    DL_D_LONG,
    DL_D_LONG_UNS,
    DL_OCTET_STR = 9,
    DL_VISIBLE_STR,
    DL_UTF8_STR = 12,
    DL_INTEGER = 15,
    DL_LONG,
    DL_CHAR_UNS,
    DL_LONG_UNS,
    DL_LONG64 = 20,
    DL_LONG64_UNS,
    DL_ENUM,
    DL_FLOAT32,
    DL_FLOAT64,
    DL_DATE_TIME,
    DL_DATE,
    DL_TIME,
    DL_DATE_TIME_S,
    DL_OI = 80,
    DL_OAD,
    DL_ROAD,
    DL_OMD,
    DL_TI,
    DL_TSA,
    DL_MAC,
    DL_RN,
    DL_REGION,
    DL_SCALER_UNIT,
    DL_RSD,
    DL_CSD,
    DL_MS,
    DL_SID,
    DL_SID_MAC,
    DL_COMDCB,
    DL_RCSD
} DLT_DATA;

/*****************************DL/T698.45数据类型中的具体定义*********************************
*********************************************************************************************
*********************************************************************************************/
typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day_of_month;
    uint8_t day_of_week;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t milliseconds;
} DATE_TIME;//日期时间数据类型

typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day_of_month;
    uint8_t day_of_week;
} DATE;//日期数据类型

typedef struct
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} TIME;//时间数据类型

typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} DATE_TIME_S;//日期时间数据类型

typedef struct
{
    uint16_t OI_date;  //对象标识
    uint8_t attr_ID;   //属性标识及其特征
    uint8_t attr_index;//属性内元素索引
} OAD;//对象属性描述符

typedef struct
{
    OAD RECODE_OAD;
    uint8_t Associate_OAD_Num;//关联对象属性描述符的个数
    OAD Associate_OAD[MAX_AssociateOAD_NUM_OF_ROAD];
} ROAD;//记录型对象属性描述符

typedef struct
{
    uint16_t OI;          //对象标识
    uint8_t attr_Func;    //方法标识
    uint8_t attr_Operator;//操作模式
} OMD;//对象方法描述符

typedef enum
{
    TI_SECOND = 0,
    TI_MINUTE,
    TI_HOUR,
    TI_DAY,
    TI_MONTH,
    TI_YEAR
} TI_UNIT;//TI单位

typedef struct
{
    TI_UNIT Time_Unit;
    uint16_t uwInterval;//间隔值
} TI;//时间间隔

typedef struct
{
    uint8_t AddrHead;               //SA的首字节,bit0-bit3表示地址长度
    uint8_t Address[MAX_TSA_SIZE];  //目标服务器地址（可变长度）
} TSA;//目标服务器地址

typedef struct
{
    uint8_t MACSize;              // MAC长度
    uint8_t MACData[MAX_MAC_SIZE];//MAC的具体数据
} MAC;//消息鉴别码输出

typedef struct
{
    uint8_t RNNum;          //随机数个数
    uint8_t RN[MAX_RN_SIZE];
} RN;//随机数RN

typedef enum
{
    BEGIN_CLOSE_END_OPEN = 0, //前闭后开
    BEGIN_OPEN_END_CLOSE,     //前开后闭
    BEGIN_CLOSE_END_CLOSE,    //前闭后闭
    BEGIN_OPEN_END_OPEN       //前开后开
} REGION_UNIT;//描述数据的区间范围

typedef enum
{
    PHY_YEAR = 1,                        // 时间---年
    PHY_MONTH,                           // 时间---月
    PHY_WEEK,                            // 时间---周
    PHY_DATE,                            // 时间---日
    PHY_HOUR,                            // 时间---小时
    PHY_MINUTE,                          // 时间---分
    PHY_SECOND,                          // 时间---秒
    PHY_ANGLE,                           //（相）角---度
    PHY_DEGREE_CELSIUS,                  // 摄氏度--- 。C
    PHY_RMB_YUAN,                        // 人民币单位---元
    PHY_METRE,                           // 米---m
    PHY_METRE_PER_SECOND,                // 米每秒---m/s
    PHY_CUBIC_METRE,                     // rV，仪表常数或脉冲值(容积)---m3
    PHY_CUBIC_METRE_CORRECT,             // 修正的体积---m3
    PHY_CUBIC_METRE_PER_HOUR,            // 流量单位-立方米/小时---m3/(60*60s)
    PHY_CUBIC_METRE_PER_HOUR_CORRECT,    //修正的流量单位-立方米/小时---m3/(60*60s)
    PHY_CUBIC_METRE_PER_24HOURS,         // 流量单位-立方米/24小时---m3/(24*60*60s)
    PHY_CUBIC_METRE_PER_24HOURS_CORRECT, // 修正的流量单位-立方米/24小时---m3/(24*60*60s)
    PHY_LITRE,                           // 容积单位-升---L=10-3m3
    PHY_KILOGRAM,                        // 质量单位-千克---kg
    PHY_NEWTON,                          // 力的单位-牛顿---N
    PHY_NEWTON_METRE,                    // 能量单位-牛顿米---Nm J=Nm=Ws
    PHY_PASCAL,                          // 压力单位-帕斯卡---N/m2
    PHY_BAR,                             // 压力单位-巴---10-5N/m2
    PHY_JOULE,                           // 能量单位-焦耳---J=Nm=Ws
    PHY_JOULE_PER_HOUR,                  // 热功单位-焦每小时---J/(60*60s)
    PHY_WATT,                            // 有功功率单位-瓦---W=J/s
    PHY_KILOWATT,                        // 有功功率单位-千瓦---kW=J/(s*1000)
    PHY_VOLT_AMPERE,                     // 视在功率单位-伏安---VA
    PHY_KILOVOLT_AMPERE,                 // 视在功率单位-千伏安---kVA
    PHY_VAR,                             // 无功功率单位-乏---var
    PHY_KILOVAR,                         // 无功功率单位-千乏---kvar
    PHY_KILOWATT_HOUR,                   // 有功能量 rw，有功电能表常数或脉冲值-千瓦-时---kWh=kW*(60*60s)
    PHY_KILOVOLT_AMPERE_HOUR,            // 视在能量 rS，视在电能表常数或脉冲值-千伏-安-小时---kVAh=kVA*(60*60s)
    PHY_KILOVAR_HOUR,                    // 无功能量 rB，无功电能表常数或脉冲值-千乏-时---kvarh=kvar*(60*60s)
    PHY_AMPERE,                          // 电流单位-安培---A
    PHY_COULOMB,                         // 电量单位-库伦---C=As
    PHY_VOLT,                            // 电压单位-伏特---V
    PHY_VOLT_PER_METRE,                  // 电场强度单位-伏每米---V/m
    PHY_FARAD,                           // 电容单位-法拉---F=C/V=As/V
    PHY_OHM,                             // 电阻单位-欧姆---OHM=V/A
    PHY_OHM_METRE,                       // 电阻系数单位---OHMm2/m=OHMm
    PHY_WEBER,                           // 磁通量单位-韦伯---Wb=Vs
    PHY_TESLA,                           // 磁通密度单位-特斯拉---T=Wb/m2
    PHY_AMPERE_PER_METRE,                // 磁场强度单位-安培每米---A/m
    PHY_HENRY,                           // 电感单位-亨利---H=Wb/A
    PHY_HERTZ,                           // 频率单位-赫兹---Hz=1/s
    PHY_FRACTION_OF_WATT_HOUR,           // 有功能量表常数或脉冲值单位-瓦时分之一---1/(Wh)
    PHY_FRACTION_OF_VAR_HOUR,            // 无功能量表常数或脉冲值单位-乏时分之一---1/(varh)
    PHY_FRACTION_OF_VOLT_AMPERE_HOUR,    // 视在能量表常数或脉冲值单位-伏安时分之一---1/(VAh)
    PHY_PERCENT,                         // 百分比 %
    PHY_BYTE,                            // 字节
    PHY_dBm,                             // 分贝毫瓦 dBm
    PHY_RMB_YUAN_PER_KILOVAR_HOUR,       // 电价 元/kWh
    PHY_AMPERE_HOUR,                     // 安时 Ah
    PHY_MILLISECOND,                     // 毫秒 ms

    PHY_OTHER_UNIT = 254,                // 其他单位
    PHY_MISS_UNIT = 255                  // 无单位、缺单位、计数
} PHYSICAL_UNIT;//物理单位

typedef struct
{
    uint8_t ucConvertFactor;       // 换算因子，基数为10
    PHYSICAL_UNIT ePhysicalUnit; // 物理单位
} SCALER_UNIT;//换算及单位

typedef enum
{
    RSD_NULL = 0,
    RSD_SELECTOR1,
    RSD_SELECTOR2,
    RSD_SELECTOR3,
    RSD_SELECTOR4,
    RSD_SELECTOR5,
    RSD_SELECTOR6,
    RSD_SELECTOR7,
    RSD_SELECTOR8,
    RSD_SELECTOR9,
    RSD_SELECTOR10
} RSD;//记录选择描述符

typedef enum
{
    CSD_OF_OAD =0,
    CSD_OF_ROAD
} CSD;//列选择描述符

typedef enum
{
    MS_NONE_METER = 0,            // 无电能表
    MS_ALL_USER_ADDRESS,          // 全部用户地址
    MS_GROUP_USER_TYPE,           // 一组用户类型
    MS_GROUP_USER_ADDRESS,        // 一组用户地址
    MS_GROUP_CONFIG_NO,           // 一组配置序号
    MS_GROUP_USER_TYPE_REGION,    // 一组用户类型区间
    MS_GROUP_USER_ADDRESS_REGION, // 一组用户地址区间
    MS_GROUP_CONFIG_NO_REGION     // 一组配置序号区间
} MS;//电能表集合

typedef struct
{
    uint32_t udwID;                                    //安全标识
    uint8_t Additional_Size;                           //附加数据长度
    uint8_t Additional_Data[SID_ADDITIONAL_DATA_SIZE]; //附加数据 octet-string
} SID;//安全标识

typedef struct
{
    SID SID; //安全标识
    MAC MAC; //数据MAC
} SID_MAC;//带MAC的安全标识

typedef enum
{
    BR_300bps = 0,
    BR_600bps,
    BR_1200bps,
    BR_2400bps,
    BR_4800bps,
    BR_7200bps,
    BR_9600bps,
    BR_19200bps,
    BR_38400bps,
    BR_57600bps,
    BR_115200bps,
    BR_Adaptive = 255
} BAUD_RATE;//波特率

typedef enum
{
    CB_NULL = 0,
    CB_ODD,
    CB_EVEN
} CHECK_BIT;//校验位

typedef enum
{
    DB_5BITS = 5,
    DB_6BITS,
    DB_7BITS,
    DB_8BITS
} DATA_BIT;//数据位

typedef enum
{
    SB_1BIT = 1,
    SB_2BITS,
} STOP_BIT;//停止位

typedef enum
{
    FC_NULL = 0,
    FC_HARDWARE,
    FC_SOFTWARE
} FLOW_CONTROL;//流控

typedef struct
{
    BAUD_RATE eBaudRate;       // 波特率
    CHECK_BIT eCheckBit;       // 校验位
    DATA_BIT eDataBit;         // 数据位
    STOP_BIT eStopBit;         // 停止位
    FLOW_CONTROL eFlowControl; // 流控
} COMDCB;//串口控制块

typedef struct
{
    uint8_t CSD_Num;               //CSD的个数
    CSD stCSD[MAX_RCSD_CSD_NUM]; //每个CSD的具体内容
} RCSD;//记录列选择描述符

typedef enum
{
    DAR_SUCCEED = 0,              // 成功
    DAR_HW_INVALID,               // 硬件失效
    DAR_TMP_INVALID,              // 暂时失效
    DAR_REFUSE_RW,                // 拒绝读写
    DAR_OBJ_NOT_DEF,              // 对象未定义
    DAR_CLASS_NOT_FIT,            // 对象接口类不符合
    DAR_OBJ_NOT_EXIST,            // 对象不存在
    DAR_TYPE_NOT_MATCH,           // 类型不匹配
    DAR_OUT_OF_RANGE,             // 越界
    DAR_DTBLK_NOT_AVAILABLE,      // 数据块不可用
    DAR_SPLIT_FRAME_TRAS_CANCELL, // 分帧传输已取消
    DAR_SPLIT_FRAME_TRAS_NSTAT,   // 不处于分帧传输状态
    DAR_BLKWR_CANCELL,            // 块写取消
    DAR_BLKWR_STATE_NEXIST,       // 不存在块写状态
    DAR_DTBLKNUM_INVILID,         // 数据块序号无效
    DAR_PWD_ERROR,                // 密码错/未授权
    DAR_BAUD_NOCHANG,             // 通信速率不能更改
    DAR_YEAR_OOR,                 // 年时区数超
    DAR_DAY_OOR,                  // 日时段数超
    DAR_TARIFF_OOR,               // 费率数超
    DAR_SECURITY_AUTH_NMATCH,     // 安全认证不匹配
    DAR_RECHARGE_REPEAT,          // 重复充值
    DAR_ESAM_ERROR,               // ESAM验证失败
    DAR_SECURITY_AUTH_FALSE,      // 安全认证失败
    DAR_CUSTOMERCODE_NMATCH,      // 客户编号失败
    DAR_RECHARGE_TIMES_ERROR,     // 充值次数错误
    DAR_HOARD_EXCESS,             // 购电超囤积
    DAR_ADDR_ERROR,               // 地址异常
    DAR_SYMMETRY_DEC_ERROR,       // 对称解密错误
    DAR_NONSYMMETRY_DEC_ERROR,    // 非对称解密错误
    DAR_SIGNATURE_ERROR,          // 签名错误
    DAR_METER_HANG,               // 电能表挂起
    DAR_TIMELAB_INVALID,          // 时间标签无效
    DAR_REQ_TIMEOUT,              // 请求超时
    DAR_ESAM_P1P2_ERROR,          // ESAM的P1P2错误
    DAR_ESAM_LC_ERROR,            // ESAM的LC错误
    DAR_OTHER_ERROR = 255         // 其它
} DAR;//数据访问结果

/*****************************DL/T698.45应用层数据单元类型***********************************
*********************************************************************************************
*********************************************************************************************/
typedef enum
{
    LINK_Request = 1,      //预连接请求
    CONNECT_Request,       //建立应用连接请求
    RELEASE_Request,       //断开应用连接请求
    GET_Request = 5,       //读取请求
    SET_Request,           //设置请求
    ACTION_Request,        //操作请求
    REPORT_Response,       //上报通知
    PROXY_Request,         //代理请求
    SECURITY_Request = 16, //安全请求
    LINK_Response = 129,   //预连接响应
    CONNECT_Response,      //建立应用连接响应
    RELEASE_Response,      //断开应用连接响应
    RELEASE_Notification,  //断开应用连接通知
    GET_Response,          //读取响应
    SET_Response,          //设置响应
    ACTION_Response,       //操作响应
    REPORT_Notification,   //上报通知
    PROXY_Response,        //代理响应
    SECURITY_Response = 144//安全响应
} APDU_ID;   //APDU的服务序号

typedef enum
{
    APDU_LOGIN = 0, // 登录
    APDU_HEARTBEAT, // 心跳
    APDU_LOGOUT     // 退出登录
} APDU_LINK_REQUEST_TYPE;//预连接请求中的请求类型

typedef enum
{
    GetRequestNormal = 1, // 读取一个对象属性请求
    GetRequestNormalList, // 读取若干个对象属性请求
    GetRequestRecord,     // 读取一个记录型对象属性请求
    GetRequestRecordList, // 读取若干个记录型对象属性请求
    GetRequestNext        // 读取分帧响应的下一个数据块请求
} APDU_GET_REQUEST;

typedef enum
{
    GetResponseNormal = 1, // 响应一个对象属性请求
    GetResponseNormalList, // 响应若干个对象属性请求
    GetResponseRecord,     // 响应一个记录型对象属性请求
    GetResponseRecordList, // 响应若干个记录型对象属性请求
    GetResponseNext        // 响应分帧响应的下一个数据块请求
} APDU_GET_RESPONSE;

typedef enum
{
    SetRequestNormal = 1,       // 设置一个对象属性请求
    SetRequestNormalList,       // 设置若干个对象属性请求
    SetThenGetRequestNormalList // 设置后读取若干个对象属性请求
} APDU_SET_REQUEST;

typedef enum
{
    SetResponseNormal = 1,       // 设置一个对象属性的确认信息响应
    SetResponseNormalList,       // 设置若干个对象属性的确认信息响应
    SetThenGetResponseNormalList // 设置后读取若干个对象属性的确认信息响应
} APDU_SET_RESPONSE;

typedef enum
{
    ActionRequest = 1,             // 操作一个对象方法请求
    ActionRequestList,             // 操作若干个对象方法请求
    ActionThenGetRequestNormalList // 操作若干个对象方法后读取若干个对象属性请求
} APDU_ACTION_REQUEST;

typedef enum
{
    ActionResponseNormal = 1,             // 操作一个对象方法的响应
    ActionResponseNormalList,             // 操作若干个对象方法的响应
    ActionThenGetResponseNormalList       // 操作若干个对象方法后读取若干个对象属性的响应
} APDU_ACTION_RESPONSE;

typedef enum
{
    ReportNotificationList = 1,   // 上报若干个对象属性
    ReportNotificationRecordList, // 上报若干个记录型对象属性
} APDU_REPORT_NOTIFICATION;

typedef enum
{
    ReportResponseList = 1,   // 上报若干个对象属性的响应
    ReportResponseRecordList, // 上报若干个记录型对象属性的响应
} APDU_REPORT_RESPONSE;

typedef enum
{
    ProxyGetRequestList = 1,       // 代理读取若干个服务器的若干个对象属性请求
    ProxyGetRequestRecord,         // 代理读取一个服务器的一个记录型对象属性请求
    ProxySetRequestList,           // 代理设置若干个服务器的若干个对象属性请求
    ProxySetThenGetRequestList,    // 代理设置后读取若干个服务器的若干个对象属性请求
    ProxyActionRequestList,        // 代理操作若干个服务器的若干个对象方法请求
    ProxyActionThenGetRequestList, // 代理操作后读取若干个服务器的若干个对象方法和属性请求
    ProxyTransCommandRequest       // 代理透明转发命令请求
} APDU_PROXY_REQUEST;

typedef enum
{
    ProxyGetResponseList = 1,       // 代理读取若干个服务器的若干个对象属性响应
    ProxyGetResponseRecord,         // 代理读取一个服务器的一个记录型对象属性响应
    ProxySetResponseList,           // 代理设置若干个服务器的若干个对象属性响应
    ProxySetThenGetResponseList,    // 代理设置后读取若干个服务器的若干个对象属性响应
    ProxyActionResponseList,        // 代理操作若干个服务器的若干个对象方法响应
    ProxyActionThenGetResponseList, // 代理操作后读取若干个服务器的若干个对象方法和属性响应
    ProxyTransCommandResponse      // 代理透明转发命令响应
} APDU_PROXY_RESPONSE;

typedef struct
{
    DATE_TIME_S TransTime;//发送时标
    TI TransDelay;        //允许传输延时时间
} TIME_TAG;//时间标签

typedef enum
{
    ERROR_INFO = 0,  //错误信息
    RES_DATA         //数据
} GET_RESULT;//读取结果

/*****************************DL/T698.45对象标识定义*****************************************
*********************************************************************************************
*********************************************************************************************/
// A.1 电能量类对象标识
#define OI_CMB_ACT 0x0000                  // 组合有功电能
#define OI_FWACT 0x0010                    // 正向有功电能
#define OI_AFWACT 0x0011                   // A相正向有功电能
#define OI_BFWACT 0x0012                   // B相正向有功电能
#define OI_CFWACT 0x0013                   // C相正向有功电能
#define OI_RVERACT 0x0020                  // 反向有功电能
#define OI_ARVERACT 0x0021                 // A相反向有功电能
#define OI_BRVERACT 0x0022                 // B相反向有功电能
#define OI_CRVERACT 0x0023                 // C相反向有功电能
#define OI_CMB_REACT_1 0x0030              // 组合无功1电能
#define OI_ACMB_REACT_1 0x0031             // A相组合无功1电能
#define OI_BCMB_REACT_1 0x0032             // B相组合无功1电能
#define OI_CCMB_REACT_1 0x0033             // C相组合无功1电能
#define OI_CMB_REACT_2 0x0040              // 组合无功2电能
#define OI_ACMB_REACT_2 0x0041             // A相组合无功2电能
#define OI_BCMB_REACT_2 0x0042             // B相组合无功2电能
#define OI_CCMB_REACT_2 0x0043             // C相组合无功2电能
#define OI_FSTQUAD_REACT 0x0050            // 第一象限无功电能
#define OI_AFSTQUAD_REACT 0x0051           // A相第一象限无功电能
#define OI_BFSTQUAD_REACT 0x0052           // B相第一象限无功电能
#define OI_CFSTQUAD_REACT 0x0053           // C相第一象限无功电能
#define OI_SNDQUAD_REACT 0x0060            // 第二象限无功电能
#define OI_ASNDQUAD_REACT 0x0061           // A相第二象限无功电能
#define OI_BSNDQUAD_REACT 0x0062           // B相第二象限无功电能
#define OI_CSNDQUAD_REACT 0x0063           // C相第二象限无功电能
#define OI_THDQUAD_REACT 0x0070            // 第三象限无功电能
#define OI_ATHDQUAD_REACT 0x0071           // A相第三象限无功电能
#define OI_BTHDQUAD_REACT 0x0072           // B相第三象限无功电能
#define OI_CTHDQUAD_REACT 0x0073           // C相第三象限无功电能
#define OI_FUTHQUAD_REACT 0x0080           // 第四象限无功电能
#define OI_AFUTHQUAD_REACT 0x0081          // A相第四象限无功电能
#define OI_BFUTHQUAD_REACT 0x0082          // B相第四象限无功电能
#define OI_CFUTHQUAD_REACT 0x0083          // C相第四象限无功电能
#define OI_FW_APARNT 0x0090                // 正向视在电能
#define OI_AFW_APARNT 0x0091               // A相正向视在电能
#define OI_BFW_APARNT 0x0092               // B相正向视在电能
#define OI_CFW_APARNT 0x0093               // C相正向视在电能
#define OI_RVER_APARNT 0x00A0              // 反向视在电能
#define OI_ARVER_APARNT 0x00A1             // A相反向视在电能
#define OI_BRVER_APARNT 0x00A2             // B相反向视在电能
#define OI_CRVER_APARNT 0x00A3             // C相反向视在电能
#define OI_FWACT_FUDMTAL_WAVEALL 0x0110    // 正向有功基波总电能
#define OI_AFWACT_FUDMTAL_WAVE 0x0111      // A相正向有功基波电能
#define OI_BFWACT_FUDMTAL_WAVE 0x0112      // B相正向有功基波电能
#define OI_CFWACT_FUDMTAL_WAVE 0x0113      // C相正向有功基波电能
#define OI_RVERACT_FUDMTAL_WAVEALL 0x0120  // 反向有功基波总电能
#define OI_ARVERACT_FUDMTAL_WAVE 0x0121    // A相反向有功基波电能
#define OI_BRVERACT_FUDMTAL_WAVE 0x0122    // B相反向有功基波电能
#define OI_CRVERACT_FUDMTAL_WAVE 0x0123    // C相反向有功基波电能
#define OI_FWACT_HARMONIC_WAVEALL 0x0210   // 正向有功谐波总电能
#define OI_AFWACT_HARMONIC_WAVE 0x0211     // A相正向有功谐波电能
#define OI_BFWACT_HARMONIC_WAVE 0x0212     // B相正向有功谐波电能
#define OI_CFWACT_HARMONIC_WAVE 0x0213     // C相正向有功谐波电能
#define OI_RVERACT_HARMONIC_WAVEALL 0x0220 // 反向有功谐波总电能
#define OI_ARVERACT_HARMONIC_WAVE 0x0221   // A相反向有功谐波电能
#define OI_BRVERACT_HARMONIC_WAVE 0x0222   // B相反向有功谐波电能
#define OI_CRVERACT_HARMONIC_WAVE 0x0223   // C相反向有功谐波电能
#define OI_COPPERLOSS_ACT_COMPALL 0x0300   // 铜损有功总电能补偿量
#define OI_ACOPPERLOSS_ACT_COMP 0x0301     // A相铜损有功电能补偿量
#define OI_BCOPPERLOSS_ACT_COMP 0x0302     // B相铜损有功电能补偿量
#define OI_CCOPPERLOSS_ACT_COMP 0x0303     // C相铜损有功电能补偿量
#define OI_IRONLOSS_ACT_COMPALL 0x0400     // 铁损有功总电能补偿量
#define OI_AIRONLOSS_ACT_COMP 0x0401       // A相铁损有功电能补偿量
#define OI_BIRONLOSS_ACT_COMP 0x0402       // B相铁损有功电能补偿量
#define OI_CIRONLOSS_ACT_COMP 0x0403       // C相铁损有功电能补偿量
#define OI_RELEVANT_ALL 0x0500             // 关联总电能
#define OI_ARELEVANT 0x0501                // A相关联电能
#define OI_BRELEVANT 0x0502                // B相关联电能
#define OI_CRELEVANT 0x0503                // C相关联电能
// A.2 最大需量类对象标识
#define OI_FWACT_MAXDEM 0x1010            // 正向有功最大需量
#define OI_AFWACT_MAXDEM 0x1011           // A相正向有功最大需量
#define OI_BFWACT_MAXDEM 0x1012           // B相正向有功最大需量
#define OI_CFWACT_MAXDEM 0x1013           // C相正向有功最大需量
#define OI_RVERACT_MAXDEM 0x1020          // 反向有功最大需量
#define OI_ARVERACT_MAXDEM 0x1021         // A相反向有功最大需量
#define OI_BRVERACT_MAXDEM 0x1022         // B相反向有功最大需量
#define OI_CRVERACT_MAXDEM 0x1023         // C相反向有功最大需量
#define OI_CMB_REACT1_MAXDEM 0x1030       // 组合无功1最大需量
#define OI_ACMB_REACT1_MAXDEM 0x1031      // A相组合无功1最大需量
#define OI_BCMB_REACT1_MAXDEM 0x1032      // B相组合无功1最大需量
#define OI_CCMB_REACT1_MAXDEM 0x1033      // C相组合无功1最大需量
#define OI_CMB_REACT2_MAXDEM 0x1040       // 组合无功2最大需量
#define OI_ACMB_REACT2_MAXDEM 0x1041      // A相组合无功2最大需量
#define OI_BCMB_REACT2_MAXDEM 0x1042      // B相组合无功2最大需量
#define OI_CCMB_REACT2_MAXDEM 0x1043      // C相组合无功2最大需量
#define OI_FSTQUAD_MAXDEM 0x1050          // 第一象限最大需量
#define OI_AFSTQUAD_MAXDEM 0x1051         // A相第一象限最大需量
#define OI_BFSTQUAD_MAXDEM 0x1052         // B相第一象限最大需量
#define OI_CFSTQUAD_MAXDEM 0x1053         // C相第一象限最大需量
#define OI_SNDQUAD_MAXDEM 0x1060          // 第二象限最大需量
#define OI_ASNDQUAD_MAXDEM 0x1061         // A相第二象限最大需量
#define OI_BSNDQUAD_MAXDEM 0x1062         // B相第二象限最大需量
#define OI_CSNDQUAD_MAXDEM 0x1063         // C相第二象限最大需量
#define OI_THDQUAD_MAXDEM 0x1070          // 第三象限最大需量
#define OI_ATHDQUAD_MAXDEM 0x1071         // A相第三象限最大需量
#define OI_BTHDQUAD_MAXDEM 0x1072         // B相第三象限最大需量
#define OI_CTHDQUAD_MAXDEM 0x1073         // C相第三象限最大需量
#define OI_FUTHQUAD_MAXDEM 0x1080         // 第四象限最大需量
#define OI_AFUTHQUAD_MAXDEM 0x1081        // A相第四象限最大需量
#define OI_BFUTHQUAD_MAXDEM 0x1082        // B相第四象限最大需量
#define OI_CFUTHQUAD_MAXDEM 0x1083        // C相第四象限最大需量
#define OI_FW_APARNT_MAXDEM 0x1090        // 正向视在最大需量
#define OI_AFW_APARNT_MAXDEM 0x1091       // A相正向视在最大需量
#define OI_BFW_APARNT_MAXDEM 0x1092       // B相正向视在最大需量
#define OI_CFW_APARNT_MAXDEM 0x1093       // C相正向视在最大需量
#define OI_RVER_APARNT_MAXDEM 0x10A0      // 反向视在最大需量
#define OI_ARVER_APARNT_MAXDEM 0x10A1     // A相反向视在最大需量
#define OI_BRVER_APARNT_MAXDEM 0x10A2     // B相反向视在最大需量
#define OI_CRVER_APARNT_MAXDEM 0x10A3     // C相反向视在最大需量
#define OI_FRZ_FWACT_MAXDEM 0x1110        // 冻结周期内正向有功最大需量
#define OI_FRZ_AFWACT_MAXDEM 0x1111       // 冻结周期内A相正向有功最大需量
#define OI_FRZ_BFWACT_MAXDEM 0x1112       // 冻结周期内B相正向有功最大需量
#define OI_FRZ_CFWACT_MAXDEM 0x1113       // 冻结周期内C相正向有功最大需量
#define OI_FRZ_RVERACT_MAXDEM 0x1120      // 冻结周期内反向有功最大需量
#define OI_FRZ_ARVERACT_MAXDEM 0x1121     // 冻结周期内A相反向有功最大需量
#define OI_FRZ_BRVERACT_MAXDEM 0x1122     // 冻结周期内B相反向有功最大需量
#define OI_FRZ_CRVERACT_MAXDEM 0x1123     // 冻结周期内C相反向有功最大需量
#define OI_FRZ_CMB_REACT1_MAXDEM 0x1130   // 冻结周期内组合无功1最大需量
#define OI_FRZ_ACMB_REACT1_MAXDEM 0x1131  // 冻结周期内A相组合无功1最大需量
#define OI_FRZ_BCMB_REACT1_MAXDEM 0x1132  // 冻结周期内B相组合无功1最大需量
#define OI_FRZ_CCMB_REACT1_MAXDEM 0x1133  // 冻结周期内C相组合无功1最大需量
#define OI_FRZ_CMB_REACT2_MAXDEM 0x1140   // 冻结周期内组合无功2最大需量
#define OI_FRZ_ACMB_REACT2_MAXDEM 0x1141  // 冻结周期内A相组合无功2最大需量
#define OI_FRZ_BCMB_REACT2_MAXDEM 0x1142  // 冻结周期内B相组合无功2最大需量
#define OI_FRZ_CCMB_REACT2_MAXDEM 0x1143  // 冻结周期内C相组合无功2最大需量
#define OI_FRZ_FSTQUAD_MAXDEM 0x1150      // 冻结周期内第一象限最大需量
#define OI_FRZ_AFSTQUAD_MAXDEM 0x1151     // 冻结周期内A相第一象限最大需量
#define OI_FRZ_BFSTQUAD_MAXDEM 0x1152     // 冻结周期内B相第一象限最大需量
#define OI_FRZ_CFSTQUAD_MAXDEM 0x1153     // 冻结周期内C相第一象限最大需量
#define OI_FRZ_SNDQUAD_MAXDEM 0x1160      // 冻结周期内第二象限最大需量
#define OI_FRZ_ASNDQUAD_MAXDEM 0x1161     // 冻结周期内A相第二象限最大需量
#define OI_FRZ_BSNDQUAD_MAXDEM 0x1162     // 冻结周期内B相第二象限最大需量
#define OI_FRZ_CSNDQUAD_MAXDEM 0x1163     // 冻结周期内C相第二象限最大需量
#define OI_FRZ_THDQUAD_MAXDEM 0x1170      // 冻结周期内第三象限最大需量
#define OI_FRZ_ATHDQUAD_MAXDEM 0x1171     // 冻结周期内A相第三象限最大需量
#define OI_FRZ_BTHDQUAD_MAXDEM 0x1172     // 冻结周期内B相第三象限最大需量
#define OI_FRZ_CTHDQUAD_MAXDEM 0x1173     // 冻结周期内C相第三象限最大需量
#define OI_FRZ_FUTHQUAD_MAXDEM 0x1180     // 冻结周期内第四象限最大需量
#define OI_FRZ_AFUTHQUAD_MAXDEM 0x1181    // 冻结周期内A相第四象限最大需量
#define OI_FRZ_BFUTHQUAD_MAXDEM 0x1182    // 冻结周期内B相第四象限最大需量
#define OI_FRZ_CFUTHQUAD_MAXDEM 0x1183    // 冻结周期内C相第四象限最大需量
#define OI_FRZ_FW_APARNT_MAXDEM 0x1190    // 冻结周期内正向视在最大需量
#define OI_FRZ_AFW_APARNT_MAXDEM 0x1191   // 冻结周期内A相正向视在最大需量
#define OI_FRZ_BFW_APARNT_MAXDEM 0x1192   // 冻结周期内B相正向视在最大需量
#define OI_FRZ_CFW_APARNT_MAXDEM 0x1193   // 冻结周期内C相正向视在最大需量
#define OI_FRZ_RVER_APARNT_MAXDEM 0x11A0  // 冻结周期内反向视在最大需量
#define OI_FRZ_ARVER_APARNT_MAXDEM 0x11A1 // 冻结周期内A相反向视在最大需量
#define OI_FRZ_BRVER_APARNT_MAXDEM 0x11A2 // 冻结周期内B相反向视在最大需量
#define OI_FRZ_CRVER_APARNT_MAXDEM 0x11A3 // 冻结周期内C相反向视在最大需量
//A.3变量类对象标识
#define OI_VOLTAGE 0x2000       // 电压
#define OI_CURRENT 0x2001       // 电流
#define OI_VOLPhANGLE 0x2002    // 电压相角
#define OI_CURRPhANGLE 0x2003   // 电压电流相角
#define OI_ACTIVEPOWER 0x2004   // 有功功率
#define OI_REACTIVEPOWER 0x2005 // 无功功率
#define OI_APPARENTPOWER 0x2006 // 视在功率
#define OI_1MIN_AVERAGEACTPWR 0x2007        // 一分钟平均有功功率
#define OI_1MIN_AVERAGEREACTPWR 0x2008      // 一分钟平均无功功率
#define OI_1MIN_AVERAGEAPARNTPWR 0x2009     // 一分钟平均视在功率
#define OI_POWERFACTOR 0x200A               // 功率因数
#define OI_VOLTAGEWAVEDISTORTION 0x200B     // 电压波形失真度
#define OI_CURRENTWAVEDISTORTION 0x200C     // 电流波形失真度
#define OI_VOLTAGE_HARMONICCONTENT 0x200D   // 电压谐波含有量
#define OI_PWR_GRID_FREQUENCY 0x200F        // 电网频率
#define OI_METER_INSIDE_TEMPER 0x2010       // 表内温度
#define OI_CLOCK_BAT_VOLT 0x2011            // 时钟电池电压
#define OI_PWR_OFF_READING_BATVOLT 0x2012   // 停电抄表电池电压
#define OI_CLOCK_BAT_WORKING_TIME 0x2013    // 时钟电池工作时间
#define OI_METER_RUN_SWORD 0x2014           // 电能表运行状态字
#define OI_METER_FOLLOW_REPORT_SWORD 0x2015 // 电能表跟随上报状态字
#define OI_PRE_ACT_DEM 0x2017               // 当前有功需量
#define OI_PRE_REACT_DEM 0x2018             // 当前无功需量
#define OI_PRE_APARNT_DEM 0x2019            // 当前视在需量
#define OI_PRE_ELECTRPRICE 0x201A           // 当前电价
#define OI_PRE_TARIFF_ELECPRICE 0x201B      // 当前费率电价
#define OI_PRE_STEP_ELECPRICE 0x201C        // 当前阶梯电价
#define OI_OCCURRENCE_TIME 0x201E           // 事件发生时间
#define OI_END_TIME 0x2020                      // 事件结束时间
#define OI_DATA_FRZ_TIME 0x2021                 // 数据冻结时间
#define OI_EVENT_RCRD_NUM 0x2022                // 时间记录序号
#define OI_FRZ_RCRD_NUM 0x2023                  // 冻结记录序号
#define OI_EVENT_SOURCE 0x2024                  // 事件发生源
#define OI_EVENT_PRE_VALUE 0x2025               // 事件当前值
#define OI_VOLT_UNBALANCE_RATE 0x2026           // 电压不平衡率
#define OI_CURRENT_UNBALANCE_RATE 0x2027        // 电流不平衡率
#define OI_LOAD_RATE 0x2028                     // 负载率
#define OI_AH_VALUE 0x2029                      // 安时值
#define OI_TARGET_SERVER_ADDR 0x202A            // 目标服务器地址
#define OI_PRE_WALLET_FILE 0x202C               // 当前钱包文件
#define OI_PRE_OVERDRAFT_AMOUNT 0x202D          // 当前透支金额
#define OI_CUMULATIVE_AMOUNT 0x202E             // 累计购电金额
#define OI_MONTHLY_ELECTR 0x2031                // 月度用电量
#define OI_STEP_STLMNT_ELECTR 0x2032            // 阶梯结算用电量
#define OI_CTRL_CMD_EXE_SWORD 0x2040            // 控制命令执行状态字
#define OI_CTRL_CMD_ERROR_SWORD 0x2041          // 控制命令错误状态字
#define OI_MINUTEREGION_STATISTICS 0x2100       // 分钟区间统计
#define OI_HOURREGION_STATISTICS 0x2101         // 小时区间统计
#define OI_DAYREGION_STATISTICS 0x2102          // 日区间统计
#define OI_MONTHREGION_STATISTICS 0x2103        // 月区间统计
#define OI_YEARREGION_STATISTICS 0x2104         // 年区间统计
#define OI_MINUTE_AVERAGE 0x2110                // 分钟区间平均
#define OI_HOUR_AVERAGE 0x2111                  // 小时平均
#define OI_DAY_AVERAGE 0x2112                   // 日平均
#define OI_MONTH_AVERAGE 0x2113                 // 月平均
#define OI_YEAR_AVERAGE 0x2114                  // 年平均
#define OI_MINUTE_EXTREMUM 0x2120               // 分钟极值
#define OI_HOUR_EXTREMUM 0x2121                 // 小时极值
#define OI_DAY_EXTREMUM 0x2122                  // 日极值
#define OI_MONTH_EXTREMUM 0x2123                // 月极值
#define OI_YEAR_EXTREMUM 0x2124                 // 年极值
#define OI_PHASEA_VOL_QUALIFICATION_RATE 0x2131 //A相电压合格率
#define OI_PHASEB_VOL_QUALIFICATION_RATE 0x2132 //B相电压合格率
#define OI_PHASEC_VOL_QUALIFICATION_RATE 0x2133 //C相电压合格率
#define OI_DAY_MAXACTPWR_TIME 0x2140            // 日最大有功功率及发生时间
#define OI_MONTH_MAXACTPWR_TIME 0x2141          // 月最大有功功率及发生时间
#define OI_COMMFLOW 0x2200
#define OI_RUNTIME 0x2203
#define OI_RESETTM 0x2204
#define OI_ESAM 0xF100
#define OI_ESAMSAFEMODEPARA 0xF101
#define OI_TG1 0x2301
#define OI_TG2 0x2302
#define OI_TG3 0x2303
#define OI_TG4 0x2304
#define OI_TG5 0x2305
#define OI_TG6 0x2306
#define OI_TG7 0x2307
#define OI_TG8 0x2308
#define OI_REMOTE_CONTROL 0x8000
#define OI_ENSURE_POWER_SERVING 0x8001
#define OI_DUN_ALARM 0x8002
#define OI_SAFETY_VALUE 0x8100
#define OI_CONTROL_PERIOD 0x8101
#define OI_PCONTROL_ALARM_TIME 0x8102
#define OI_PMETER1 0x2401
#define OI_PMETER2 0x2402
#define OI_PMETER3 0x2403
#define OI_PMETER4 0x2404
#define OI_PMETER5 0x2405
#define OI_PMETER6 0x2406
#define OI_PMETER7 0x2407
#define OI_PMETER8 0x2408
//  A.4 事件类对象标识
#define OI_METER_LOSS_VOLT 0x3000             // 电能表失压事件
#define OI_METER_UNDER_VOLT 0x3001            // 电能表欠压事件
#define OI_METER_OVER_VOLT 0x3002             // 电能表过压事件
#define OI_METER_PHASE_BREAKING 0x3003        // 电能表断相事件
#define OI_METER_LOSS_CURRENT 0x3004          // 电能表失流事件
#define OI_METER_OVER_CURRENT 0x3005          // 电能表过流事件
#define OI_METER_CURRENT_BREAK 0x3006         // 电能表断流事件
#define OI_METER_PWR_RVER 0x3007              // 电能表功率反向事件
#define OI_METER_OVERLOAD 0x3008              // 电能表过载事件
#define OI_METER_FWACT_DEM_BEYOND 0x3009      // 电能表正向有功需量超限事件
#define OI_METER_RVERACT_DEM_BEYOND 0x300A    // 电能表反向有功需量超限事件
#define OI_METER_REACT_DEM_BEYOND 0x300B      // 电能表无功需量超限事件
#define OI_METER_PWRFACTOR_LOWERLIMIT 0x300C  // 电能表功率因数超下限事件
#define OI_METER_ALL_LOSS_VOLTAGE 0x300D      // 电能表全失压事件
#define OI_METER_AUXILIARY_PWR_DOWN 0x300E    // 电能表辅助电源掉电事件
#define OI_METER_VOLTAGE_RVER_PH 0x300F       // 电能表电压逆相序事件
#define OI_METER_CURRENT_RVER_PH 0x3010       // 电能表电流逆相序事件
#define OI_METER_PWR_DOWN 0x3011              // 电能表掉电事件
#define OI_METER_PROG 0x3012                  // 电能表编程事件
#define OI_METER_RESET 0x3013                 // 电能表清零事件
#define OI_METER_DEM_RESET 0x3014             // 电能表需量清零事件
#define OI_METER_EVENT_RESET 0x3015           // 电能表事件清零事件
#define OI_METER_CALI_TIME 0x3016             // 电能表校时事件
#define OI_METER_TI_TAB_PROG 0x3017           // 电能表时段表编程事件
#define OI_METER_TZ_TAB_PROG 0x3018           // 电能表时区表编程事件
#define OI_METER_WEEKEND_PROG 0x3019          // 电能表周休日编程事件
#define OI_METER_STLMNT_DATE_PROG 0x301A      // 电能表结算日编程事件
#define OI_METER_UNCAP 0x301B                 // 电能表开盖事件
#define OI_METER_OPEN_BUTTON_BOX 0x301C       // 电能表开端钮盖事件
#define OI_METER_VOLT_UNBALANCE 0x301D        // 电能表电压不平衡事件
#define OI_METER_CURRENT_UNBALANCE 0x301E     // 电能表电流不平衡事件
#define OI_METER_TRIPPING_OPERATION 0x301F    // 电能表跳闸事件
#define OI_METER_SWITCH_ON 0x3020             // 电能表合闸事件
#define OI_METER_HOLIDAY_PROG 0x3021          // 电能表节假日编程
#define OI_METER_ACT_CMB_PROG 0x3022          // 电能表有功组合方式编程事件
#define OI_METER_REACT_CMB_PROG 0x3023        // 电能表无功组合方式编程事件
#define OI_METER_TARIFFPARA_PROG 0x3024       // 电能表费率参数表编程事件
#define OI_METER_STEP_TAB_PROG 0x3025         // 电能表阶梯表编程事件
#define OI_METER_SECRET_KEY_UPDATE 0x3026     // 电能表秘钥更新事件
#define OI_METER_ABNORMAL_CARD 0x3027         // 电能表异常插卡事件
#define OI_METER_PURCHASE_ELECTR_RCRD 0x3028  // 电能表购电记录
#define OI_METER_REFUND_RCRD 0x3029           // 电能表退费记录
#define OI_METER_MAGNETIC_INTERFERENCE 0x302A // 电能表恒定磁场干扰事件
#define OI_METER_LOADSWITCH_MISOPERATE 0x302B // 电能表负荷开关误动作事件
#define OI_METER_PWR_ABNORMAL 0x302C          // 电能表电源异常事件
#define OI_METER_CURRENT_SERUNBALANCE 0x302D  // 电能表电流严重不平衡事件
#define OI_METER_CLOCK_FAULT 0x302E           // 电能表时钟故障事件
#define OI_METER_METERING_CHIP_FAILURE 0x302F // 电能表计量芯片故障事件
#define OI_COMM_MODULE_CHANGE 0x3030          // 通信模块变更事件
#define OI_TER_INITIALIZATION 0x3100            // 终端初始化事件
#define OI_TER_VERSION_CHANGE 0x3101            // 终端版本变更事件
#define OI_TER_STATUS_CHANGE 0x3104             // 终端状态量变位事件
#define OI_CLOCK_BEYOND 0x3105                  // 电能表时钟超差事件
#define OI_TERMINAL_OUTAGE 0x3106               // 终端停/上电事件
#define OI_DC_UPPER_LIMIT 0x3107                // 终端直流模拟量越上限事件
#define OI_DC_LOWER_LIMIT 0x3108                // 终端直流模拟量越下限事件
#define OI_TER_MSG_CERTIFY 0x3109               // 终端消息认证错误事件
#define OI_DEVICE_FAULT 0x310A                  // 设备故障记录
#define OI_METER_DATA_FALL 0x310B               // 电能表示度下降事件
#define OI_ENERGY_BEYOND 0x310C                 // 电能量超差事件
#define OI_METER_FLY 0x310D                     // 电能表飞走事件
#define OI_METER_STOP 0x310E                    // 电能表停走事件
#define OI_COLLECT_FAIL 0x310F                  // 终端抄表失败事件
#define OI_COMM_FLOW_BEYOND 0x3110              // 月通信流量超限事件
#define OI_FIND_UNKNOWN_METER 0x3111            // 发现未知电能表事件
#define OI_CROSS_REGION_METER 0x3112            // 跨台区电能表事件
#define OI_SET_CLOCK 0x3114                     // 终端对时事件
#define OI_YK 0x3115                            // 遥控跳闸记录
#define OI_ACT_EXCEED_LIMIT_RCRD 0x3116         // 有功总电能差动越限事件记录
#define OI_OUTLOOP_STAT_CHANGE 0x3117           // 输出回路接入状态变位事件记录
#define OI_TER_PROG_RCRD 0x3118                 // 终端编程记录
#define OI_TER_CURRENTLOOP_ABNORMAL 0x3119      // 终端电流回路异常事件
#define OI_METER_NET_STATE_SWITCH 0x311A        // 电能表在网状态切换事件
#define OI_TER_METER_CALI_RCRD 0x311B           // 终端对电表校时记录
#define OI_METER_DATA_CHANGE 0x311C             // 电能表数据变更监控记录
#define OI_PWR_CTRL_TRIP_RCRD 0x3200            // 功控跳闸记录
#define OI_ELECCTRL_TRIP_RCRD 0x3201            // 电控跳闸记录
#define OI_PURCHASE_ELEC_PARA_SET_RCRD 0x3202   // 购电参数设置记录
#define OI_ELEC_CTRL_ALARM_RCRD 0x3203          // 电控告警事件记录
#define OI_CHANNELREPORTINGSTATUS 0x3300        // 通道上报状态
#define OI_STANDARD_RCRD_UNIT 0x3301            // 标准事件记录单元
#define OI_PROG_RCRD_UNIT 0x3302                // 编程记录事件单元
#define OI_FIND_UNKNOWNMETER_UNIT 0x3303        // 发现未知电能表事件单元
#define OI_CROSS_REGIONMETER_UNIT 0x3304        // 跨台区电能表事件单元
#define OI_PWRCTRL_TRIP_RCRDUNIT 0x3305         // 功控跳闸记录单元
#define OI_ELECTRONICCTRL_TRIPRCRDUNIT 0x3306   // 电控跳闸记录单元
#define OI_ELECTRONICCTRL_ALARMRCRDUNIT 0x3307  // 电控告警事件单元
#define OI_METER_DEM_EXCEEDLIMITUNIT 0x3208     // 电能表需量超限事件单元
#define OI_PWR_ON_OFF_RCRDUNIT 0x3209           // 停/上电事件记录单元
#define OI_REMOTE_RCRDUNIT 0x330A               // 遥控事件记录单元
#define OI_ACTALLDIFFEXCEED_RCRDUNIT 0x330B     // 有功总电能量差动越限事件记录单元
#define OI_EVENT_CLEAR_EVENT_RCRDUNIT 0x330C    // 事件清零事件记录单元
#define OI_TER_METER_CALI_UNIT 0x330D           // 终端对电表校时记录单元
#define OI_METER_NET_STATESWITCH_UNIT 0x330E    // 电能表在网状态切换事件单元
#define OI_METER_DATACHANGEMONI_RCRDUNIT 0x330F // 电能表数据变更监控记录单元
#define OI_METER_ABNORMALCARDRCRD_UNIT 0x3310   // 电能表异常插卡记录单元
#define OI_METER_REFUNDRCRD_UNIT 0x3311         // 退费事件记录单元
//A.5参变量类对象标识
#define OI_DATETIME 0x4000                     // 日期时间
#define OI_COMMADDR 0x4001                     // 通信地址
#define OI_METER_NO 0X4002                     // 表号
#define OI_CUSTOMER_NO 0X4003                  // 客户编号
#define OI_POSITION 0x4004                     // 设备地理位置
#define OI_GROUPADDR 0x4005                    // 组地址
#define OI_CLKSOURCE 0x4006                    // 时钟源
#define OI_LCDPARA 0x4007                      // LCD参数
#define OI_TWO_TZ_TAB_SWITCHTIME 0x4008        // 两套时区表切换时间
#define OI_TWOSETS_DAYTI_SWITCHTIME 0x4009     // 两套日时段切换时间
#define OI_TWOSETS_TITARIFF_SWITCHTIME 0x400A  // 两套分时费率切换时间
#define OI_TWOSETS_STEPPRICE_SWITCHTIME 0x400B // 两套阶梯电价切换事件
#define OI_TZ_TI_NUM 0x400C                    // 时区时段数
#define OI_STEP_NUM 0x400D                     // 阶梯数
#define OI_HARMONIC_ANALYSIS_TIMES 0x400E      // 谐波分析次数
#define OI_SECRET_KEY_ALL_NUM 0x400F           // 秘钥总条数
#define OI_METERING_NUM 0x4010                 // 计量元件数
#define OI_PUBLIC_HOLIDAY_TAB 0x4011           // 公共假日表
#define OI_WEEKEND_FWORD 0x4012                // 周休日特征字
#define OI_WEEKEND_ADOPTED_DAYSCHED 0x4013     // 周休日采用的日时段表
#define OI_PRESET_TZ_TAB 0x4014                // 当前套时区表
#define OI_BACKUPSET_TZ_TAB 0x4015             // 备用套时区表
#define OI_PRESENT_DAYTI_TAB 0x4016            // 当前套日时段表
#define OI_BACKUPSET_DAILY_TI_TAB 0x4017       // 备用套日时段表
#define OI_PRESET_TARIFF_PRICE 0x4018          // 当前套费率电价
#define OI_BACKUPSET_TARIFF_PRICE 0x4019       // 备用套费率电价
#define OI_PRESET_STEP_PRICE 0x401A            // 当前套阶梯电价
#define OI_BACKUPSET_STEP_PRICE 0x401B         // 备用套阶梯电价
#define OI_CURRENT_TRANS_RATIO 0x401C          // 电流互感器变比
#define OI_VOLTAGE_TRANS_RATIO 0x401D          // 电压互感器变比
#define OI_ALARM_AMOUNT_LIMITVALUE 0x401E      // 报警金额限值
#define OI_OTHER_AMOUNT_LIMITVALUE 0x401F      // 其它金额限值
#define OI_ALARM_ELEC_LIMITVALUE 0x4020        // 报警电量限值
#define OI_OTHER_ELEC_LIMITVALUE 0x4021        // 其它电量限值
#define OI_CARD_STATUS_WORD 0x4022             // 插卡状态字
#define OI_TERMINAL_ELIMINATE 0x4024           // 剔除状态字
#define OI_COLL_UPG_RESULT_TAB 0x4025          // 采集器远程升级结果表
#define OI_COLL_UPG_RESULT 0x4026              // 采集器升级结果
#define OI_VOLTAGE_QUALIFIED_RATE 0x4030       // 电压合格率参数
#define OI_MAX_DEMAND_PERIOD 0x4100            // 最大需量周期
#define OI_SLIDER_TIME 0x4101                  // 滑差时间
#define OI_CALI_METER_PULSE_WIDTH 0x4102       // 校表脉冲宽度
#define OI_ASSET_MANAGEMENT_CODE 0x4103        // 资产管理编码
#define OI_RATED_VOLTAGE 0x4104                // 额定电压
#define OI_RATED_CURRENT 0x4105                // 额定电流
#define OI_MAX_CURRENT 0x4106                  // 最大电流
#define OI_ACTV_ACCURACY_GRADE 0x4107          // 有功准确度等级
#define OI_REACTV_ACCURACY_GRADE 0x4108        // 无功准确度等级
#define OI_METER_ACTV_CONSTANT 0x4109          // 电能表有功常数
#define OI_METER_REACTV_CONSTANT 0x410A        // 电能表无功常数
#define OI_METER_MODEL 0x410B                  // 电能表型号
#define OI_ABC_PHAS_CONDCT_COEFF 0x410C        // ABC各相电导系数
#define OI_ABC_PHAS_REACT_COEFF 0x410D         // ABC各相电抗系数
#define OI_ABC_PHAS_RESIST_COEFF 0x410E        // ABC各相电阻系数
#define OI_ABC_PHAS_SUSCEPT_COEFF 0x410F       // ABC各相电纳系数
#define OI_METER_RUNNING_FWORD1 0x4110         // 电能表运行特征状态字1
#define OI_SOFTWARE_RCRD_NUM 0x4111            // 软件备案号
#define OI_ACT_CMB_MODE_FWORD 0x4112           // 有功组合方式特征字
#define OI_REACT_CMB1_FWORD 0x4113             // 无功组合方式1特征字
#define OI_REACT_CMB2_FWORD 0x4114             // 无功组合方式2特征字
#define OI_STLMNT_DATE 0x4116                  // 结算日
#define OI_TI_DEM_FRZ_PERI 0x4117              // 期间需量冻结周期
#define OI_CASCADE_COMM_PARAM 0x4202           // 级联通信参数
#define OI_BROADCAST_TIME 0x4204               // 终端广播校时
#define OI_DEVICE 0x4300                       // 电气设备
#define OI_APP_CONNECTION 0x4400               // 应用连接
#define OI_APP_AUTHENTCTN_PWD 0x4401           // 应用连接认证密码
#define OI_WIRELESS_NETWORK_COMM1 0x4500       // 公网通信模块1
#define OI_WIRELESS_NETWORK_COMM2 0x4501       // 公网通信模块2
#define OI_ETHERNET_COMM1 0x4510               // 以太网通信模块1
#define OI_ETHERNET_COMM2 0x4511               // 以太网通信模块2
//  A.6 冻结类对象标识
#define OI_INSTANTFRZ 0x5000            // 瞬时冻结
#define OI_SECFRZ 0x5001                // 秒冻结
#define OI_MINUTEFRZ 0x5002             // 分钟冻结
#define OI_HOURFRZ 0x5003               // 小时冻结
#define OI_DAY_FRZ 0x5004               // 日冻结
#define OI_STLMNTFRZ 0x5005             // 结算日冻结
#define OI_MONTHFRZ 0x5006              // 月冻结
#define OI_YEARFRZ 0x5007               // 年冻结
#define OI_TZ_TAB_SWITCHFRZ 0x5008      // 时区表切换冻结
#define OI_DAYTI_TAB_SWITCHFRZ 0x5009   // 日时段表切换日冻结
#define OI_TARIFFPRICE_SWITCHFRZ 0x500A // 费率电价切换冻结
#define OI_STEP_SWITCH_FRZ 0x500B       // 阶梯切换冻结
#define OI_STEP_STLMNT_FRZ 0x5011       // 阶梯结算冻结
//  A.7 采集监控类对象标识
#define OI_COLL_DOC_CONFIG_TAB 0x6000    // 采集档案配置表
#define OI_COLLECTDOC_CONFIG_UNIT 0x6001 // 采集档案配置单元
#define OI_SEARCH_TAB 0x6002             // 搜表
#define OI_SEARCH_TAB_RESULT 0x6003      // 一个搜表结果
#define OI_CROSS_PLATFORM_RESULT 0x6004  // 一个跨台区结果
#define OI_TASK_CONFIG_TABLE 0x6012            // 任务配置表
#define OI_TASK_CONFIG_UNIT 0x6013             // 任务配置单元
#define OI_GENERAL_COLL_SCHEME_SET 0x6014      // 普通采集方案集
#define OI_GENERALCOLL_SCHEME 0x6015           // 普通采集方案
#define OI_EVENT_COLL_SCHEME_SET 0x6016        // 事件采集方案集
#define OI_EVENTCOLL_SCHEME 0x6017             // 事件采集方案
#define OI_TRANSPARENT_SCHEME_SET 0x6018       // 透明方案集
#define OI_TRANSPARENT_SCHEME 0x6019           // 透明方案
#define OI_TRANSPARENT_SCHEME_RESULTSET 0x601A // 透明方案结果集
#define OI_TRANSPARENT_SCHEME_RESULT 0x601B    // 一个透明方案结果
#define OI_REPORT_SCHEME_SET 0x601C            // 上报方案集
#define OI_REPORT_SCHEME 0x601D                // 上报方案
#define OI_COLLECT_RULE_LIB 0x601E             // 采集规则库
#define OI_COLLECT_RULE 0x601F                 // 采集规则
#define OI_COLLECT_STATESET 0x6032             // 采集状态集
#define OI_COLLECT_STATE 0x6033                // 采集状态
#define OI_COLLECT_TASK_MONITORSET 0x6034      // 采集任务监控集
#define OI_COLLECT_TASK_MONITOR_UNIT 0x6035    // 采集任务监控单元
#define OI_COLLECT_STARTUP_TIMESTAMP 0x6040    // 采集启动时标
#define OI_COLLECT_SUCCEED_TIMESTAMP 0x6041    // 采集成功时标
#define OI_COLLECT_STORAGE_TIMESTAMP 0x6042    // 采集存储时标
//  A.9 控制类对象标识
#define OI_REMOTE_CONTROL 0x8000               // 遥控
#define OI_ENSURE_POWER_SERVING 0x8001         // 保电
#define OI_DUN_ALARM 0x8002                    // 催费告警
#define OI_GENERAL_CHI_INFO 0x8003             // 一般中文信息
#define OI_IMPORTANT_CHI_INFO 0x8004           // 重要中文信息
#define OI_SAFETY_VALUE 0x8100                 // 终端保安定值
#define OI_CONTROL_PERIOD 0x8101               // 终端功控时段
#define OI_PCONTROL_ALARM_TIME 0x8102          // 功控告警时间
#define OI_TI_PWR_CTRL 0x8103                  // 时段功控
#define OI_FACTORY_REST_CTRL 0x8104            // 厂休控制
#define OI_BUSINESS_STOP_CTRL 0x8105           // 营业报停控制
#define OI_PRE_PWR_FLOAT_DW_CTRL 0x8106        // 当前功率下浮控
#define OI_BUY_ELEC_CTRL 0x8107                // 购电控
#define OI_MONTH_ELEC_CTRL 0x8108              // 月电控
#define OI_TI_PWRCTRL_CONFIG_UNIT 0x8109       // 时段功控配置单元
#define OI_FACTORY_RESTCTRL_CONFIG_UNIT 0x810A // 厂休控配置单元
#define OI_BUSINESSSTOPCTRL_CONFIG_UNIT 0x810B // 营业报停控配置单元
#define OI_BUY_ELECCTRL_CONFIG_UNIT 0x810C     // 购电控配置单元
#define OI_MONTH_ELECCTRL_CONFIG_UNIT 0x810D   // 月电控配置单元
#define OI_CTRL_OBJ 0x810E                     // 控制对象
#define OI_TRIP_ROUND 0x810F                   // 跳闸轮次
#define OI_ELEC_CTRL_FIXED_VALUE 0x8110        // 电控定值
//  A.10　文件传输类对象
#define OI_FILE_BLOCK_TRANS_MANAGE 0xF001 //文件分块传输管理
//  A.11 ESAM接口类对象
#define OI_ESAM 0xF100                // ESAM
#define OI_ESAM_SAFE_MODE_PARA 0xF101 // 安全模式参数
//  A.12 输入输出设备类对象
#define OI_RS232 0xF200    // RS232
#define OI_RS485 0xF201    // RS485
#define OI_INFRARED 0xF202 // 红外
#define OI_SWITCHINPUT 0xF203
#define OI_DC_ANALOG 0xF204         // 直流模拟量
#define OI_RELAY_OUTPUT 0xF205      // 继电器输出
#define OI_ALARM_OUTPUT 0xF206      // 告警输出
#define OI_MULTI_FUNC_TERM 0xF207   // 多功能端子
#define OI_AC_SAMPLING 0xF208       // 交采接口
#define OI_PLC 0xF209               // 载波/微功率无线接口
#define OI_PINPUTDEVICE 0xF20A      // 脉冲输入设备
#define OI_BLUETOOTH 0xF20B         // 蓝牙
#define OI_SLAVENODE_ELEMENT 0xF210 // 从节点单元
//  A.13 显示类对象
#define OI_AUTO_ROUND_DISPLAY 0xF300    // 自动轮显
#define OI_KEYDOWN_ROUND_DISPLAY 0xF301 // 按键轮显
#endif
