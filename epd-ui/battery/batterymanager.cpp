#include "batterymanager.h"
#include "customize.h"
#include <QDebug>
#include "tcp/StationCommand.h"
#include "systemutils.h"

#define RS485_PATH "/dev/ttyS5"

BatteryManager::BatteryManager(QObject *parent) :
    QObject(parent)
{
    Battry_File_name = CONFIG_FILE;
    Battery_serial = new serial();
    QString name = RS485_PATH;
    Battery_serial->openPort(name,BAUD115200,DATA_8,PAR_NONE,STOP_1,FLOW_OFF,TIME_OUT);
    connect(Battery_serial,SIGNAL(BatteryHasData()),this,SLOT(BatteryReadData()));
    emit Battery_serial->BatteryHasData();
}

void BatteryManager::ReadBatteryPara(BatteryPara *Battery_data)
{
    for(int j = DAY_OR_NIGHT;j < BATTERY_PARA_NUM;j++){
        Battery_mutex.tryLock(1000);
        BatterySendDataCommand_A(j);
        Current_command = j;
    }
    Battery_data = &Battery_buffer;
    for(int j = DAY_OR_NIGHT;j < BATTERY_PARA_NUM;j++)
        BatteryIntAndStringHandle(j, Battery_data);
}

BatteryManager *BatteryManager::BatteryManagerInt()
{
    static BatteryManager BatteryIntmanager;
    return &BatteryIntmanager;
}

void BatteryManager::BatteryFIleSet(QString file_name, QString NodeName, QString KeyName, QVariant vaule)
{
    Battery_file = new QSettings(file_name,QSettings::IniFormat);
    Battery_file->beginGroup(NodeName);
    Battery_file->setValue(KeyName,vaule);
    Battery_file->endGroup();
    delete Battery_file;
    Battery_file = 0;
}

