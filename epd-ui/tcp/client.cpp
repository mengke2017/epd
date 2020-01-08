#include "client.h"
#include <QDebug>
#include <time.h>
#include <QString>
#include <QTextCodec>

#define CONFIG_FILE  "/SocketSyspam.ini"

client::client(QObject *parent) :
    QObject(parent)
{
    timer = new QTimer();
    socket = new QTcpSocket(this);
    my_filename = CONFIG_FILE;
    connect(socket,SIGNAL(connected()),this,SLOT(ConnectSuccess()));
    connect(socket,SIGNAL(readyRead()),this,SLOT(ReadMsg()));
    Serial = 0;
    //my_syspam.device_id = ConfigFIleGet("client_syspam","device_id").toInt();

    system(BACK_LED1_CFG);
    system(BACK_LED2_CFG);
    get_version();
}

void client::ReadMsg(void)
{
    QByteArray msg = socket->readAll();
    TCPsocket_Protocol(msg);
}

void client::ConnectSuccess(void)
{
    qDebug()<<"connect success!";
    ConfigFIleSet("client_syspam","ip",my_syspam.ip);
    ConfigFIleSet("client_syspam","port",my_syspam.port);
    ConfigFIleSet("client_syspam","device_id",my_syspam.device_id);
}

void client::ConnectToHost(QString ip,qint32 port,qint32 dev_id)
{
//    if(FileUtils::exists(DIAL_FLAG_FILE)) {
//        isDial = true;
    my_syspam.ip = ip;
    my_syspam.port = port;
    my_syspam.device_id = dev_id;
    socket->abort();
    socket->connectToHost(my_syspam.ip,my_syspam.port);
//    }
    connect(timer, SIGNAL(timeout()), this, SLOT(TimeOut())); //将定时器与TimeOut函数绑定
    timer->start(1000);//一秒计时一次
}

