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
    serial_2 = new serial();
    tcp_client = new client();
    connect(serial_2,SIGNAL(hasdata(QString,qint32,qint32)),tcp_client,SLOT(ConnectToHost(QString,qint32,qint32)));
    connect(tcp_client,SIGNAL(veh_data_re()),this,SLOT(ReadVehicleLocation()),Qt::QueuedConnection);
    client_syspam my_syspam;

    QString name = "/dev/ttymxc1";
    serial_2->openPort(name,BAUD9600,DATA_8,PAR_NONE,STOP_1,FLOW_OFF,TIME_OUT);

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
    //LineStatus * pstatus;
    static int size = 0;
 //   status_list = new LineStatus();
    size = 0;
    while(!(Node = tcp_client->ReadSingle_vehicle()).vehicle_name.isEmpty()) {
       // pstatus = (LineStatus *)malloc(sizeof(LineStatus));

//        status_list.reserve(10);
//        status_list[0].stat_id = Node.vehicle_name;
//        status_list[0].over_count = Node.count;
       // pstatus->stat_id = Node.vehicle_name;
        //pstatus->over_count = Node.count;

//        qDebug()<<Node.vehicle_name<<" "<<pstatus->stat_id;
//        qDebug()<<Node.count<<" "<<pstatus->over_count;
//        qDebug()<<QString::number(Node.vehicle_amount);
        for(int i = 0;i < Node.vehicle_amount;i++){
            list_.append(Node.station_index[i]);
           // pstatus->che_pos.append(Node.station_index[i]);
            //qDebug()<<QString::number(list_.at(i))<<" "<<QString::number(pstatus->che_pos.at(i));
        }
      //  status_list.append(*pstatus);
        emit update_status(Node.vehicle_name,Node.count,list_);
    }

//    qDebug()<<QString::number(list_.at(2));
//    qDebug()<<QString::number(list_.at(3));
}
