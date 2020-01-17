#include "backstagemanager.h"
#include "tcp/StationCommand.h"
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
    tcp_client = client::getInstance();
    timer = new QTimer();

    connect(serial_2,SIGNAL(hasdata(QString,qint32,qint32)),tcp_client,SLOT(ConnectToHost(QString,qint32,qint32)));
    connect(tcp_client,SIGNAL(veh_data_re()),this,SLOT(ReadVehicleLocation()));
    connect(timer, SIGNAL(timeout()), this, SLOT(WeatherRequest()));
    timer->start(5000);

    client_syspam my_syspam;

    QString name = SERIAL_PATH;
    serial_2->openPort(name,BAUD9600,DATA_8,PAR_NONE,STOP_1,FLOW_OFF,TIME_OUT);

    my_syspam.ip = tcp_client->ConfigFIleGet("client_syspam","ip").toString();
    my_syspam.port = tcp_client->ConfigFIleGet("client_syspam","port").toInt();
    my_syspam.device_id = tcp_client->ConfigFIleGet("client_syspam","device_id").toInt();
    http_client =  new http(QString::number(my_syspam.device_id));
    connect(tcp_client,SIGNAL(http_command(int)),http_client,SLOT(HttpPostRequest(int)));
    connect(http_client,SIGNAL(to_local(int)),this,SLOT(ui_handle(int)));
    emit serial_2->hasdata(my_syspam.ip,my_syspam.port,my_syspam.device_id);  // 连接服务器
}

void BackstageManager::ReadVehicleLocation()
{
  //  qWarning()<<"111";
    vehicle_localtion Node;
    QList<qint8> list_;
    while(!(Node = tcp_client->ReadSingle_vehicle()).vehicle_name.isEmpty()) {
      //  qWarning()<<"ReadVehicleLocation";
        for(int i = 0;i < Node.vehicle_amount;i++) {
            list_.append(Node.station_index[i]);
        }
        emit update_status(Node.vehicle_name,Node.count,list_);
    }
//    static int flag = 0;
//    if(flag == 0) {
//        list_.append(2); list_.append(5); list_.append(10);
//        emit update_status("1路南","3",list_);
//        list_.clear();
//        list_.append(2); list_.append(3); list_.append(7); list_.append(9);
//        emit update_status("6","1",list_);
//        list_.clear();
//        list_.append(4); list_.append(7);
//        emit update_status("19","5",list_);
//        list_.clear();
//        list_.append(2); list_.append(8); list_.append(13);
//        emit update_status("501南","2",list_);
//        list_.clear();
//        flag = 1;
//    } else {
//        list_.append(3); list_.append(7); list_.append(13);
//        emit update_status("1路南","2",list_);
//        list_.clear();
//        list_.append(2); list_.append(4); list_.append(6); list_.append(11);
//        emit update_status("6","3",list_);
//        list_.clear();
//        list_.append(7); list_.append(10);
//        emit update_status("19","6",list_);
//        list_.clear();
//        list_.append(1); list_.append(9); list_.append(10);
//        emit update_status("501南","3",list_);
//        list_.clear();
//        flag = 0;
//    }
}

void BackstageManager::WeatherRequest(void)
{
    emit tcp_client->http_command(WEATHER_HTTP);
 //   sleep(1);
 //   emit tcp_client->http_command(UPDATE_LINE_HTTP);
 //   timer->stop();
//    sleep(1);
//    emit tcp_client->http_command(GET_INI_HTTP);
}

void BackstageManager::ui_handle(int uiflag)
{
    qWarning("ui_handle");
    switch(uiflag)
    {
    case WEATHER_HTTP:
        emit read_weather();
        break;
    case UPDATE_LINE_HTTP:
        emit read_line();
        break;
    case GET_INI_HTTP:
        emit read_initpara();
        break;
    }
}
