#ifndef CUSTOMIZE_H
#define CUSTOMIZE_H
#include <QDebug>

#define ARM_32BIT    1

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
//typedef unsigned long   uint64_t;
typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long   uint64;
typedef  char   int8;
typedef  short  int16;
typedef  int    int32;
typedef  long   int64;
//typedef unsigned long  uint64_t;
#define DIAL_FLAG_FILE  "/usb0"
#define CONFIG_FILE  "./SocketSyspam.ini"

/**********太阳能电池参数**********/
//DEVICE ID
#define DEV_ID                    0x01
//功能码
#define FUNC_WRITE                0x10
#define FUNC_ONLY_READ_2          0X02
#define FUNC_READ                 0X03
#define FUNC_ONLY_READ            0X04
//变量地址
/****************A1-A38*****************************************************
*****************实时参数：整个系统在正常工作时各参数的实时数据和实时状态，同时也包括*******
*****************发电量和用电量的历史统计。**************************************/
#define A1_DEVICE_OVERHEAT             0x2000
#define A2_DAY_OR_NIGHT                0x200C
#define A3_ARRAY_VOLTAGE               0x3100
#define A4_ARRAY_CURRENT               0x3101
#define A5_ARRAY_POWER_L               0x3102
#define A6_ARRAY_POWER_H               0x3103
#define A7_VOLTAGE                     0x310C
#define A8_CURRENT                     0x310D
#define A9_POWER_L                     0x310E
#define A10_POWER_H                    0x310F
#define A11_BATTERY_TEMPERATURE        0x3110
#define A12_TEMPERATURE                0x3111
#define A13_BATTERY_POWER              0x311A
#define A14_BATTERY_VOLTAG_ELEVEL      0x311D
#define A15_BATTERY_STATUS             0x3200
#define A16_CHARGE_STATUS              0x3201
#define A17_DISCHARGE_STATUS           0x3202
#define A18_MAX_VOLTAGE                0x3302
#define A19_MIN_VOLTAGE                0x3303
#define A20_DAY_POWER_DISCHARGE_L      0x3304
#define A21_DAY_POWER_DISCHARGE_H      0x3305
#define A22_MONTH_POWER_DISCHARGE_L    0x3306
#define A23_MONTH_POWER_DISCHARGE_H    0x3307
#define A24_YEAR_POWER_DISCHARGE_L     0x3308
#define A25_YEAR_POWER_DISCHARGE_H     0x3309
#define A26_TOTAL_POWER_DISCHARGE_L    0x330A
#define A27_TOTAL_POWER_DISCHARGE_H    0x330B
#define A28_DAY_POWER_CHARGE_L         0x330C
#define A29_DAY_POWER_CHARGE_H         0x330D
#define A30_MONTH_POWER_CHARGE_L       0x330E
#define A31_MONTH_POWER_CHARGE_H       0x330F
#define A32_YEAR_POWER_CHARGE_L        0x3310
#define A33_YEAR_POWER_CHARGE_H        0x3311
#define A34_TOTAL_POWER_CHARGE_L       0x3312
#define A35_TOTAL_POWER_CHARGE_H       0x3313
#define A36_BATTERY_VOLTAGE            0x331A
#define A37_BATTERY_CURRENT_L          0x331B
#define A38_BATTERY_CURRENT_H          0x331C

/****************B1-B25*****************************************************
*****************蓄电池参数：针对所选用的蓄电池类型设置相应的参数，主要是对一些特殊电****
*****************压点进行合理的设置********************************************/
#define B1_BATTERY_RATED_CURRENT            0x3005
#define B2_LOAD_RATED_CURRENT               0x300E
#define B3_BATTERY_VOLTAG_ELEVEL            0x311D
#define B4_BATTERY_TYPE                     0x9000
#define B5_BATTERY_CAPACITY                 0x9001
#define B6_TEMPERATURE_COMPENCITION         0x9002
#define B7_OVER_VOLTAG                      0x9003
#define B8_CHARGE_LIMIT_VOLTAG              0x9004
#define B9_OVER_VOLTAG_RECOVER              0x9005
#define B10_BALANCE_CHARGE_VOLTAG           0x9006
#define B11_HOIST_CHARGE_VOLTAG             0x9007
#define B12_FLOAT_CHARGE_VOLTAG             0x9008
#define B13_HOIST_VOLTAG_RECOVER            0x9009
#define B14_LOW_BREAK_VOLTAG_RECOVER        0x900A
#define B15_UNDER_VOLTAG_WARNING_RECOVER    0x900B
#define B16_UNDER_VOLTAG_WARNING            0x900C
#define B17_LOW_VOLTAG_BREAK                0x900D
#define B18_DISCHARGE_LIMLIT                0x900E
#define B19_SYSTEM_VOLTAG_LEVEL             0x9067
#define B20_HANDLE_SWITCH                   0x906A
#define B21_BALANCE_TIME                    0x906B
#define B22_HOIST_TIME                      0x906C
#define B23_DISCHARGE_DEEP                  0x906D
#define B24_CHARGE_DEEP                     0x906E
#define B25_CHARGE_MANAGER                  0x9070
/**************************************************/
/***********HTTP CMD*************/
#define GET_WEATHER_HTTP  0
#define WEATHER_HTTP      0
#define GET_LINE_HTTP     1    //更新线路
#define UPDATE_LINE_HTTP  1
#define GET_INI_HTTP      2   //主动更新初始化参数
#define GET_SERVICE_TIME  3
#define GET_LINE_STYLE    4
#define GET_SOFT_FILE     5
#define PUT_SHOT_SCREEN   6
#define PUT_ERROR_MSG     7
#define GET_VERSION       8

#define PARAMS_NUM        9
/*********************************/

typedef struct
{
    QString stat_id;
    QString over_count;
    QList<qint8> che_pos;
} LineStatus;  // update

typedef struct {
    QString line_id;
    QString stat_id;
    QString endstat_name;
    QString Begtime;
    QString Endtime;
    QString timeSum;
    QString timeWin;
    QString price;
    QList<QString> name_list;
    uint8_t current_index;
    uint8_t station_total;
    bool    showFlag;
} PageInfo;  //creat

typedef struct {
    QString line_id;
    QString type;
    QString seq;
} StyelInfo;

typedef struct {
    QString type;
//    QString bgdate;
//    QString enddate;
//    QString bgtime;
//    QString endtime;
    uint star_sec;
    uint end_sec;
    QString value;
} Msg;

typedef struct {
    QString station_name;
    QString bg_time;
    QString end_time;
    QString open;
    QString shut;
    QString brightness;
    uint16_t black_value;
    uint16_t black_count;
} InitPara;

enum weather_t {
    BAOXUE = 0, BAOYU, DABAOXUE, DABAOYU, DAXUE,
    DAYU, DUOYUN, LEIZHENYU, MAI, QING, SHACHENBAO,
    TAIFENG, TEDABAOYU, WU, XIAOXUE,XIAOYU,YIN,
    YUJIAXUE, ZHENYU, ZHONGXUE, ZHONGYU
};

#endif // CUSTOMIZE_H
