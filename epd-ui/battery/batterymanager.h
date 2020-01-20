#ifndef BATTERYMANAGER_H
#define BATTERYMANAGER_H

#include <QObject>
#include <QSettings>
#include <QMutex>
#include <QThread>
#include "serial/serial.h"
//实时参数
#define DAY_OR_NIGHT              0
#define ARRAY_VOLTAGE             1
#define ARRAY_CURRENT             2
#define VOLTAGE                   3
#define CURRENT                   4
#define BATTERY_TEMPERATURE       5
#define TEMPERATURE               6
#define BATTERY_POWER             7
#define BATTERY_VOLTAGE           8
#define ARRAY_POWER               9//
#define BATTERY_VOLTAG_ELEVEL     10//
#define MAX_VOLTAGE               11//
#define MIN_VOLTAGE               12//
#define DAY_POWER_DISCHARGE       13//
#define MONTH_POWER_DISCHARGE     14//
#define YEAR_POWER_DISCHARGE      15//
#define TOTAL_POWER_DISCHARGE     16//
#define DAY_POWER_CHARGE          17//
#define MONTH_POWER_CHARGE        18//
#define YEAR_POWER_CHARGE         19//
#define TOTAL_POWER_CHARGE        20//
#define POWER                     21//
#define BATTERY_CURRENT           22//
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
typedef struct battery_para{
    int data_len;
    quint16 crc;
    QByteArray arry;
}Battery_data;

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

class BatteryManager : public QObject
{
    Q_OBJECT
public:
    explicit BatteryManager(QObject *parent = 0);
    void ReadBatteryPara(BatteryPara *Battery_data);
    static BatteryManager * BatteryManagerInt();
private :
    void BatteryFIleSet(QString file_name, QString NodeName, QString KeyName, QVariant vaule);
    void BatteryIntAndStringHandle(int num, BatteryPara *Battery_data);
    void BatterySendDataCommand_A(int num);
    void BatterySendDataCommand_B(void);
    void BatterySendData(Battery_data *data);
    void BatteryWaitData(Battery_data *recieve_data, int Comannd_num);
    QVariant BatteryFIleGet(QString file_name, QString NodeName, QString KeyName);
    quint16 CRC16_Modbus(QByteArray &arry);
    QSettings     *Battery_file;
    serial        *Battery_serial;
    QString       Battry_File_name;
    QMutex        Battery_mutex;
    quint8        Current_command;
    BatteryPara   Battery_buffer;
signals:

public slots:
    void BatteryReadData();
};

#endif // BATTERYMANAGER_H
