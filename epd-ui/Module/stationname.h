#ifndef STATIONNAME_H
#define STATIONNAME_H
#include <QFrame>
#include <QWidget>
#include <QLabel>
class StationNmae:public QFrame
{
    Q_OBJECT

public:
    StationNmae(int16_t xpos, int16_t ypos);
    ~StationNmae();
    QWidget *stat_name_widget;
    QWidget *name_widget;
    QLabel *drop_label;
    QLabel *name_label;
    QLabel *name_label2;
    void SetText(QString);
    void setSelect();
};

#endif // STATIONNAME_H
