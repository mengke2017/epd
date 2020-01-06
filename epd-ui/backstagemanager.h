#ifndef BACKSTAGEMANAGER_H
#define BACKSTAGEMANAGER_H
#include "tcp/client.h"
#include "serial/serial.h"
#include <QObject>
#include <QMetaType>
#include "customize.h"
class BackstageManager : public QObject
{
    Q_OBJECT

public:
    explicit BackstageManager(QObject *parent = 0);
private:
    client           *tcp_client;
    serial           *serial_2;
signals:
    void update_status(QString,QString,QList<qint8>);
public slots:
    void ReadVehicleLocation(void);
    void start(void);
};

#endif // BACKSTAGEMANAGER_H
