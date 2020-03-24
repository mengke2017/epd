#ifndef BACKSTAGEMANAGER_H
#define BACKSTAGEMANAGER_H
#include "tcp/client.h"
#include "serial/serial.h"
#include <QObject>
#include <QMetaType>
#include <QTimer>
#include "customize.h"
#include "tcp/http.h"
#include "battery/batterymanager.h"

class BackstageManager : public QObject
{
    Q_OBJECT

public:
    explicit BackstageManager(QObject *parent = 0);
private:
    client           *tcp_client;
    serial           *serial_2;
    http             *http_client;
    QTimer           *timer;
    void WeatherRequest();
    BatteryManger   *battery;
signals:
    void update_status(QString,QString,QList<qint8>);
    void read_weather();
    void read_line();
    void read_initpara();
    void update_bulletin(QString);
    void ui_start(bool);
public slots:
    void ReadVehicleLocation(void);
    void start(void);
    void serTimerOut(uint current_sec);
    void ui_handle(int);
//    void ui_bulletin(QList<Msg>);
};

#endif // BACKSTAGEMANAGER_H
