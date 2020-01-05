#ifndef CUSTOMIZE_H
#define CUSTOMIZE_H
#include <QDebug>

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
//typedef unsigned long  uint64_t;

typedef struct
{
    QString stat_id;
    QString over_count;
    QList<int> che_pos;
} LineStatus;  // update

typedef struct {
    QString stat_id;
    QString endstat_name;
    QString Begtime;
    QString Endtime;
    QString price;
    QString over_count;
    QList<QString> name_list;
    QList<int> che_pos;
    uint8_t current_index;
    uint8_t station_total;
} PageInfo;  //creat



#endif // CUSTOMIZE_H