void client::TCPsocket_Protocol(QByteArray DataBuf)
{
    procotol_syspam procotol_struct;
    if(!DataBuf.isEmpty()){
        QString data = QString(DataBuf);
        QStringList data_list = data.split(",");
        if(data_list.size() >= 7){  // 一条协议里至少有7种内容

             procotol_struct.FRAME_HEADER = data_list.at(0);  // head
             procotol_struct.direction = data_list.at(1).toInt();  // 方向
             procotol_struct.command_name  = data_list.at(2).toInt();  // 指令名称
             procotol_struct.command_serial = data_list.at(3).toInt();  // 指令序号
             procotol_struct.data_length = data_list.at(4).toInt();  // 数据长度
             procotol_struct.FRAME_END = data_list.at(data_list.size() - 1);  // 帧尾
        }
        if(procotol_struct.FRAME_HEADER == HEADER &&
                procotol_struct.FRAME_END == END) {  // 帧头和帧尾符合协议
            if(procotol_struct.direction == SERVICE2CLIENT) {   //  服务器主动调用和下发
                switch(procotol_struct.command_name){
                    case 33:  // 下发时钟数据
                        {
                              QString date = "date -s '";
                              char str[100];
                              time_t sec = data_list.at(5).toInt();//seconds form 1970/1/1/
                              struct tm *localTime;
                              localTime = localtime(&sec);
                              if(localTime != NULL){//有效时间
                                  strftime(str,100,"%c",localTime);
                                  QString date_time = QString(QLatin1String(str));
                                  date_time.insert(3,",");//保证格式正确
                                  date_time = date.append(date_time);
                                  date_time.append("'");
                                  char*  dates;
                                  QByteArray ba = date_time.toLatin1(); // must
                                  dates=ba.data();
                                  system(dates);
                                  char clock[] = "hwclock -w";
                                  system(clock);
                                  SendOK_Response(ANSWER_SERVICE2CLIENT,procotol_struct.command_name,procotol_struct.command_serial);
                              }
                        }
                        break;
                    case 34://LED屏开关命令 背光灯

                        break;
                    case 39://服务器下发公告信息

                        break;
                    case 61://紧急消息

                        break;
                    case 13://车辆分布位置
                        {
                            QStringList Data_buf;
                            qDebug()<<"444";
                            Data_buf = data_list.at(5).split(SINGAL_VEHICLE_END);  // 通过</line>分解字符串
                            if(Data_buf.at(Data_buf.size() - 1).contains(VEHICLE_LOCATION_FLAG,Qt::CaseSensitive)){  // 帧结构符合协议
                                for(int i = 0;i < Data_buf.size() - 1;i++){  // 去掉车辆信息中的<lines>结尾
                                    AddVehicleLocationTolist(Data_buf.at(i));
                                }
                                emit veh_data_re();
                            }
                        }
                        break;
                    case 41://批量通知

                        break;
                    case 8://下发命令
                        QString cmd;
                        cmd = data_list.at(5).mid(0,procotol_struct.data_length);
                        if(!cmd.compare(CMD_RESTSRT)) {  // 重启
                            system("reboot");
                        } else if(!cmd.compare(CMD_CLOSE)) {  // 关机
                            system("poweroff");
                        } else if(!cmd.compare(CMD_LIGHT_ON)) {  // 开灯

                        } else if(!cmd.compare(CMD_LIGHT_OFF)) {  // 关灯

                        } else if(!cmd.compare(CMD_UPDATE_SET)) {  // 更新设置

                        } else if(!cmd.compare(CMD_UPDATE_PRO)) {  // 更新节目
                            emit update_program();
                        } else if(!cmd.compare(CMD_UPDATE_LINE)) {  // 更新线路
                            emit update_lineinfo();
                        } else if(!cmd.compare(CMD_UPDATE_FILE)) {  // 软件升级

                        } else if(!cmd.compare(CMD_SCREENSHOT_ON)) {  // 截屏开

                        } else if(!cmd.compare(CMD_SCREENSHOT_OFF)) {  // 截屏关

                        } else if(!cmd.compare(CMD_LIGHT_LOW)) {  // 背光 低
                            system(BACK_LED1_OFF);
                            system(BACK_LED2_OFF);
                        } else if(!cmd.compare(CMD_LIGHT_MED)) {  // 背光 中
                            system(BACK_LED1_ON);
                            system(BACK_LED2_OFF);
                        } else if(!cmd.compare(CMD_LIGHT_HIG)) {  // 背光 高
                            system(BACK_LED1_ON);
                            system(BACK_LED2_ON);
                        } else if(!cmd.compare(CMD_SCREE_ON)) {  // 屏幕 开

                        } else if(!cmd.compare(CMD_SCREE_OFF)) {  // 屏幕 关

                        } else if(!cmd.compare(CMD_TEST_ON)) {  // 测试模式开

                        } else if(!cmd.compare(CMD_TEST_OFF)) {  // 测试模式关

                        } else if(!cmd.compare(CMD_GET_PARA)) {  // 获取初始化参数
                            emit get_initpara();
                        }
                        break;

                }
            } else if(procotol_struct.direction == ANSWER_CLIENT2SERVICE) {  // 服务器应答
                switch(procotol_struct.command_name) {
                case -1:
                    if(procotol_struct.command_serial == Serial) {
                        if(data_list.at(5) == 'O' && data_list.at(6) == 'K') {
                            mHeartbeatFlag = true;
                            qWarning()<<"Heartbeat OK.";
                        }
                    }
                    break;
                case 1:
                    if(procotol_struct.command_serial == Serial) {
                        if(data_list.at(5) == 'O' && data_list.at(6) == 'K') {
                            mSignUpFlag = true;
                            qWarning()<<"SignUp OK.";
                        }
                    }
                    break;
                }
            }
        }
    }
}

void client::SendOK_Response(qint8 direction, qint16 name, qint16 serial)  //  应答OK
{
    QString SendData;
    SendData.append(HEADER);
    SendData.append(",");
    SendData.append(QString::number(direction));
    SendData.append(",");
    SendData.append(QString::number(name));
    SendData.append(",");
    SendData.append(QString::number(serial));
    SendData.append(",2,OK,");
    SendData.append(END);
    socket->write(SendData.toLatin1());
}

