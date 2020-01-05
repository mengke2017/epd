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
    QList<qint8> list_;
    QList<LineStatus> status_list;
    static int size = 0;
 //   status_list = new LineStatus();

    while(!(Node = tcp_client->ReadSingle_vehicle()).vehicle_name.isEmpty()){

        status_list.reserve(++size);
        status_list[size-1].stat_id = Node.vehicle_name;
        status_list[size-1].over_count = Node.count;
        qDebug()<<Node.vehicle_name<<" "<<status_list.at(size-1).stat_id;
        qDebug()<<Node.count<<" "<<status_list.at(size-1).over_count;
        qDebug()<<QString::number(Node.vehicle_amount);
        for(int i = 0;i < Node.vehicle_amount;i++){
            //qDebug()<<QString::number(Node.station_index[i]);
            list_.append(Node.station_index[i]);
            status_list[size-1].che_pos.append(Node.station_index[i]);
            qDebug()<<QString::number(list_.at(i))<<" "<<QString::number(status_list.at(size-1).che_pos.at(i));
        }
    }
    size = 0;
    emit update_status(status_list);
//    qDebug()<<QString::number(list_.at(2));
//    qDebug()<<QString::number(list_.at(3));
}
