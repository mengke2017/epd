#include "backstagemanager.h"
#include <signal.h>
#include <QDebug>

#define SERIAL_PATH  "/dev/ttyS1"

BackstageManager::BackstageManager(QObject *parent):
    QObject(parent)
{
    start();
}

void BackstageManager::start()
{
    serial_2 = new serial();
    tcp_client = new client();
    connect(serial_2,SIGNAL(hasdata(QString,qint32,qint32)),tcp_client,SLOT(ConnectToHost(QString,qint32,qint32)));
    connect(tcp_client,SIGNAL(veh_data_re()),this,SLOT(ReadVehicleLocation()),Qt::QueuedConnection);
    client_syspam my_syspam;

    QString name = SERIAL_PATH;
    serial_2->openPort(name,BAUD9600,DATA_8,PAR_NONE,STOP_1,FLOW_OFF,TIME_OUT);

    my_syspam.ip = tcp_client->ConfigFIleGet("client_syspam","ip").toString();
    my_syspam.port = tcp_client->ConfigFIleGet("client_syspam","port").toInt();
    my_syspam.device_id = tcp_client->ConfigFIleGet("client_syspam","device_id").toInt();
    emit serial_2->hasdata(my_syspam.ip,my_syspam.port,my_syspam.device_id);  // 连接服务器
}

void BackstageManager::ReadVehicleLocation()
{
    vehicle_localtion Node;
    QList<qint8> list_;

    while(!(Node = tcp_client->ReadSingle_vehicle()).vehicle_name.isEmpty()) {

        for(int i = 0;i < Node.vehicle_amount;i++) {
            list_.append(Node.station_index[i]);
        }
        emit update_status(Node.vehicle_name,Node.count,list_);
    }
}
