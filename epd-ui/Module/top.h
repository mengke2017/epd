#ifndef TOP_H
#define TOP_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QTextEdit>
#include <QDebug>
#include <QList>
#include "bulletin.h"

class TopWidget:public QFrame
{
    Q_OBJECT

public:
    TopWidget();
    ~TopWidget();

    QWidget *top_widget;
    QLabel *stat_name;
    QLabel *weath_ico;
    QLabel *weath_text;
    QLabel *date_text;
    QLabel *time_label;

    QWidget *title_widget;
    QLabel *lineId_name;
    QLabel *line_info_name;
    QLabel *over_stat_name;


    QList<QString> weath_list_ico;
    QList<QString> weath_list_text;

    QString current_date;
    QString current_time;
public:
    void showall();
    void updateStat_name(QString);
    void updateWeather(QString  weather, QString temp);
    void updateDate(QString, QString);
    void updateTime(QString);
signals:
    void timeover(uint);
//private:
//    void initwidget(QWidget *widget, QString name, bool enabled, QRect rect, QFont font, QString stylesheet);
//    void initlabel(QLabel *label,  QString name, bool enabled, QRect rect, QFont font,
//    QString stylesheet, Qt::AlignmentFlag flag, QString text );

private slots:
    void TimeUpdate();
};

#endif  // _TOP_H_
