#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QSettings>
#include <QVariant>
#include <iterator>
#include <list>
#include <algorithm>
#include <QTimer>
#include "fileutils.h"
#include "customize.h"
//#include <iostream>
#define HEADER                  "$GPRS"
#define END                     "$END$"
#define CLIENT2SERVICE          0         //终端主动上传数据
#define ANSWER_CLIENT2SERVICE   1         //服务器对终端上传数据的应答
#define SERVICE2CLIENT          2         //服务器调取（或者下发）终端的数据
#define ANSWER_SERVICE2CLIENT   3         //终端对服务器调取数据的应答
#define VEHICLE_AMOUNT          50        //一条线路的上车辆最大数量
#define VEHICLE_LOCATION_FLAG   "</lines>" //
#define SINGAL_VEHICLE_END      "</line>" //单独一条线路的结尾
#define SPLIT_CHAR              "vehicle "

#define BACK_LED1_CFG  "echo \"1\" > /sys/class/gpio_sw/PC22/cfg\n"
#define BACK_LED2_CFG  "echo \"1\" > /sys/class/gpio_sw/PC21/cfg\n"

#define BACK_LED1_ON  "echo \"1\" > /sys/class/gpio_sw/PC22/data\n"
#define BACK_LED1_OFF "echo \"0\" > /sys/class/gpio_sw/PC22/data\n"

#define BACK_LED2_ON  "echo \"1\" > /sys/class/gpio_sw/PC21/data\n"
#define BACK_LED2_OFF "echo \"0\" > /sys/class/gpio_sw/PC21/data\n"


#define CMD_RESTSRT         "restart"
#define CMD_CLOSE           "close"
#define CMD_LIGHT_ON        "light_on"
#define CMD_LIGHT_OFF       "light_off"
#define CMD_UPDATE_SET      "update_setting"
#define CMD_UPDATE_PRO      "update_program"
#define CMD_UPDATE_LINE     "update_lineinfo"
#define CMD_UPDATE_FILE     "update_software"
#define CMD_SCREENSHOT_ON   "screenshot_on"
#define CMD_SCREENSHOT_OFF  "screenshot_off"
#define CMD_LIGHT_LOW       "light_low"
#define CMD_LIGHT_MED       "light_medium"
#define CMD_LIGHT_HIG       "light_high"
#define CMD_SCREE_ON        "screen_on"
#define CMD_SCREE_OFF       "screen_off"
#define CMD_TEST_ON         "test_on"
#define CMD_TEST_OFF        "test_off"
#define CMD_GET_PARA        "get_initparams"

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
    static client* getInstance();
    void ConfigFIleSet(QString NodeName, QString KeyName, QVariant vaule);
    QVariant ConfigFIleGet(QString NodeName,QString KeyName);
    void DeleteAll_vehicle(void);
    void ReadAll_vehicle(void);
    vehicle_localtion ReadSingle_vehicle(void);
    void clientHeartbeat();
    void clientSignUp();
    bool isConnected();
    void socketConnect(bool is);

    QList<Msg> msg_list;
    client_syspam my_syspam;
private:
    void TCPsocket_Protocol(QByteArray DataBuf);
    void SendOK_Response(qint8 direction,qint16 name,qint16 serial);
    void AddVehicleLocationTolist(QString data);
    void get_version();    
  //  void command_handle(QString com);
    QTimer *timer;
    QTcpSocket    *socket;
    QSettings     *TCP_set_file;
    QString       my_filename;
    list<vehicle_localtion> vehicle_list;
    list<vehicle_localtion>::iterator vehicle_iterator;
    uint16_t Serial;
    QString Version;
    volatile bool mSocketClientFlag;
    volatile bool mHeartbeatFlag;
    volatile bool mSignUpFlag;
    volatile uint16_t mSocketClientTime;
    volatile uint16_t mHeartbeatTime;
signals:
    void veh_data_re();
    void get_initpara();
    void http_command(int);
    void to_ui_bulletin(Msg);
private slots:
    void ReadMsg(void);
    void ConnectSuccess(void);
    void TimeOut();
public slots:
    void ConnectToHost(QString ip,qint32 port,qint32 dev_id);
};

#endif // CLIENT_H
