#include "backstagemanager.h"
#include <signal.h>
#include <QDebug>
BackstageManager::BackstageManager(QObject *parent):
    QObject(parent)
{
   // start();
}

void BackstageManager::start()
{
    client_syspam my_syspam;
    serial_2 = new serial(this);
    tcp_client = new client(this);
    QString name = "/dev/ttymxc1";
    serial_2->openPort(name,BAUD9600,DATA_8,PAR_NONE,STOP_1,FLOW_OFF,TIME_OUT);
    connect(tcp_client,SIGNAL(veh_data_re()),this,SLOT(ReadVehicleLocation()));
    connect(serial_2,SIGNAL(hasdata(QString,qint32,qint32)),tcp_client,SLOT(ConnectToHost(QString,qint32,qint32)));
    my_syspam.ip = tcp_client->ConfigFIleGet("client_syspam","ip").toString();
    my_syspam.port = tcp_client->ConfigFIleGet("client_syspam","port").toInt();
    my_syspam.device_id = tcp_client->ConfigFIleGet("client_syspam","device_id").toInt();
    emit serial_2->hasdata(my_syspam.ip,my_syspam.port,my_syspam.device_id);
}

void BackstageManager::ReadVehicleLocation()
{
    vehicle_localtion Node;
    QList<qint8> list;
    while(!(Node = tcp_client->ReadSingle_vehicle()).vehicle_name.isEmpty()){
        qDebug()<<Node.vehicle_name;
        qDebug()<<Node.count;
         qDebug()<<QString::number(Node.vehicle_amount);
         for(int i = 0;i < Node.vehicle_amount;i++){
            //qDebug()<<QString::number(Node.station_index[i]);
             list.append(Node.station_index[i]);
              qDebug()<<QString::number(list.at(i));
         }
    }
    qDebug()<<QString::number(list.at(2));
    qDebug()<<QString::number(list.at(3));
}
