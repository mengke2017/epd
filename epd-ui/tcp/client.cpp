#include "client.h"
#include <QDebug>
#include <time.h>
#include <QDateTime>
#include <QString>
#include <QTextCodec>
#include "tcp/StationCommand.h"
#include "battery/batterymanager.h"

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
client* client::getInstance()
{
    static client instance;
    return &instance;
}
void client::ReadMsg(void)
{
    QByteArray msg = socket->readAll();
//    qWarning()<<"msg: "<<msg;
    TCPsocket_Protocol(msg);
}

void client::ConnectSuccess(void)
{
    qWarning()<<"connect success!";
    ConfigFIleSet("client_syspam","ip",my_syspam.ip);
    ConfigFIleSet("client_syspam","port",my_syspam.port);
    ConfigFIleSet("client_syspam","device_id",my_syspam.device_id);
}

void client::ConnectToHost(QString ip,qint32 port,qint32 dev_id)
{
    my_syspam.ip = ip;
    my_syspam.port = port;
    my_syspam.device_id = dev_id;
    socket->abort();
    socket->connectToHost(my_syspam.ip,my_syspam.port);
    connect(timer, SIGNAL(timeout()), this, SLOT(TimeOut())); //将定时器与TimeOut函数绑定
    timer->start(5000);//一秒计时一次
}

