#ifndef CUSTOMIZE_H
#define CUSTOMIZE_H
#include <QDebug>

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

typedef struct
{
    QString stat_id;
    QString over_count;
    QList<qint8> che_pos;
} LineStatus;  // update

typedef struct {
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
} PageInfo;  //creat

typedef struct {
    QString type;
    QString bgdate;
    QString enddate;
    QString bgtime;
    QString endtime;
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