void client::AddVehicleLocationTolist(QString data)  // 获取线路状态
{
    vehicle_localtion my_vehicle_localtion;
    //vehicle_list.push_back(Head);
    QString str =  data.section("\"",1,1);//截取线路名称    
    /*********gbk编码到Unicode转换*******/
    QTextCodec *gbk = QTextCodec::codecForName("gbk");
    QString unicode=gbk->toUnicode(str.toLocal8Bit());
    if(unicode.right(1) == "路"){//检测到“路”
        my_vehicle_localtion.vehicle_name = unicode.left(unicode.count() - 1);
    }else{
        my_vehicle_localtion.vehicle_name = unicode;
    }
    QString count_ = data.section("\"",3,3);//截取车距
    my_vehicle_localtion.count = count_;
    QStringList Singalvehicle_list = data.split(SPLIT_CHAR);//通过SPLIT_CHAR，把每一条线路下的每一辆车区分开来
    my_vehicle_localtion.vehicle_amount = 0;
    for(int j = 0;j < Singalvehicle_list.size();j++){
        if(Singalvehicle_list.at(j).contains("veh_id")){//统计车辆ID数量
            QString index =  Singalvehicle_list.at(j).section("\"",3,3);//截取station_index索引
            my_vehicle_localtion.station_index[my_vehicle_localtion.vehicle_amount] = index.toInt();
            my_vehicle_localtion.vehicle_amount++;
        }

    }
    vehicle_list.push_back(my_vehicle_localtion);
}

void client::ConfigFIleSet(QString NodeName, QString KeyName, QVariant vaule)  // 配置文件设置
{
    TCP_set_file = new QSettings(my_filename,QSettings::IniFormat);
    TCP_set_file->beginGroup(NodeName);
    TCP_set_file->setValue(KeyName,vaule);
    TCP_set_file->endGroup();
    delete TCP_set_file;
    TCP_set_file = 0;
}

QVariant client::ConfigFIleGet(QString NodeName, QString KeyName)  // 配置文件获取
{
    TCP_set_file = new QSettings(my_filename,QSettings::IniFormat);
    QVariant vaule = TCP_set_file->value(QString("/%1/%2").arg(NodeName).arg(KeyName));
    delete TCP_set_file;
    TCP_set_file = 0;
    return vaule;
}

void client::DeleteAll_vehicle()
{
    vehicle_list.clear();
}

void client::ReadAll_vehicle()
{
    for(vehicle_iterator = vehicle_list.begin();vehicle_iterator != vehicle_list.end();vehicle_iterator++){

    }
}

vehicle_localtion client::ReadSingle_vehicle()
{
    vehicle_localtion Node;
    if(vehicle_list.empty()){
        Node.vehicle_name = "";
        return Node;
    }else{
        vehicle_iterator = vehicle_list.begin();
        Node.vehicle_name = (*vehicle_iterator).vehicle_name;
        Node.count = (*vehicle_iterator).count;
        Node.vehicle_amount = (*vehicle_iterator).vehicle_amount;
        memcpy(Node.station_index,(*vehicle_iterator).station_index,sizeof(Node.station_index));
        vehicle_list.pop_front();
    }
    return Node;
}

void client::clientHeartbeat() // 终端心跳
{
    QString send_data;
    QString pack;
    qWarning("clientHeartbeat");
    send_data.append(HEADER);
    send_data.append(",0,-1,");
    send_data.append(QString::number(Serial));
    Serial++;
    send_data.append(",");
    pack.append("<?xml version=\"1.0\" encoding=\"utf-8\"?><root>");
    pack.append("<stationId>"+ QString::number(my_syspam.device_id) +"</stationId>");
    pack.append("<playingItem>0</playingItem><temperature><lcd>0</lcd><box>0</box></temperature>");
    pack.append("<humidity><lcd>80</lcd><box>80</box></humidity><bootState></bootState><fans>1000</fans>");
    pack.append("<accessControl>0</accessControl><volume>0</volume><illumination>0</illumination><TTS></TTS>");
    pack.append("<waterLevel></waterLevel><av></av><ac></ac><ap></ap><ate></ate><led></led><lumia></lumia>");
    pack.append("<heater></heater><dvr></dvr><camera></camera>< router4g></ router4g></root>");
    send_data.append(QString::number(pack.length()));   // 长度
    send_data.append(",");
    send_data.append(END);
    socket->write(send_data.toLatin1());
    qWarning()<<send_data;
}

