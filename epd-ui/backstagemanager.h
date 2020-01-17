#ifndef BACKSTAGEMANAGER_H
#define BACKSTAGEMANAGER_H
#include "tcp/client.h"
#include "serial/serial.h"
#include <QObject>
#include <QMetaType>
#include <QTimer>
#include "customize.h"
#include "tcp/http.h"
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
signals:
    void update_status(QString,QString,QList<qint8>);
    void read_weather();
    void read_line();
    void read_initpara();
public slots:
    void ReadVehicleLocation(void);
    void start(void);
    void WeatherRequest();
    void ui_handle(int);
};

#endif // BACKSTAGEMANAGER_H