void client::TCPsocket_Protocol(QByteArray DataBuf)
{
    procotol_syspam procotol_struct;
    if(!DataBuf.isEmpty()){
        /*********gbk编码到Unicode转换*******/
        QTextCodec *gbk = QTextCodec::codecForName("gbk");
        QString data=gbk->toUnicode(DataBuf.data());
        QStringList data_list = data.split(",");
        if(data_list.size() >= 7) {  // 一条协议里至少有7种内容
             procotol_struct.FRAME_HEADER = data_list.at(0);  // head
             procotol_struct.direction = data_list.at(1).toInt();  // 方向
             procotol_struct.command_name  = data_list.at(2).toInt();  // 指令名称
             procotol_struct.command_serial = data_list.at(3).toInt();  // 指令序号
             procotol_struct.data_length = data_list.at(4).toInt();  // 数据长度
             procotol_struct.FRAME_END = data_list.at(data_list.size() - 1);  // 帧尾
             data = data_list.at(5).mid(0,procotol_struct.data_length);
        }
        if(procotol_struct.FRAME_HEADER == HEADER &&
                procotol_struct.FRAME_END == END) {  // 帧头和帧尾符合协议

            if(procotol_struct.direction == SERVICE2CLIENT || procotol_struct.direction == 8) {   //  服务器主动调用和下发
                switch(procotol_struct.command_name) {
                    case 33:  // 下发时钟数据
                        {

                    qWarning("cmd time!");
                              time_t sec = data_list.at(5).toInt();//seconds form 1970/1/1/
                              QString date = "date -s \"";
                              QDateTime dt = QDateTime::fromTime_t(sec);
                              QString strDate = dt.toString(Qt::ISODate);//日期格式自定义

                              if(strDate != NULL) {//有效时间
                                  QString date_time;
                             //     qWarning()<<strDate;
                                  date_time = date.append(strDate.replace("T"," "));
                                  date_time.append("\"");
                             //     qWarning()<<date_time;
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
                    case 34://LED屏开关命令
                    qWarning("cmd LED ON OFF!");
                    SendOK_Response(ANSWER_SERVICE2CLIENT,procotol_struct.command_name,procotol_struct.command_serial);
                        break;
                    case 12://到站信息
                    qWarning("cmd station info!");
                    SendOK_Response(ANSWER_SERVICE2CLIENT,procotol_struct.command_name,procotol_struct.command_serial);
                    break;
                    case 39://服务器下发公告信息
                    qWarning("cmd serv mesg!");

                        break;
                    case 61://紧急消息
                    qWarning("cmd 61!");

                        break;
                    case 13://车辆分布位置
                        {
                            qWarning()<<"cmd car pos!";//<<data;
                            QStringList Data_buf;

                            Data_buf = data_list.at(5).split(SINGAL_VEHICLE_END);  // 通过</line>分解字符串
                            if(Data_buf.at(Data_buf.size() - 1).contains(VEHICLE_LOCATION_FLAG,Qt::CaseSensitive)){  // 帧结构符合协议
                           //     qDebug()<<"444";
                                for(int i = 0; i < Data_buf.size()-1; i++){  // 去掉车辆信息中的<lines>结尾
                                    AddVehicleLocationTolist(Data_buf.at(i));
                                }
                                emit veh_data_re();
                            }
                        }
                        break;
                    case 41://批量通知
                        /*<?xml version="1.0" encoding="gb2312"?>
                            <root>
                                <msgs date="">
                                    <!--单条消息
                                    type：1显示器滚动区，2，媒体播放区，3，LED屏
                                    bgdate：开始日期
                                    enddate：结束日期
                                    bgtime：开始时间
                                    endtime：结束时间
                                    value：消息内容
                                    -->
                                    <msg type="3" bgdate="" enddate="" bgtime="" endtime=""  value =""></msg>
                                </msgs>
                            </root>*/
                    {
                        qWarning("cmd bulletin ");
                        //qWarning()<<data;
                        QStringList Data_buf;
                        QStringList msg_buf;
                        Msg msg;
                        QString end;
                        int16_t star_index = 0, end_index = 0;
                        star_index = data.indexOf("<msg ");
                        if(star_index < 0)
                            break;
                        end_index = data.indexOf("</msg>",star_index);
                        end = "</msg>";
                        if(end_index < 0) {  // 格式不正确
                            end_index = data.indexOf("/>",star_index);
                            end = "/>";
                            if(end_index < 0)
                            break;
                        }
                        data = data.mid(star_index, end_index-star_index);
                        Data_buf = data.split(end);  // 通过</msg>分解字符串
                        for(uint16_t i = 0; i < Data_buf.length(); i++) {
                            msg_buf = Data_buf.at(i).split("\"");
                        if(msg_buf.length() < 11)
                            break;
                        msg.type = msg_buf.at(1);
                        msg.bgdate = msg_buf.at(3);
                        msg.enddate = msg_buf.at(5);
                        msg.bgtime = msg_buf.at(7);
                        msg.endtime = msg_buf.at(9);
                        msg.value = msg_buf.at(11);
                     //   qWarning()<<"msg.value: "<<msg.value;
                        msg_list.append(msg);
                        emit to_ui_bulletin(msg);
                        }
                        break;
                    }
                    case 8://下发命令
                    qWarning("cmd down cmd");
                        QString cmd;
                        cmd = data_list.at(5).mid(0,procotol_struct.data_length);
                       // command_handle(cmd);
                        if(!cmd.compare(CMD_RESTSRT)) {  // 重启
                            qWarning("reboot");
                            //system("reboot");
                            SendOK_Response(ANSWER_SERVICE2CLIENT, 0,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_CLOSE)) {  // 关机
                            qWarning("poweroff");
                            //system("poweroff");
                            SendOK_Response(ANSWER_SERVICE2CLIENT, 1,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_LIGHT_ON)) {  // 开灯
                            qWarning("CMD_LIGHT_ON");
                            SendOK_Response(ANSWER_SERVICE2CLIENT, 2,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_LIGHT_OFF)) {  // 关灯
                            qWarning("CMD_LIGHT_OFF");
                            SendOK_Response(ANSWER_SERVICE2CLIENT, 3,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_UPDATE_SET)) {  // 更新设置
                            qWarning("CMD_UPDATE_SET");
                            SendOK_Response(ANSWER_SERVICE2CLIENT, 5,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_UPDATE_PRO)) {  // 更新节目
                            qWarning("CMD_UPDATE_PRO");
                            //emit update_program();
                            SendOK_Response(ANSWER_SERVICE2CLIENT, 4,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_UPDATE_LINE)) {  // 更新线路
                            qWarning("CMD_UPDATE_LINE");
                            emit http_command(UPDATE_LINE_HTTP);
                            SendOK_Response(ANSWER_SERVICE2CLIENT, 6,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_UPDATE_FILE)) {  // 软件升级
                            qWarning("CMD_UPDATE_FILE");

                            SendOK_Response(ANSWER_SERVICE2CLIENT, 7,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_SCREENSHOT_ON)) {  // 截屏开
                            qWarning("CMD_SCREENSHOT_ON");

                            SendOK_Response(ANSWER_SERVICE2CLIENT, 10,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_SCREENSHOT_OFF)) {  // 截屏关
                            qWarning("CMD_SCREENSHOT_OFF");

                            SendOK_Response(ANSWER_SERVICE2CLIENT, 11,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_LIGHT_LOW)) {  // 背光 低
                            qWarning("CMD_LIGHT_LOW");
                            system(BACK_LED1_OFF);
                            system(BACK_LED2_OFF);
                            SendOK_Response(ANSWER_SERVICE2CLIENT, 12,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_LIGHT_MED)) {  // 背光 中
                            qWarning("CMD_LIGHT_MED");
                            system(BACK_LED1_ON);
                            system(BACK_LED2_OFF);
                            SendOK_Response(ANSWER_SERVICE2CLIENT, 13,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_LIGHT_HIG)) {  // 背光 高
                            qWarning("CMD_LIGHT_HIG");
                            system(BACK_LED1_ON);
                            system(BACK_LED2_ON);
                            SendOK_Response(ANSWER_SERVICE2CLIENT,14,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_SCREE_ON)) {  // 屏幕 开
                            qWarning("CMD_SCREE_ON");

                            SendOK_Response(ANSWER_SERVICE2CLIENT, 20,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_SCREE_OFF)) {  // 屏幕 关
                            qWarning("CMD_SCREE_OFF");

                            SendOK_Response(ANSWER_SERVICE2CLIENT, 21,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_TEST_ON)) {  // 测试模式开
                            qWarning("CMD_TEST_ON");

                            SendOK_Response(ANSWER_SERVICE2CLIENT, 25,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_TEST_OFF)) {  // 测试模式关
                            qWarning("CMD_TEST_OFF");

                            SendOK_Response(ANSWER_SERVICE2CLIENT, 26,procotol_struct.command_serial);
                        } else if(!cmd.compare(CMD_GET_PARA)) {  // 获取初始化参数
                            qWarning("CMD_GET_PARA");
                            emit http_command(GET_INI_HTTP);
                            SendOK_Response(ANSWER_SERVICE2CLIENT, 35,procotol_struct.command_serial);
                        }
                        break;
                }
            } else if(procotol_struct.direction == ANSWER_CLIENT2SERVICE) {  // 服务器应答
                switch(procotol_struct.command_name) {
           //     msg:  "$GPRS,1,1,1,2,OK,$END$"
                case -1:
               //     qWarning()<<"heartbeat "<<data;
                 //   if(procotol_struct.command_serial == Serial) {
                        if(!data.compare("OK")) {
                            mHeartbeatFlag = true;
                            qWarning()<<"Heartbeat OK.";
                        }
                 //   }
                    break;
                case 1:
               //     qWarning()<<"signup "<<data;
                 //   if(procotol_struct.command_serial == Serial) {
                        if(!data.compare("OK")) {
                            mSignUpFlag = true;
                            qWarning()<<"SignUp OK.";
                        }
                 //   }
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
 //   qWarning("222AddVehicleLocationTolist");
    vehicle_localtion my_vehicle_localtion;
    //vehicle_list.push_back(Head);
    QString unicode =  data.section("\"",1,1);//截取线路名称
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
         //   qWarning()<<"index: "<<index;
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
    QString InkInfo;
    BatteryPara batteryPara;
    qWarning("clientHeartbeat");
    send_data.append(HEADER);
    send_data.append(",0,-1,");
    Serial++;
    send_data.append(QString::number(Serial));
    send_data.append(",");
    pack.append("<?xml version=\"1.0\" encoding=\"utf-8\"?><root>");
    pack.append("<stationId>"+ QString::number(my_syspam.device_id) +"</stationId>");
    pack.append("<playingItem>0</playingItem><temperature><lcd>0</lcd><box>0</box></temperature>");
    pack.append("<humidity><lcd>0</lcd><box>0</box></humidity><bootState></bootState><fans>0</fans>");
    pack.append("<accessControl>0</accessControl><volume>0</volume><illumination>0</illumination><TTS></TTS>");
    pack.append("<waterLevel></waterLevel><av></av><ac></ac><ap></ap><ate></ate><led></led><lumia></lumia>");
    pack.append("<heater></heater><dvr></dvr><camera></camera><router4g></router4g></root>");

    if(batteryPara.dayOrNight.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<dayOrNight>"+ batteryPara.dayOrNight +"</dayOrNight>");
    if(batteryPara.arrayVoltage.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<arrayVoltage>"+ batteryPara.arrayVoltage +"</arrayVoltage>");
    if(batteryPara.arrayCurrent.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<arrayCurrent>"+ batteryPara.arrayCurrent +"</arrayCurrent>");
    if(batteryPara.arrayPower.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<arrayPower>"+ batteryPara.arrayPower +"</arrayPower>");
    if(batteryPara.voltage.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<voltage>"+ batteryPara.voltage +"</voltage>");
    if(batteryPara.current.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<current>"+ batteryPara.current +"</current>");
    if(batteryPara.arrayCurrent.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<power>"+ batteryPara.power +"</power>");
    if(batteryPara.batteryTemperature.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<batteryTemperature>"+ batteryPara.batteryTemperature +"</batteryTemperature>");
    if(batteryPara.temperature.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<temperature>"+ batteryPara.temperature +"</temperature>");
    if(batteryPara.batteryPower.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<batteryPower>"+ batteryPara.batteryPower +"</batteryPower>");
    if(batteryPara.batteryVoltage.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<batteryVoltage>"+ batteryPara.batteryVoltage +"</batteryVoltage>");
    if(batteryPara.maxVoltage.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<maxVoltage>"+ batteryPara.maxVoltage +"</maxVoltage>");
    if(batteryPara.dayPowerDischarge.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<dayPowerDischarge>"+ batteryPara.dayPowerDischarge +"</dayPowerDischarge>");
    if(batteryPara.monthPowerDischarge.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<monthPowerDischarge>"+ batteryPara.monthPowerDischarge +"</monthPowerDischarge>");
    if(batteryPara.yearPowerDischarge.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<yearPowerDischarge>"+ batteryPara.yearPowerDischarge +"</yearPowerDischarge>");
    if(batteryPara.totalPowerDischarge.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<totalPowerDischarge>"+ batteryPara.totalPowerDischarge +"</totalPowerDischarge>");
    if(batteryPara.dayPowerCharge.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<dayPowerCharge>"+ batteryPara.dayPowerCharge +"</dayPowerCharge>");
    if(batteryPara.monthPowerCharge.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<monthPowerCharge>"+ batteryPara.monthPowerCharge +"</monthPowerCharge>");
    if(batteryPara.yearPowerCharge.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<yearPowerCharge>"+ batteryPara.yearPowerCharge +"</yearPowerCharge>");
    if(batteryPara.totalPowerCharge.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<totalPowerCharge>"+ batteryPara.totalPowerCharge +"</totalPowerCharge>");
    if(batteryPara.batteryVoltage.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<batteryVoltage>"+ batteryPara.batteryVoltage +"</batteryVoltage>");
    if(batteryPara.batteryCurrent.compare("-1"))  // 如果值不等于-1
        InkInfo.append("<batteryCurrent>"+ batteryPara.batteryCurrent +"</batteryCurrent>");

    pack.append("<inkScreen>"+InkInfo+"</inkScreen>");
    send_data.append(QString::number(pack.length()));   // 长度
    send_data.append(",");
    send_data.append(pack);
    send_data.append(",");
    send_data.append(END);
    socket->write(send_data.toLatin1());
//    qWarning()<<"send:"<<send_data;
}

void client::clientSignUp()
{
    //$GPRS,0,1,?,n,123456;v1.0.0,$END$
    qWarning("clientSignUp");
    QString send_data;
    QString pack;
    send_data.append(HEADER);
    send_data.append(",0,1,");  //  方向+命令
    Serial++;
    send_data.append(QString::number(Serial));  //
    send_data.append(",");
    pack.append(QString::number(my_syspam.device_id));
    pack.append(";");
    pack.append(Version);
    send_data.append(QString::number(pack.length()));   // 长度
    send_data.append(",");
    send_data.append(pack);
    send_data.append(",");
    send_data.append(END);
    socket->write(send_data.toLatin1());
 //   qWarning()<<"send:"<<send_data;
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
                        mSignUpFlag = false;
                        mSocketClientTime = 12;  // 断开链接1分钟后重连
                    }
                } else {
                    count = 0;
                }
                clientHeartbeat();
                mHeartbeatTime = 6;   // 1分钟
            }
        }
    } else {
        if(mSocketClientTime > 0) {
            mSocketClientTime -=1;
        } else {
            mSocketClientTime = 12;  //  如果没有连接到服务器则每60秒重连一次
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
        qWarning("disconnect!");
        return false;
    } else {
       // socketConnect(true);
        return true;
    }
}
