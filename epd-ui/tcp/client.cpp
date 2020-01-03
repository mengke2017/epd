#include "client.h"
#include <QDebug>
#include <time.h>
#include <QTextCodec>
client::client(QObject *parent) :
    QObject(parent)
{
    socket = new QTcpSocket(this);
    my_filename = "/SocketSyspam.ini";
    connect(socket,SIGNAL(connected()),this,SLOT(ConnectSuccess()));
    connect(socket,SIGNAL(readyRead()),this,SLOT(ReadMsg()));
    //my_syspam.device_id = ConfigFIleGet("client_syspam","device_id").toInt();
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
    my_syspam.ip = ip;
    my_syspam.port = port;
    my_syspam.device_id = dev_id;
    socket->abort();
    socket->connectToHost(my_syspam.ip,my_syspam.port);
}

void client::TCPsocket_Protocol(QByteArray DataBuf)
{
    procotol_syspam procotol_struct;
    if(!DataBuf.isEmpty()){
        QString data = QString(DataBuf);
        QStringList data_list = data.split(",");
        if(data_list.size() >= 7){//一条协议里至少有7种内容

             procotol_struct.FRAME_HEADER = data_list.at(0);//head
             procotol_struct.direction = data_list.at(1).toInt();//方向
             procotol_struct.command_name  = data_list.at(2).toInt();//指令名称
             procotol_struct.command_serial = data_list.at(3).toInt();//指令序号
             procotol_struct.data_length = data_list.at(4).toInt();//数据长度
             procotol_struct.FRAME_END = data_list.at(data_list.size() - 1);//帧尾
        }
        if(procotol_struct.FRAME_HEADER == HEADER &&
                procotol_struct.FRAME_END == END){//帧头和帧尾符合协议
            if(procotol_struct.direction == SERVICE2CLIENT){
                switch(procotol_struct.command_name){
                    case 0://版本信息

                        break;
                    case 1://设备号

                        break;
                    case 33://下发时钟数据
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
                    case 34://LED屏开关命令

                        break;
                    case 39://服务器下发公告信息

                        break;
                    case 61://紧急消息

                        break;
                    case 13://车辆分布位置
                        {
                            QStringList Data_buf;
                            Data_buf = data_list.at(5).split(SINGAL_VEHICLE_END);//通过</line>分解字符串
                            if(Data_buf.at(Data_buf.size() - 1).contains(VEHICLE_LOCATION_FLAG,Qt::CaseSensitive)){//帧结构符合协议
                                for(int i = 0;i < Data_buf.size() - 1;i++){//去掉车辆信息中的<lines>结尾
                                    AddVehicleLocationTolist(Data_buf.at(i));
                                }
                                emit veh_data_re();
                            }

                        }
                        break;
                    case 41://批量通知

                        break;
                    case 8://下发命令

                        break;

                }
            }
        }
    }
}

void client::SendOK_Response(qint8 direction, qint16 name, qint16 serial)
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

void client::AddVehicleLocationTolist(QString data)
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

void client::ConfigFIleSet(QString NodeName, QString KeyName, QVariant vaule)
{
    TCP_set_file = new QSettings(my_filename,QSettings::IniFormat);
    TCP_set_file->beginGroup(NodeName);
    TCP_set_file->setValue(KeyName,vaule);
    TCP_set_file->endGroup();
    delete TCP_set_file;
    TCP_set_file = 0;
}

QVariant client::ConfigFIleGet(QString NodeName, QString KeyName)
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
