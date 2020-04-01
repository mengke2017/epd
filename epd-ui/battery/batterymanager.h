#ifndef BATTERYMANGER_H
#define BATTERYMANGER_H

#include <QObject>
#include <QTimer>
//#include "serial/serialport.h"
#include "battery/batterylist.h"
#include <QSettings>
#include <QSerialPort>

#define DAY_OR_NIGHT              0

#define ARRAY_VOLTAGE             1
#define ARRAY_CURRENT             2
#define VOLTAGE                   3
#define CURRENT                   4
#define BATTERY_TEMPERATURE       5
#define TEMPERATURE               6
#define BATTERY_POWER             7//蓄电池剩余电量
#define BATTERY_VOLTAGE           8//蓄电池电压
#define BATTERY_VOLTAG_ELEVEL     9
#define MAX_VOLTAGE               10
#define MIN_VOLTAGE               11

#define ARRAY_POWER               12//发电功率 L
#define DAY_POWER_DISCHARGE       13//L
#define MONTH_POWER_DISCHARGE     14//L
#define YEAR_POWER_DISCHARGE      15//L
#define TOTAL_POWER_DISCHARGE     16//L
#define DAY_POWER_CHARGE          17//L
#define MONTH_POWER_CHARGE        18//L
#define YEAR_POWER_CHARGE         19//L
#define TOTAL_POWER_CHARGE        20//L
#define POWER                     21//负载功率 L
#define BATTERY_CURRENT           22//L
#define BATTERY_PARA_NUM          23
//有些数据是L和H构成的，发送tcp包需要把两个数据合并成一个数据，所以需要加一个特殊数据
#define SPEC_ARRAY_POWER_H             24
#define SPEC_POWER_H                   25
#define SPEC_DAY_POWER_DISCHARGE_H     26
#define SPEC_MONTH_POWER_DISCHARGE     27
#define SPEC_YEAR_POWER_DISCHARGE      28
#define SPEC_TOTAL_POWER_DISCHARGE     29
#define SPEC_DAY_POWER_CHARGE          30
#define SPEC_MONTH_POWER_CHARGE        31
#define SPEC_YEAR_POWER_CHARGE         32
#define SPEC_TOTAL_POWER_CHARGE        33

#define BATTERY_RATED_CURRENT            34//蓄电池额定电流
#define LOAD_RATED_CURRENT               35//负载额定电流
#define BATTERY_VOLTAG_ELEVEL_B            36//当前蓄电池电压等级
#define BATTERY_TYPE                     37//蓄电池类型
#define BATTERY_CAPACITY                 38//蓄电池总容量
#define TEMPERATURE_COMPENCITION         39//温度补偿系数
#define OVER_VOLTAG                      30//超压断开电压
#define CHARGE_LIMIT_VOLTAG              41//充电限制
#define OVER_VOLTAG_RECOVER              42//超压断开恢复电压
#define BALANCE_CHARGE_VOLTAG            43//均衡充电电压
#define HOIST_CHARGE_VOLTAG              44//提升充电电压
#define FLOAT_CHARGE_VOLTAG              45//浮充充电电压
#define HOIST_VOLTAG_RECOVER             46//提升恢复电压
#define LOW_BREAK_VOLTAG_RECOVER         47//低压断开恢复电压
#define UNDER_VOLTAG_WARNING_RECOVER     48//欠压告警恢复电压
#define UNDER_VOLTAG_WARNING             49//欠压告警电压
#define LOW_VOLTAG_BREAK                 50//低压断开电压
#define DISCHARGE_LIMLIT                 51//放电限制电压
#define SYSTEM_VOLTAG_LEVEL              52//系统额定电压等级
#define HANDLE_SWITCH                    53//手动控制条件下设定的开/关
#define BALANCE_TIME                     54//均衡维持时间
#define HOIST_TIME                       55//提升维持时间
#define DISCHARGE_DEEP                   56//放电深度
#define CHARGE_DEEP                      57//充电深度
#define CHARGE_MANAGER                   58//电池充放电管理模式


#define NO_REPLY_DATA 0x77
#define SAME_DATA     0x77

#define COMMAND_1    0
#define COMMAND_2    1
#define COMMAND_3    2
#define COMMAND_SUM  3

#define TIMER_MSEC      10 //ms
#define REPLY_OVERTIME  100  //overtime of 2 sec
#define OPEN            0
#define CLOSE           1
#define OVER_TIME       1
#define NORMAL          0

#define A_COMMAND       0
#define B_COMMAND       1

typedef struct battery_manager{
    QString dayOrNight;          //白天夜晚 1夜晚 0白天
    QString arrayVoltage;        //阵列电压 V
    QString arrayCurrent;        //阵列电流 A
    QString arrayPower;          //阵列发电功率 W
    QString voltage;             //负载电压 V
    QString current;             //负载电流 A
    QString power;               //负载功率 W
    QString batteryTemperature;  //电池温度 ℃
    QString temperature;         //设备机内温度 ℃
    QString batteryPower;        //电池剩余电量 %
    QString batteryVoltageLevel; //电池电压等级 V
    QString maxVoltage;          //当日最高电池电压 V
    QString minVoltage;          //当日最低电池电压 V
    QString dayPowerDischarge;   //当日累计用电量 KWH
    QString monthPowerDischarge; //当月累计用电量 KWH
    QString yearPowerDischarge;  //当年累计用电量 KWH
    QString totalPowerDischarge; //总累计用电量 KWH
    QString dayPowerCharge;      //当日累计充电量 KWH
    QString monthPowerCharge;    //当月累计充电量 KWH
    QString yearPowerCharge;     //当年累计充电量 KWH
    QString totalPowerCharge;    //总累计充电量 KWH
    QString batteryVoltage;      //蓄电池电压 V
    QString batteryCurrent;      //蓄电池电流 A
}BatteryPara;

class BatteryManger : public QObject
{
    Q_OBJECT
public:
    explicit BatteryManger(QObject *parent = 0);
    static BatteryManger* getInstance();
    BatteryPara   Battery_buffer;
private:
    volatile bool     SendSwitch;
    volatile quint16  LastAOrB;//判断上一次发送A指令还是B指令
    volatile quint16  CmdIndex;
    volatile quint16  Last_Comannd;
    volatile quint16  Cmd_OverTime;
    volatile quint16  ReplyTimer;

    QTimer*      Timer_Basic;
    QTimer*      Timer_Send;
    QSerialPort*      BatterySerial;
    BatteryList* list;
    QSettings*   Battery_file;

    BatterySyspam Send_Pack;

    void SendPack(BatterySyspam& send_pack);
    void Update_SendPack(quint16 index);
    quint16 CRC16_Modbus(QByteArray& arry);
    void DataRecieveHandle();
    void BatteryIntAndStringHandle(BatteryPara& Battery_data);
    QVariant BatteryFIleGet(QString file_name, QString NodeName, QString KeyName);
    void BatteryFIleSet(QString file_name, QString NodeName, QString KeyName, QVariant vaule);
    float ReplyDataCalcul(const QByteArray& Data, quint16 Cmd);

    void ReadBatteryParam(quint16 addr, quint16 ParamNum);
    void WriteBatteryParam(quint16 addr, quint16 addr_num, quint8 data_num, const QByteArray& data);
signals:
    void RecieveOver();
private slots:
    void Basic_TimeOut();
    void Send_TimeOut();
    void DataSendHandle();
};

#endif // BATTERYMANGER_H
