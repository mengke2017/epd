#ifndef CHILDLINE_H
#define CHILDLINE_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QTextEdit>
#include <QDebug>
#include "Module/stationname.h"
#include "customize.h"

#define FIRST_STATION  140
#define LAST_STATION   1100
#define VAILD_DIST     (LAST_STATION - FIRST_STATION)

class ChildLine:public QFrame
{
    Q_OBJECT

public:
    ChildLine(int16_t ypos);
    ~ChildLine();

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
    void updateTime(QString);
    void update_status(QList<qint8>);
protected:
    void line_list_clear();
    void che_ico_list_clear();
};
#endif  // CHILDLINE_H