void client::clientSignUp()
{
    //$GPRS,0,-1,?,n,123456;v1.0.0,$END$
    qWarning("clientSignUp");
    QString send_data;
    QString pack;
    send_data.append(HEADER);
    send_data.append(",0,-1,");
    send_data.append(QString::number(Serial));
    Serial++;
    send_data.append(",");
    pack.append(QString::number(my_syspam.device_id));
    pack.append(";");
    pack.append(Version);
    send_data.append(QString::number(pack.length()));   // 长度
    send_data.append(",");
    send_data.append(END);
    socket->write(send_data.toLatin1());
    qWarning()<<send_data;
}

void client::get_version()
{
    QString buildDate = __DATE__;
    QString pDest_y = buildDate.mid(9,2);
    QString pDest_m = buildDate.mid(0,3);
    QString pDest_d = buildDate.mid(4,2);

    if (pDest_d.at(0) == ' ')
        pDest_d[0] = '0';

    if (  pDest_m.compare("Jan") == 0)  pDest_m = "01";
    else if ( pDest_m.compare("Feb") == 0)  pDest_m = "02";
    else if ( pDest_m.compare("Mar") == 0)  pDest_m = "03";
    else if ( pDest_m.compare("Apr") == 0)  pDest_m = "04";
    else if ( pDest_m.compare("May") == 0)  pDest_m = "05";
    else if ( pDest_m.compare("Jun") == 0)  pDest_m = "06";
    else if ( pDest_m.compare("Jul") == 0)  pDest_m = "07";
    else if ( pDest_m.compare("Aug") == 0)  pDest_m = "08";
    else if ( pDest_m.compare("Sep") == 0)  pDest_m = "09";
    else if ( pDest_m.compare("Oct") == 0)  pDest_m = "10";
    else if ( pDest_m.compare("Nov") == 0)  pDest_m = "11";
    else if ( pDest_m.compare("Dec") == 0)  pDest_m = "12";
    else  pDest_m = "01";
    QString rang = __TIME__;  //QString::number(QTime(0,0,0).secsTo(QTime::currentTime())%(99 - 10) + 10);
    rang = rang.mid(3, 2);

    buildDate = pDest_y + pDest_m + pDest_d + rang;
    Version = buildDate.insert(0,"V").insert(3,".").insert(6,".").insert(9,".");
}

void client::TimeOut()
{
    if(isConnected()) {
        if(mSignUpFlag == false) {
            clientSignUp();
        } else {
            static uint8_t count = 0;

            if(mHeartbeatTime > 0) {
                mHeartbeatTime -= 1;
            } else {
                if(mHeartbeatFlag == false) {
                    count ++;
                    if(count > 4) {  // 未应答重传次数
                        count = 0;
                        socket->abort();
                        mSocketClientTime = 60;  // 断开链接1分钟后重连
                    }
                } else {
                    count = 0;
                }
                clientHeartbeat();
                mHeartbeatTime = 60;   // 1分钟
            }
        }
    } else {
        if(mSocketClientTime > 0) {
            mSocketClientTime -=1;
        } else {
            mSocketClientTime = 60;  //  如果没有连接到服务器则每60秒重连一次
            socket->abort();
            socket->connectToHost(my_syspam.ip,my_syspam.port);
        }

    }
}
//void client::socketConnect(bool is)
//{
//    if(!is) {
//        mSocketClientFlag = false;
//        mSocketClientTime = 5;
//        mSignUpFlag = false;
//    }
//}

bool client::isConnected()
{
    if(socket == NULL || (socket != NULL &&
            socket->state() != QTcpSocket::ConnectedState)) {
        //socketConnect(false);
        return false;
    } else {
       // socketConnect(true);
        return true;
    }
}