void BatteryManager::BatteryIntAndStringHandle(int num, BatteryPara *Battery_data)
{
    QString BattaryPara;
    switch(num){
        case DAY_OR_NIGHT:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","dayOrNight").toString();
            BatteryFIleSet(Battry_File_name,"Battary","dayOrNight",Battery_data->dayOrNight);
            if(BattaryPara.compare(Battery_data->dayOrNight) == 0)
                Battery_data->dayOrNight = "-1";
            break;
        case ARRAY_VOLTAGE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","arrayVoltage").toString();
            BatteryFIleSet(Battry_File_name,"Battary","arrayVoltage",Battery_data->arrayVoltage);
            if(BattaryPara.compare(Battery_data->arrayVoltage) == 0)
                Battery_data->arrayVoltage = "-1";
            break;
        case ARRAY_CURRENT:
             BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","arrayCurrent").toString();
             BatteryFIleSet(Battry_File_name,"Battary","arrayCurrent",Battery_data->arrayCurrent);
             if(BattaryPara.compare(Battery_data->arrayCurrent) == 0)
                 Battery_data->arrayCurrent = "-1";
             break;
        case ARRAY_POWER:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","arrayPower").toString();
            BatteryFIleSet(Battry_File_name,"Battary","arrayPower",Battery_data->arrayPower);
            if(BattaryPara.compare(Battery_data->arrayPower) == 0)
                Battery_data->arrayPower = "-1";
            break;
        case VOLTAGE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","voltage").toString();
            BatteryFIleSet(Battry_File_name,"Battary","voltage",Battery_data->voltage);
            if(BattaryPara.compare(Battery_data->voltage) == 0)
                Battery_data->voltage = "-1";
            break;
        case CURRENT:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","current").toString();
            BatteryFIleSet(Battry_File_name,"Battary","current",Battery_data->current);
            if(BattaryPara.compare(Battery_data->current) == 0)
                Battery_data->current = "-1";
            break;
        case POWER:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","power").toString();
            BatteryFIleSet(Battry_File_name,"Battary","power",Battery_data->power);
            if(BattaryPara.compare(Battery_data->power) == 0)
                Battery_data->power = "-1";
            break;
        case BATTERY_TEMPERATURE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","batteryTemperature").toString();
            BatteryFIleSet(Battry_File_name,"Battary","batteryTemperature",Battery_data->batteryTemperature);
            if(BattaryPara.compare(Battery_data->batteryTemperature) == 0)
                Battery_data->batteryTemperature = "-1";
            break;
        case TEMPERATURE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","temperature").toString();
            BatteryFIleSet(Battry_File_name,"Battary","temperature",Battery_data->temperature);
            if(BattaryPara.compare(Battery_data->temperature) == 0)
                Battery_data->temperature = "-1";
            break;
        case BATTERY_POWER:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","batteryPower").toString();
            BatteryFIleSet(Battry_File_name,"Battary","batteryPower",Battery_data->batteryPower);
            if(BattaryPara.compare(Battery_data->batteryPower) == 0)
                Battery_data->batteryPower = "-1";
            break;
        case BATTERY_VOLTAG_ELEVEL:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","batteryVoltageLevel").toString();
            BatteryFIleSet(Battry_File_name,"Battary","batteryVoltageLevel",Battery_data->batteryVoltageLevel);
            if(BattaryPara.compare(Battery_data->batteryVoltageLevel) == 0)
                Battery_data->batteryVoltageLevel = "-1";
            break;
        case MAX_VOLTAGE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","maxVoltage").toString();
            BatteryFIleSet(Battry_File_name,"Battary","maxVoltage",Battery_data->maxVoltage);
            if(BattaryPara.compare(Battery_data->maxVoltage) == 0)
                Battery_data->maxVoltage = "-1";
            break;
        case MIN_VOLTAGE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","minVoltage").toString();
            BatteryFIleSet(Battry_File_name,"Battary","minVoltage",Battery_data->minVoltage);
            if(BattaryPara.compare(Battery_data->minVoltage) == 0)
                Battery_data->minVoltage = "-1";
            break;
        case DAY_POWER_DISCHARGE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","dayPowerDischarge").toString();
            BatteryFIleSet(Battry_File_name,"Battary","dayPowerDischarge",Battery_data->dayPowerDischarge);
            if(BattaryPara.compare(Battery_data->dayPowerDischarge) == 0)
                Battery_data->dayPowerDischarge = "-1";
            break;
        case MONTH_POWER_DISCHARGE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","monthPowerDischarge").toString();
            BatteryFIleSet(Battry_File_name,"Battary","monthPowerDischarge",Battery_data->monthPowerDischarge);
            if(BattaryPara.compare(Battery_data->monthPowerDischarge) == 0)
                Battery_data->monthPowerDischarge = "-1";
            break;
        case YEAR_POWER_DISCHARGE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","yearPowerDischarge").toString();
            BatteryFIleSet(Battry_File_name,"Battary","yearPowerDischarge",Battery_data->yearPowerDischarge);
            if(BattaryPara.compare(Battery_data->yearPowerDischarge) == 0)
                Battery_data->yearPowerDischarge = "-1";
            break;
        case TOTAL_POWER_DISCHARGE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","totalPowerDischarge").toString();
            BatteryFIleSet(Battry_File_name,"Battary","totalPowerDischarge",Battery_data->totalPowerDischarge);
            if(BattaryPara.compare(Battery_data->totalPowerDischarge) == 0)
                Battery_data->totalPowerDischarge = "-1";
            break;
        case DAY_POWER_CHARGE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","dayPowerCharge").toString();
            BatteryFIleSet(Battry_File_name,"Battary","dayPowerCharge",Battery_data->dayPowerCharge);
            if(BattaryPara.compare(Battery_data->dayPowerCharge) == 0)
                Battery_data->dayPowerCharge = "-1";
            break;
        case MONTH_POWER_CHARGE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","monthPowerCharge").toString();
            BatteryFIleSet(Battry_File_name,"Battary","monthPowerCharge",Battery_data->monthPowerCharge);
            if(BattaryPara.compare(Battery_data->monthPowerCharge) == 0)
                Battery_data->monthPowerCharge = "-1";
            break;
        case YEAR_POWER_CHARGE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","yearPowerCharge").toString();
            BatteryFIleSet(Battry_File_name,"Battary","yearPowerCharge",Battery_data->yearPowerCharge);
            if(BattaryPara.compare(Battery_data->yearPowerCharge) == 0)
                Battery_data->yearPowerCharge = "-1";
            break;
        case TOTAL_POWER_CHARGE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","totalPowerCharge").toString();
            BatteryFIleSet(Battry_File_name,"Battary","totalPowerCharge",Battery_data->totalPowerCharge);
            if(BattaryPara.compare(Battery_data->totalPowerCharge) == 0)
                Battery_data->totalPowerCharge = "-1";
            break;
        case BATTERY_VOLTAGE:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","batteryVoltage").toString();
            BatteryFIleSet(Battry_File_name,"Battary","batteryVoltage",Battery_data->batteryVoltage);
            if(BattaryPara.compare(Battery_data->batteryVoltage) == 0)
                Battery_data->batteryVoltage = "-1";
            break;
        case BATTERY_CURRENT:
            BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","batteryCurrent").toString();
            BatteryFIleSet(Battry_File_name,"Battary","batteryCurrent",Battery_data->batteryCurrent);
            if(BattaryPara.compare(Battery_data->batteryCurrent) == 0)
                Battery_data->batteryCurrent = "-1";
            break;
    }
}

