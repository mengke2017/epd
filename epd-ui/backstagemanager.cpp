#include "backstagemanager.h"
#include "tcp/StationCommand.h"
#include <signal.h>
#include <QDebug>
#include <QDateTime>

#define SERIAL_PATH  "/dev/ttyS3"

BackstageManager::BackstageManager(QObject *parent):
    QObject(parent)
{
   // qRegisterMetaType<Msg>("Msg");
    start();
}

void BackstageManager::start()
{
    serial_2 = new QSerialPort(this);
    tcp_client = client::getInstance();
    battery = BatteryManger::getInstance();
//    connect(serial_2,SIGNAL(hasdata()),this,SLOT(comSlot()));
    connect(this,SIGNAL(to_SetIP(QString,uint32,uint32)),tcp_client,SLOT(ConnectToHost(QString,uint32,uint32)));
    connect(tcp_client,SIGNAL(veh_data_re()),this,SLOT(ReadVehicleLocation()));
    connect(tcp_client,SIGNAL(client_shot()),this,SLOT(soltShotScreen()));

    client_syspam my_syspam;

   // QString name = SERIAL_PATH;

    serial_2 = new QSerialPort(this);
    serial_2->setPortName(SERIAL_PATH);
    if(serial_2->open(QIODevice::ReadWrite))
    {
        //设置波特率
        serial_2->setBaudRate(115200);
        //设置数据位
        serial_2->setDataBits(QSerialPort::Data8);
        //设置校验位
        serial_2->setParity(QSerialPort::NoParity);
        //设置流控制
        serial_2->setFlowControl(QSerialPort::NoFlowControl);
        //设置停止位
        serial_2->setStopBits(QSerialPort::OneStop);
        connect(serial_2,SIGNAL(readyRead()),this,SLOT(comSlot()), Qt::QueuedConnection);
    } else {
        qWarning()<<"open serial_2 faile!";
    }

//    serial_2->openPort(name,BAUD9600,DATA_8,PAR_NONE,STOP_1,FLOW_OFF,TIME_OUT);

    my_syspam.ip = tcp_client->ConfigFIleGet("client_syspam","ip").toString();
    my_syspam.port = tcp_client->ConfigFIleGet("client_syspam","port").toInt();
    my_syspam.device_id = tcp_client->ConfigFIleGet("client_syspam","device_id").toInt();
    http_client =  new http(QString::number(my_syspam.device_id));
    connect(tcp_client,SIGNAL(http_command(int)),http_client,SLOT(HttpPostRequest(int)));
    connect(battery,SIGNAL(to_alarm(QString)),this,SLOT(solt_http_alarm(QString)));
    connect(http_client,SIGNAL(to_local(int)),this,SLOT(ui_handle(int)));
    connect(http_client,SIGNAL(to_battery_time(QString)),battery,SLOT(SetBatteryTime(QString)));
//    connect(tcp_client,SIGNAL(to_ui_bulletin(QList<Msg>)),this,SLOT(ui_bulletin(QList<Msg>)));
//    emit serial_2->hasdata(my_syspam.ip,my_syspam.port,my_syspam.device_id);  // 连接服务器
    tcp_client->ConnectToHost(my_syspam.ip, my_syspam.port, my_syspam.device_id);
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
        WeatherRequest();
        count = 0;
    }
    for(int i = 0; i < list.length(); i ++) {
        if(list.at(i).star_sec <= current_sec && list.at(i).end_sec >= current_sec) {
            emit update_bulletin(list.at(i).value);
        }
    }
   // if(tcp_client->screen_shot)
}

void BackstageManager::WeatherRequest(void)
{
    if(tcp_client->isConnected()) {
        emit tcp_client->http_command(GET_WEATHER_HTTP);
        emit tcp_client->http_command(GET_SERVICE_TIME);
    //    emit tcp_client->http_command(PUT_ERROR_MSG);
    }
}

void BackstageManager::ui_handle(int uiflag)
{
//    qWarning("uiflag: %d", uiflag);
    switch(uiflag)
    {
    case GET_WEATHER_HTTP:
        emit read_weather();
        break;
    case GET_LINE_HTTP:
        emit read_line();
        break;
    case GET_INI_HTTP:
        emit read_initpara();
        break;
    case GET_LINE_STYLE:
        emit read_line_style();
        break;
    case GET_VERSION:
        emit tcp_client->http_command(GET_SOFT_FILE);
        break;
    case GET_SOFT_FILE:
        system("reboot");
        //    qWarning("reboot");
        break;
    }
}

void BackstageManager::comSlot() {
    QString ip_tcp;
    uint32 port_tcp = 0;
    uint32 dev_id_tcp = 0;
    int16 star_index, end_index;
    QString buf_string;
    static QByteArray data;
    QString str_data;
    data += serial_2->readAll();
   // qDebug() << "@@data@@:" <<data.toHex();

    star_index = data.indexOf('$');
    if(star_index < 0) {
        data.remove(0,data.size());
        goto error;
    }
    end_index = data.indexOf('\n');
    if(end_index <= 0) {
        return;
    } else {
        if (data.at(end_index-1) != '\r') {
            return;
        }
    }
    data = data.mid(star_index+1, end_index-star_index);

    do{
        star_index = data.indexOf('$');
        data = data.mid(star_index+1);
    }while(data.indexOf('$')>=0);

    str_data = data;
    qWarning()<<"str_data: "<<str_data;
    if(!str_data.isEmpty()){
        QStringList TCP_pam = str_data.split(",");
        if(TCP_pam.size()==3){
            buf_string = TCP_pam.at(0);
            if(buf_string.size() > 3)
                if(buf_string.left(3) == "ip:")
                    ip_tcp = buf_string.mid(3);

            buf_string = TCP_pam.at(1);
            if(buf_string.size() > 5)
                if(buf_string.left(5) == "port:")
                    port_tcp = (buf_string.mid(5)).toInt();

            buf_string = TCP_pam.at(2);
            if(buf_string.size() > 2)
                if(buf_string.left(3) == "id:")
                    dev_id_tcp = (buf_string.mid(3)).toInt();
            if(ip_tcp.isEmpty())
                goto error;
            emit to_SetIP(ip_tcp,port_tcp,dev_id_tcp);
            QString tmp = "OK\n$ip:"+ip_tcp+",port:"+QString::number(port_tcp)
                    +",id:"+QString::number(dev_id_tcp);
            tmp.append(0x0d).append(0x0a);
            serial_2->write(tmp.toLatin1());
            data.remove(0,data.size());
            //qWarning()<<"data: "<<data.toHex();
            return;
        }
    }
error:
    serial_2->write("config error!");
    data.remove(0,data.size());
    //qWarning()<<"data: "<<data.toHex();
}

void BackstageManager::to_http(uint8 command) {

    tcp_client->http_command(command);
}

void BackstageManager::soltShotScreen() {
    emit shot_screen();
}

void BackstageManager::slot_set_led(uint8 flag){
    if(flag) {
        system(BACK_LED1_ON);
        if(flag > 1)
            system(BACK_LED2_ON);
    } else {
        system(BACK_LED1_OFF);
        system(BACK_LED2_OFF);
    }
}
void BackstageManager::solt_http_alarm(QString alarm) {
    qWarning("solt_http_alarm");
    http_client->alarm_list.append(alarm);
    emit tcp_client->http_command(PUT_ERROR_MSG);
}
