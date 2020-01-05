#ifndef CHILDLINE_H
#define CHILDLINE_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QTextEdit>
#include <QDebug>
#include "Module/stationname.h"
#include "customize.h"

#define FIRST_STATION  165
#define LAST_STATION   1065
#define VAILD_DIST     (LAST_STATION - FIRST_STATION)

class ChildLine:public QFrame
{
    Q_OBJECT

public:
    ChildLine(int16_t ypos);

    QWidget *child_line_widget;
    QLabel *line_id;
    QLabel *info;
    QList<QLabel*>che_ico_list;
    QList<StationNmae *> line_list;
    void update_line_id(QString);
    void create_line(QString id, int8_t current_index, int8_t station_total,
                     QList<QString> stat_name_list,
                     QString timeS, QString timeE, QString price);
    void update_info(QString timeS, QString timeE, QString price);
    void create_cheico(QList<int> che_index);
    void updateTime(QString);
    void line_list_clear();
    void che_ico_list_clear();
    void update_status(LineStatus status);
};
#endif  // CHILDLINE_H