void BatteryManager::BatterySendDataCommand_A(int num)
{
    Battery_data SendData;
    SendData.arry[0] = DEV_ID;
    SendData.arry[1] = FUNC_ONLY_READ;
    SendData.arry[4] = 0x00;
    SendData.arry[5] = 0x01;
    SendData.data_len = 6;
    switch(num){
       case DAY_OR_NIGHT:
            SendData.arry[1] = FUNC_ONLY_READ_2;
            SendData.arry[2] = (A2_DAY_OR_NIGHT & 0xff00) >> 8;
            SendData.arry[3] = quint8(A2_DAY_OR_NIGHT);
            break;
       case ARRAY_VOLTAGE:
            SendData.arry[2] = (A3_ARRAY_VOLTAGE & 0xff00) >> 8;
            SendData.arry[3] = quint8(A3_ARRAY_VOLTAGE);
            break;
       case ARRAY_CURRENT:
            SendData.arry[2] = (A4_ARRAY_CURRENT & 0xff00) >> 8;
            SendData.arry[3] = quint8(A4_ARRAY_CURRENT);
            break;
       case ARRAY_POWER:
            SendData.arry[2] = (A5_ARRAY_POWER_L & 0xff00) >> 8;
            SendData.arry[3] = quint8(A5_ARRAY_POWER_L);
            SendData.arry[4] = 0x00;
            SendData.arry[5] = 0x02;
            break;
       case VOLTAGE:
            SendData.arry[2] = (A7_VOLTAGE & 0xff00) >> 8;
            SendData.arry[3] = quint8(A7_VOLTAGE);
            break;
       case CURRENT:
            SendData.arry[2] = (A8_CURRENT & 0xff00) >> 8;
            SendData.arry[3] = quint8(A8_CURRENT);
            break;
       case POWER:
            SendData.arry[2] = (A9_POWER_L & 0xff00) >> 8;
            SendData.arry[3] = quint8(A9_POWER_L);
            SendData.arry[4] = 0x00;
            SendData.arry[5] = 0x02;
            break;
       case BATTERY_TEMPERATURE:
            SendData.arry[2] = (A11_BATTERY_TEMPERATURE & 0xff00) >> 8;
            SendData.arry[3] = quint8(A11_BATTERY_TEMPERATURE);
            break;
       case TEMPERATURE:
            SendData.arry[2] = (A12_TEMPERATURE & 0xff00) >> 8;
            SendData.arry[3] = quint8(A12_TEMPERATURE);
            break;
       case BATTERY_POWER:
            SendData.arry[2] = (A13_BATTERY_POWER & 0xff00) >> 8;
            SendData.arry[3] = quint8(A13_BATTERY_POWER);
            break;
       case BATTERY_VOLTAG_ELEVEL:
            SendData.arry[2] = (A14_BATTERY_VOLTAG_ELEVEL & 0xff00) >> 8;
            SendData.arry[3] = quint8(A14_BATTERY_VOLTAG_ELEVEL);
            break;
       case MAX_VOLTAGE:
            SendData.arry[2] = (A18_MAX_VOLTAGE & 0xff00) >> 8;
            SendData.arry[3] = quint8(A18_MAX_VOLTAGE);
            break;
       case MIN_VOLTAGE:
            SendData.arry[2] = (A19_MIN_VOLTAGE & 0xff00) >> 8;
            SendData.arry[3] = quint8(A19_MIN_VOLTAGE);
            break;
       case DAY_POWER_DISCHARGE:
            SendData.arry[2] = (A20_DAY_POWER_DISCHARGE_L & 0xff00) >> 8;
            SendData.arry[3] = quint8(A20_DAY_POWER_DISCHARGE_L);
            SendData.arry[4] = 0x00;
            SendData.arry[5] = 0x02;
            break;
       case MONTH_POWER_DISCHARGE:
            SendData.arry[2] = (A22_MONTH_POWER_DISCHARGE_L & 0xff00) >> 8;
            SendData.arry[3] = quint8(A22_MONTH_POWER_DISCHARGE_L);
            SendData.arry[4] = 0x00;
            SendData.arry[5] = 0x02;
            break;
       case YEAR_POWER_DISCHARGE:
            SendData.arry[2] = (A24_YEAR_POWER_DISCHARGE_L & 0xff00) >> 8;
            SendData.arry[3] = quint8(A24_YEAR_POWER_DISCHARGE_L);
            SendData.arry[4] = 0x00;
            SendData.arry[5] = 0x02;
            break;
       case TOTAL_POWER_DISCHARGE:
            SendData.arry[2] = (A26_TOTAL_POWER_DISCHARGE_L & 0xff00) >> 8;
            SendData.arry[3] = quint8(A26_TOTAL_POWER_DISCHARGE_L);
            SendData.arry[4] = 0x00;
            SendData.arry[5] = 0x02;
            break;
       case DAY_POWER_CHARGE:
            SendData.arry[2] = (A28_DAY_POWER_CHARGE_L & 0xff00) >> 8;
            SendData.arry[3] = quint8(A28_DAY_POWER_CHARGE_L);
            SendData.arry[4] = 0x00;
            SendData.arry[5] = 0x02;
            break;
       case MONTH_POWER_CHARGE:
            SendData.arry[2] = (A30_MONTH_POWER_CHARGE_L & 0xff00) >> 8;
            SendData.arry[3] = quint8(A30_MONTH_POWER_CHARGE_L);
            SendData.arry[4] = 0x00;
            SendData.arry[5] = 0x02;
            break;
       case YEAR_POWER_CHARGE:
            SendData.arry[2] = (A32_YEAR_POWER_CHARGE_L & 0xff00) >> 8;
            SendData.arry[3] = quint8(A32_YEAR_POWER_CHARGE_L);
            SendData.arry[4] = 0x00;
            SendData.arry[5] = 0x02;
            break;
       case TOTAL_POWER_CHARGE:
            SendData.arry[2] = (A34_TOTAL_POWER_CHARGE_L & 0xff00) >> 8;
            SendData.arry[3] = quint8(A34_TOTAL_POWER_CHARGE_L);
            SendData.arry[4] = 0x00;
            SendData.arry[5] = 0x02;
            break;
       case BATTERY_VOLTAGE:
            SendData.arry[2] = (A36_BATTERY_VOLTAGE & 0xff00) >> 8;
            SendData.arry[3] = quint8(A36_BATTERY_VOLTAGE);
            break;
       case BATTERY_CURRENT:
            SendData.arry[2] = (A37_BATTERY_CURRENT_L & 0xff00) >> 8;
            SendData.arry[3] = quint8(A37_BATTERY_CURRENT_L);
            SendData.arry[4] = 0x00;
            SendData.arry[5] = 0x02;
            break;
    }
    SendData.crc = CRC16_Modbus(SendData.arry);
    BatterySendData(&SendData);
}

