#ifndef MAINLINE_H
#define MAINLINE_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QTextEdit>
#include <QDebug>
#include "customize.h"

class MainLine:public QFrame
{
    Q_OBJECT

public:
    MainLine(int32_t ypos);
    ~MainLine();

    QWidget *line_widget;
    QLabel *line_id;
    QLabel *line_info;
    QLabel *over_stat;
    QLabel *beg_end_time_label;
    QLabel *beg_end_time_value;
    QLabel *price_label;
    QLabel *price_value;

public:
    void showall();
    void updateLine_id(QString);
    void updateLine_info(QString info);
    void updateOver_stat(QString count);
    void updateBeg_end_time(QString Begtime, QString Endtime);
    void updatePrice_value(QString price);
    void creat_line(QString id, QString info, QString Begtime,
                     QString Endtime, QString price);
    void update_status(QString);
};

#endif  // MAINLINE_H
