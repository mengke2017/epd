#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QSettings>
#include <QVariant>
#include <iterator>
#include <list>
#include <algorithm>
//#include <iostream>
#define HEADER                  "$GPRS"
#define END                     "$END$"
#define CLIENT2SERVICE          0         //终端主动上传数据
#define ANSWER_CLIENT2SERVICE   1         //服务器对终端上传数据的应答
#define SERVICE2CLIENT          2         //服务器调取（或者下发）终端的数据
#define ANSWER_SERVICE2CLIENT   3         //终端对服务器调取数据的应答
#define VEHICLE_AMOUNT          50        //一条线路的上车辆最大数量
#define VEHICLE_LOCATION_FLAG   "<lines>" //
#define SINGAL_VEHICLE_END      "</line>" //单独一条线路的结尾
#define SPLIT_CHAR              "vehicle "
using namespace std;
typedef struct tcp_syspam{
    QString ip;
    qint32  port;
    qint32  device_id;
}client_syspam;

typedef struct tcp_procotol_syspam{
    QString FRAME_HEADER;
    QString FRAME_END;
    QString buf;
    qint8   direction;
    qint16  command_name;
    qint16  command_serial;
    qint16  data_length;
}procotol_syspam;

typedef struct vehicle_information{
    QString vehicle_name;
    QString count;
    qint8   vehicle_amount;
    qint8   station_index[VEHICLE_AMOUNT];
}vehicle_localtion;

class client : public QObject
{
    Q_OBJECT
public:
    explicit client(QObject *parent = 0);
    void ConfigFIleSet(QString NodeName, QString KeyName, QVariant vaule);
    QVariant ConfigFIleGet(QString NodeName,QString KeyName);
    void DeleteAll_vehicle(void);
    void ReadAll_vehicle(void);
    vehicle_localtion ReadSingle_vehicle(void);
private:
    void TCPsocket_Protocol(QByteArray DataBuf);
    void SendOK_Response(qint8 direction,qint16 name,qint16 serial);
    void AddVehicleLocationTolist(QString data);
    QTcpSocket    *socket;
    QSettings     *TCP_set_file;
    QString       my_filename;
    client_syspam my_syspam;
    list<vehicle_localtion> vehicle_list;
    list<vehicle_localtion>::iterator vehicle_iterator;
signals:
    void veh_data_re();
private slots:
    void ReadMsg(void);
    void ConnectSuccess(void);
public slots:
    void ConnectToHost(QString ip,qint32 port,qint32 dev_id);
};

#endif // CLIENT_H