void BatteryManager::BatterySendDataCommand_B(void)
{
    Battery_data SendData;
//蓄电池额定电流

 //负载额定电流

 //当前蓄电池电压等级

//蓄电池类型 - 放电限制电压 B4-B18 15
   // SendData.arry.resize();
//    SendData.arry[0] = DEV_ID;
//    SendData.arry[1] = 0x10;
//    SendData.arry[2] = 0x90;
//    SendData.arry[3] = 0x00;
//    SendData.arry[4] = 0x00;
//    SendData.arry[5] = 0x0F;
//    SendData.arry[6] = 0x1E;
//    SendData.arry[7] =
//系统额定电压等级

//手动控制条件下设定的开/关-充电深度 B20-B24

//电池充放电管理模式
}

void BatteryManager::BatterySendData(Battery_data *send_data)
{
    send_data->arry.append(SystemUtils::_u16ToQByteArray(send_data->crc));
    Battery_serial->writeData(send_data->arry,send_data->arry.size());
    qWarning()<<"send: "<<send_data->arry.toHex();
}

void BatteryManager::BatteryWaitData(Battery_data *recieve_data, int Comannd_num)
{
    quint16 value_int_L;
    quint16 value_int_H;
    quint32 value_int_32;
    float   value_float;
    if(Comannd_num == DAY_OR_NIGHT){
        value_int_L = recieve_data->arry[3];
        Battery_buffer.dayOrNight = QString::number(value_int_L);
    }else if(Comannd_num < ARRAY_POWER){
        value_int_L = quint16(recieve_data->arry[3] << 8) + recieve_data->arry[4];
        value_float = (float)value_int_L;
        value_float /= 100;
        switch(Comannd_num){
            case ARRAY_VOLTAGE:
                Battery_buffer.arrayVoltage = QString::number(value_float);
                break;
            case ARRAY_CURRENT:
                Battery_buffer.arrayCurrent = QString::number(value_float);
                break;
            case VOLTAGE:
                Battery_buffer.voltage = QString::number(value_float);
                break;
            case CURRENT:
                Battery_buffer.current = QString::number(value_float);
                break;
            case BATTERY_TEMPERATURE:
                Battery_buffer.batteryTemperature = QString::number(value_float);
                break;
            case TEMPERATURE:
                Battery_buffer.temperature = QString::number(value_float);
                break;
            case BATTERY_POWER:
                Battery_buffer.batteryPower = QString::number(value_float);
                break;
            case BATTERY_VOLTAG_ELEVEL:
                Battery_buffer.batteryVoltageLevel = QString::number(value_float);
                break;
            case MAX_VOLTAGE:
                Battery_buffer.maxVoltage = QString::number(value_float);
                break;
            case MIN_VOLTAGE:
                Battery_buffer.minVoltage = QString::number(value_float);
                break;
            case BATTERY_VOLTAGE:
                Battery_buffer.batteryVoltage = QString::number(value_float);
                break;
        }
    }else if(Comannd_num < BATTERY_PARA_NUM){
        value_int_L  = quint16(recieve_data->arry[3] << 8) + recieve_data->arry[4];
        value_int_H  = quint16(recieve_data->arry[5] << 8) + recieve_data->arry[6];
        value_int_32 = quint32(value_int_H << 16) + value_int_L;
        value_float = (float)value_int_32;
        value_float /= 100;
        switch(Comannd_num){
            case POWER:
                Battery_buffer.power = QString::number(value_float);
                break;
            case DAY_POWER_DISCHARGE:
                Battery_buffer.dayPowerDischarge = QString::number(value_float);
                break;
            case MONTH_POWER_DISCHARGE:
                Battery_buffer.monthPowerDischarge = QString::number(value_float);
                break;
            case YEAR_POWER_DISCHARGE:
                Battery_buffer.yearPowerDischarge = QString::number(value_float);
                break;
            case TOTAL_POWER_DISCHARGE:
                Battery_buffer.totalPowerDischarge = QString::number(value_float);
                break;
            case DAY_POWER_CHARGE:
                Battery_buffer.dayPowerCharge = QString::number(value_float);
                break;
            case MONTH_POWER_CHARGE:
                Battery_buffer.monthPowerCharge = QString::number(value_float);
                break;
            case YEAR_POWER_CHARGE:
                Battery_buffer.yearPowerCharge = QString::number(value_float);
                break;
            case TOTAL_POWER_CHARGE:
                Battery_buffer.totalPowerCharge = QString::number(value_float);
                break;
            case BATTERY_CURRENT:
                Battery_buffer.batteryCurrent = QString::number(value_float);
                break;
            case ARRAY_POWER:
                Battery_buffer.arrayPower = QString::number(value_float);
                break;

        }
    }
    Battery_mutex.unlock();
}

