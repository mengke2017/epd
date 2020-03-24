#include "backstagemanager.h"
#include "tcp/StationCommand.h"
#include <signal.h>
#include <QDebug>
#include <QDateTime>

#define SERIAL_PATH  "/dev/ttyS2"

BackstageManager::BackstageManager(QObject *parent):
    QObject(parent)
{
   // qRegisterMetaType<Msg>("Msg");
    start();
}

void BackstageManager::start()
{
    serial_2 = new serial();
    tcp_client = client::getInstance();
//    timer = new QTimer();

    connect(serial_2,SIGNAL(hasdata(QString,uint32,uint32)),tcp_client,SLOT(ConnectToHost(QString,uint32,uint32)));
    connect(tcp_client,SIGNAL(veh_data_re()),this,SLOT(ReadVehicleLocation()));
//    connect(timer, SIGNAL(timeout()), this, SLOT(timerOut()));
//    timer->start(10000);

    client_syspam my_syspam;

    QString name = SERIAL_PATH;
    serial_2->openPort(name,BAUD9600,DATA_8,PAR_NONE,STOP_1,FLOW_OFF,TIME_OUT);

    my_syspam.ip = tcp_client->ConfigFIleGet("client_syspam","ip").toString();
    my_syspam.port = tcp_client->ConfigFIleGet("client_syspam","port").toInt();
    my_syspam.device_id = tcp_client->ConfigFIleGet("client_syspam","device_id").toInt();
    http_client =  new http(QString::number(my_syspam.device_id));
    connect(tcp_client,SIGNAL(http_command(int)),http_client,SLOT(HttpPostRequest(int)));
    connect(http_client,SIGNAL(to_local(int)),this,SLOT(ui_handle(int)));
//    connect(tcp_client,SIGNAL(to_ui_bulletin(QList<Msg>)),this,SLOT(ui_bulletin(QList<Msg>)));
    emit serial_2->hasdata(my_syspam.ip,my_syspam.port,my_syspam.device_id);  // 连接服务器
}

void BackstageManager::ReadVehicleLocation()
{
  //  qWarning()<<"111";
    vehicle_localtion Node;
    QList<qint8> list_;
    while(!(Node = tcp_client->ReadSingle_vehicle()).vehicle_name.isEmpty()) {
        for(int i = 0;i < Node.vehicle_amount;i++) {
            list_.append(Node.station_index[i]);
        }
        emit update_status(Node.vehicle_name,Node.count,list_);
    }
}

void BackstageManager::serTimerOut(uint current_sec)
{
    QList<Msg> list = tcp_client->msg_list;
    static int16_t count = -1;
    count++;
    if (count >= 360 || count == 0) { // 10*36 hour
//        qWarning("11111");
        if(count == 0) {  // first
            emit tcp_client->http_command(GET_INI_HTTP);
            emit tcp_client->http_command(UPDATE_LINE_HTTP);
            WeatherRequest();
        }
        WeatherRequest();
        count = 0;
    }

    for(int i = 0; i < list.length(); i ++) {
        if(list.at(i).star_sec <= current_sec && list.at(i).end_sec >= current_sec) {
            emit update_bulletin(list.at(i).value);
        }
    }
}
void BackstageManager::WeatherRequest(void)
{
//    qWarning()<<"2222";
//    BatteryPara batteryPara;
//    battery->ReadBatteryPara(&batteryPara);
    emit tcp_client->http_command(WEATHER_HTTP);
//    qWarning()<<"22222";
//    sleep(1);

//    sleep(1);
    emit tcp_client->http_command(GET_SERVICE_TIME);
//    qWarning()<<"111111";
//    timer->stop();
}

void BackstageManager::ui_handle(int uiflag)
{
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