QVariant BatteryManager::BatteryFIleGet(QString file_name, QString NodeName, QString KeyName)
{
    Battery_file = new QSettings(file_name,QSettings::IniFormat);
    QVariant vaule = Battery_file->value(QString("/%1/%2").arg(NodeName).arg(KeyName));
    delete Battery_file;
    Battery_file = 0;
    return vaule;
}

quint16 BatteryManager::CRC16_Modbus(QByteArray &data)
{
    static const quint16 crc16Table[] =
            {
                0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
                0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
                0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
                0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
                0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
                0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
                0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
                0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
                0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
                0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
                0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
                0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
                0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
                0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
                0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
                0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
                0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
                0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
                0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
                0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
                0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
                0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
                0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
                0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
                0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
                0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
                0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
                0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
                0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
                0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
                0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
                0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
            };
        quint8 buf;
        quint16 crc16 = 0xFFFF;
        for ( quint16 i = 0; i < data.size(); ++i )
        {
            buf = data.at( i ) ^ crc16;
            crc16 >>= 8;
            crc16 ^= crc16Table[ buf ];
        }
        return crc16;
}

void BatteryManager::BatteryReadData()
{
    QByteArray data = Battery_serial->readData();
    Battery_data data_;
    if(!data.isEmpty()){
        if(DEV_ID == data[0]){
            data_.data_len = data[2];
            int i = 0;
            for(i = 0;i < data_.data_len + 3;++i){
                data_.arry.append(data[i]);
            }
            data_.crc = ((quint16)data[i + 1]<<8) + data[i];
            if(CRC16_Modbus(data_.arry) == data_.crc){
                BatteryWaitData(&data_, Current_command);
            }
        }
    }
}
