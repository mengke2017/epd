#include "battery/batterymanager.h"
#include "battery/batterylist.h"
#include <QThread>
#include <QSemaphore>
#include "customize.h"
#include "systemutils.h"

#define BATTERY_UART   "/dev/ttyS5"

QSemaphore SendSem(4);
quint8 OverTimeTab[WARING_COMMAND_NUM][2] =
{
    {DAY_OR_NIGHT,0},
    {ARRAY_VOLTAGE,0},
    {ARRAY_CURRENT,0},
    {VOLTAGE,0},
    {CURRENT,0},
    {BATTERY_TEMPERATURE,0},
    {TEMPERATURE,0},
    {BATTERY_POWER,0},
    {BATTERY_VOLTAGE,0},
    {ARRAY_POWER,0},
    {BATTERY_VOLTAG_ELEVEL,0},
    {MAX_VOLTAGE,0},//               11//
    {MIN_VOLTAGE,0},//              12//
    {DAY_POWER_DISCHARGE,0},//       13//
    {MONTH_POWER_DISCHARGE,0},//     14//
    {YEAR_POWER_DISCHARGE,0},//      15//
    {TOTAL_POWER_DISCHARGE,0},//     16//
    {DAY_POWER_CHARGE,0},//          17//
    {MONTH_POWER_CHARGE,0},//        18//
    {YEAR_POWER_CHARGE,0},//         19//
    {TOTAL_POWER_CHARGE,0},//        20//
    {POWER,0},//                     21//
    {BATTERY_CURRENT,0},//           22//
    {DEVICE_OVERHEAT,0},//         23//
    {BATTERY_STATUS,0},//        24//
    {CHARGE_STATUS,0},//                     25//
    {DISCHARGE_STATUS,0},//           26//
};
BatteryManger::BatteryManger(QObject *parent) :
    QObject(parent)
{
    SendSwitch  = OPEN;
    SendSwitch_Warn = CLOSE;
    SendSwitch_Write = CLOSE;

    for(quint16 i = 0; i < BATTERY_PARA_NUM; i++){
        OverTimeTab[i][1] = NORMAL;
    }

    list = new BatteryList(this);
    BatterySerial = new QSerialPort(this);
    BatterySerial->setPortName(BATTERY_UART);
    if(BatterySerial->open(QIODevice::ReadWrite))
    {
        //设置波特率
        BatterySerial->setBaudRate(115200);
        //设置数据位
        BatterySerial->setDataBits(QSerialPort::Data8);
        //设置校验位
        BatterySerial->setParity(QSerialPort::NoParity);
        //设置流控制
        BatterySerial->setFlowControl(QSerialPort::NoFlowControl);
        //设置停止位
        BatterySerial->setStopBits(QSerialPort::OneStop);
        connect(BatterySerial,SIGNAL(readyRead()),this,SLOT(DataSendHandle()), Qt::QueuedConnection);
    }
//    QString name = BATTERY_UART;
//    if(!BatterySerial->openPort(name,BAUD115200,DATA_8,PAR_NONE,STOP_1,FLOW_OFF,TIME_OUT)) {
//        qWarning("open battery error!");
//    }

//    connect(BatterySerial, SIGNAL(hasdata()), this, SLOT(DataSendHandle()));


//    connect(BatterySerial, SIGNAL(BatteryHasData()), this, SLOT(DataSendHandle()));

    Timer_Basic = new QTimer(this);
    connect(Timer_Basic, SIGNAL(timeout()), this, SLOT(Basic_TimeOut()));
    Timer_Basic->start(TIMER_MSEC);

    QThread* SendThead = new QThread(this);
    Timer_Send = new QTimer();
    Timer_Send->setInterval(10*TIMER_MSEC);
    Timer_Send->moveToThread(SendThead);
    connect(Timer_Send, SIGNAL(timeout()), this, SLOT(Send_TimeOut()), Qt::QueuedConnection);
    connect(SendThead, SIGNAL(started()), Timer_Send, SLOT(start()));
    SendThead->start(QThread::TimeCriticalPriority);

    connect(this,SIGNAL(RecieveOver()),this,SLOT(DataSendHandle()));

    CmdIndex = DAY_OR_NIGHT;
    Update_SendPack(CmdIndex);
}

BatteryManger *BatteryManger::getInstance()
{
    static BatteryManger Instance;
    return &Instance;
}

void BatteryManger::SendPack(BatterySyspam& data)
{
    SendSem.acquire(2);
    data.arry.append(SystemUtils::_u16ToQByteArray(data.crc));
    if(BatterySerial->isOpen())
        BatterySerial->write(data.arry/*,data.arry.size()*/);

 //   qWarning()<<"send: "<<data.arry.toHex()<<"index:"<<QString::number(data.cmd);
    OverTimeTab[data.cmd][1] = NORMAL;
    ReplyTimer = REPLY_OVERTIME;
    LastAOrB   = A_COMMAND;
    SendSem.release(1);
}

void BatteryManger::Update_SendPack(quint16 index)
{
    Send_Pack.arry.clear();
    Send_Pack.cmd = index;
    Send_Pack.arry[0] = DEV_ID;
    Send_Pack.arry[1] = FUNC_ONLY_READ;
    Send_Pack.arry[4] = 0x00;
    Send_Pack.arry[5] = 0x01;
    switch(index){
        case DAY_OR_NIGHT:
            Send_Pack.arry[1] = FUNC_ONLY_READ_2;
            Send_Pack.arry[2] = (A2_DAY_OR_NIGHT & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A2_DAY_OR_NIGHT);
            break;
        case ARRAY_VOLTAGE:
            Send_Pack.arry[2] = (A3_ARRAY_VOLTAGE & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A3_ARRAY_VOLTAGE);
            break;
        case ARRAY_CURRENT:
            Send_Pack.arry[2] = (A4_ARRAY_CURRENT & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A4_ARRAY_CURRENT);
            break;
        case ARRAY_POWER:
            Send_Pack.arry[2] = (A5_ARRAY_POWER_L & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A5_ARRAY_POWER_L);
            Send_Pack.arry[4] = 0x00;
            Send_Pack.arry[5] = 0x02;
            break;
        case VOLTAGE:
            Send_Pack.arry[2] = (A7_VOLTAGE & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A7_VOLTAGE);
            break;
        case CURRENT:
            Send_Pack.arry[2] = (A8_CURRENT & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A8_CURRENT);
            break;
        case POWER:
            Send_Pack.arry[2] = (A9_POWER_L & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A9_POWER_L);
            Send_Pack.arry[4] = 0x00;
            Send_Pack.arry[5] = 0x02;
            break;
        case BATTERY_TEMPERATURE:
            Send_Pack.arry[2] = (A11_BATTERY_TEMPERATURE & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A11_BATTERY_TEMPERATURE);
            break;
        case TEMPERATURE:
            Send_Pack.arry[2] = (A12_TEMPERATURE & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A12_TEMPERATURE);
            break;
        case BATTERY_POWER:
            Send_Pack.arry[2] = (A13_BATTERY_POWER & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A13_BATTERY_POWER);
            break;
        case BATTERY_VOLTAG_ELEVEL:
            Send_Pack.arry[2] = (A14_BATTERY_VOLTAG_ELEVEL & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A14_BATTERY_VOLTAG_ELEVEL);
            break;
        case MAX_VOLTAGE:
             Send_Pack.arry[2] = (A18_MAX_VOLTAGE & 0xff00) >> 8;
             Send_Pack.arry[3] = quint8(A18_MAX_VOLTAGE);
             break;
        case MIN_VOLTAGE:
            Send_Pack.arry[2] = (A19_MIN_VOLTAGE & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A19_MIN_VOLTAGE);
            break;
        case DAY_POWER_DISCHARGE:
            Send_Pack.arry[2] = (A20_DAY_POWER_DISCHARGE_L & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A20_DAY_POWER_DISCHARGE_L);
            Send_Pack.arry[4] = 0x00;
            Send_Pack.arry[5] = 0x02;
            break;
        case MONTH_POWER_DISCHARGE:
            Send_Pack.arry[2] = (A22_MONTH_POWER_DISCHARGE_L & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A22_MONTH_POWER_DISCHARGE_L);
            Send_Pack.arry[4] = 0x00;
            Send_Pack.arry[5] = 0x02;
            break;
        case YEAR_POWER_DISCHARGE:
            Send_Pack.arry[2] = (A24_YEAR_POWER_DISCHARGE_L & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A24_YEAR_POWER_DISCHARGE_L);
            Send_Pack.arry[4] = 0x00;
            Send_Pack.arry[5] = 0x02;
            break;
        case TOTAL_POWER_DISCHARGE:
            Send_Pack.arry[2] = (A26_TOTAL_POWER_DISCHARGE_L & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A26_TOTAL_POWER_DISCHARGE_L);
            Send_Pack.arry[4] = 0x00;
            Send_Pack.arry[5] = 0x02;
                break;
        case DAY_POWER_CHARGE:
            Send_Pack.arry[2] = (A28_DAY_POWER_CHARGE_L & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A28_DAY_POWER_CHARGE_L);
            Send_Pack.arry[4] = 0x00;
            Send_Pack.arry[5] = 0x02;
            break;
        case MONTH_POWER_CHARGE:
            Send_Pack.arry[2] = (A30_MONTH_POWER_CHARGE_L & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A30_MONTH_POWER_CHARGE_L);
            Send_Pack.arry[4] = 0x00;
            Send_Pack.arry[5] = 0x02;
            break;
        case YEAR_POWER_CHARGE:
            Send_Pack.arry[2] = (A32_YEAR_POWER_CHARGE_L & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A32_YEAR_POWER_CHARGE_L);
            Send_Pack.arry[4] = 0x00;
            Send_Pack.arry[5] = 0x02;
            break;
        case TOTAL_POWER_CHARGE:
            Send_Pack.arry[2] = (A34_TOTAL_POWER_CHARGE_L & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A34_TOTAL_POWER_CHARGE_L);
            Send_Pack.arry[4] = 0x00;
            Send_Pack.arry[5] = 0x02;
            break;
        case BATTERY_VOLTAGE:
            Send_Pack.arry[2] = (A36_BATTERY_VOLTAGE & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A36_BATTERY_VOLTAGE);
            break;
        case BATTERY_CURRENT:
            Send_Pack.arry[2] = (A37_BATTERY_CURRENT_L & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A37_BATTERY_CURRENT_L);
            Send_Pack.arry[4] = 0x00;
            Send_Pack.arry[5] = 0x02;
            break;
        case DEVICE_OVERHEAT:
            Send_Pack.arry[1] = FUNC_ONLY_READ_2;
            Send_Pack.arry[2] = (A1_DEVICE_OVERHEAT & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A1_DEVICE_OVERHEAT);
                break;
        case BATTERY_STATUS:
            Send_Pack.arry[2] = (A15_BATTERY_STATUS & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A15_BATTERY_STATUS);
            break;
        case CHARGE_STATUS:
            Send_Pack.arry[2] = (A16_CHARGE_STATUS & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A16_CHARGE_STATUS);
            break;
        case DISCHARGE_STATUS:
            Send_Pack.arry[2] = (A17_DISCHARGE_STATUS & 0xff00) >> 8;
            Send_Pack.arry[3] = quint8(A17_DISCHARGE_STATUS);
            break;
    }
    Send_Pack.crc = CRC16_Modbus(Send_Pack.arry);
}

quint16 BatteryManger::CRC16_Modbus(QByteArray &data)
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
            crc16 ^= crc16Table[buf];
        }
    return crc16;
}

void BatteryManger::DataRecieveHandle()
{
    BatterySyspam *data;
    float   value_float = 0;
    quint8  value1 = 0;
    while((data = list->get()) != NULL){
        QString value;
        if(data->cmd == DAY_OR_NIGHT){
            if(NO_REPLY_DATA != data->arry[0]){
                value1 = (quint8)data->arry[3];
                Battery_buffer.dayOrNight = QString::number(value1);
            }else{
                Battery_buffer.dayOrNight = "-1";
            }
            //qDebug()<<"Battery_buffer.dayOrNight = " << Battery_buffer.dayOrNight;
        }else if(data->cmd < ARRAY_POWER){
            if(NO_REPLY_DATA != data->arry[0]){
                value_float = ReplyDataCalcul(data->arry,data->cmd);
                value = QString::number(value_float);
            }else{
                value = "-1";
            }
            switch(data->cmd){
                case ARRAY_VOLTAGE:
                    Battery_buffer.arrayVoltage = value;
                    //qDebug()<<"Battery_buffer.arrayVoltage" << QString::number(value_float);
                    break;
                case ARRAY_CURRENT:
                    Battery_buffer.arrayCurrent = value;
                   // qDebug()<<"Battery_buffer.arrayCurrent" << QString::number(value_float);
                    break;
                case VOLTAGE:
                    Battery_buffer.voltage = value;
                    //qDebug()<<"Battery_buffer.voltage" << QString::number(value_float);
                    break;
                case CURRENT:
                    Battery_buffer.current = value;
                    //qDebug()<<"Battery_buffer.current" << QString::number(value_float);
                    break;
                case BATTERY_TEMPERATURE:
                    Battery_buffer.batteryTemperature = value;
                    //qDebug()<<"Battery_buffer.batteryTemperature" << QString::number(value_float);
                    break;
                case TEMPERATURE:
                    Battery_buffer.temperature = value;
                    //qDebug()<<"Battery_buffer.temperature" << QString::number(value_float);
                    break;
                case BATTERY_POWER:
                    Battery_buffer.batteryPower = value;
                    //qDebug()<<"Battery_buffer.batteryPower" << QString::number(value_float);
                    break;
                case BATTERY_VOLTAG_ELEVEL:
                    Battery_buffer.batteryVoltageLevel = value;
                    //qDebug()<<"Battery_buffer.batteryVoltageLevel" << QString::number(value_float);
                    break;
                case MAX_VOLTAGE:
                    Battery_buffer.maxVoltage = value;
                    //qDebug()<<"Battery_buffer.maxVoltage" << QString::number(value_float);
                    break;
                case MIN_VOLTAGE:
                    Battery_buffer.minVoltage = value;
                    //qDebug()<<"Battery_buffer.minVoltage" << QString::number(value_float);
                    break;
                case BATTERY_VOLTAGE:
                    Battery_buffer.batteryVoltage = value;
                    //qDebug()<<"Battery_buffer.batteryVoltage" << QString::number(value_float);
                    break;
            }
           // qDebug()<<"vaule_float:"<<value;
        }else if(data->cmd < BATTERY_PARA_NUM){
            if(NO_REPLY_DATA != data->arry[0]){
                value_float = ReplyDataCalcul(data->arry,data->cmd);
                value = QString::number(value_float);
            }else{
                value = "-1";
            }
            //qDebug()<<"value_float_L_H:"<<value;
            switch(data->cmd){
                case POWER:
                    Battery_buffer.power = value;
                    //qDebug()<<"Battery_buffer.power" << QString::number(value_float);
                    break;
                case DAY_POWER_DISCHARGE:
                    Battery_buffer.dayPowerDischarge = value;
                    //qDebug()<<"Battery_buffer.dayPowerDischarge" << QString::number(value_float);
                    break;
                case MONTH_POWER_DISCHARGE:
                    Battery_buffer.monthPowerDischarge = value;
                    //qDebug()<<"Battery_buffer.monthPowerDischarge" << QString::number(value_float);
                    break;
                case YEAR_POWER_DISCHARGE:
                    Battery_buffer.yearPowerDischarge = value;
                    //qDebug()<<"Battery_buffer.yearPowerDischarge" << QString::number(value_float);
                    break;
                case TOTAL_POWER_DISCHARGE:
                    Battery_buffer.totalPowerDischarge = value;
                    //qDebug()<<"Battery_buffer.totalPowerDischarge" << QString::number(value_float);
                    break;
                case DAY_POWER_CHARGE:
                    Battery_buffer.dayPowerCharge = value;
                    //qDebug()<<"Battery_buffer.dayPowerCharge" << QString::number(value_float);
                    break;
                case MONTH_POWER_CHARGE:
                    Battery_buffer.monthPowerCharge = value;
                    //qDebug()<<"Battery_buffer.monthPowerCharge" << QString::number(value_float);
                    break;
                case YEAR_POWER_CHARGE:
                    Battery_buffer.yearPowerCharge = value;
                    //qDebug()<<"Battery_buffer.yearPowerCharge" << QString::number(value_float);
                    break;
                case TOTAL_POWER_CHARGE:
                    Battery_buffer.totalPowerCharge = value;
                    //qDebug()<<"Battery_buffer.totalPowerCharge" << QString::number(value_float);
                    break;
                case BATTERY_CURRENT:
                    Battery_buffer.batteryCurrent = value;
                    //qDebug()<<"Battery_buffer.batteryCurrent" << QString::number(value_float);
                    break;
                case ARRAY_POWER:
                    Battery_buffer.arrayPower = value;
                    //qDebug()<<"Battery_buffer.arrayPower" << QString::number(value_float);
                    break;
            }
        }
        list->remove();
    }
    BatteryIntAndStringHandle(Battery_buffer);

}
QByteArray BatteryManger::ReplyDataCal(const QByteArray &Data, quint16 Cmd)
{
    QByteArray data;
    if(Cmd == DEVICE_OVERHEAT){
        data.resize(1);
        data[0] = Data[3];
    }else{
        data.resize(2);
        data[0] = Data[3];
        data[1] = Data[4];
    }
    return data;
}
void BatteryManger::DataRecieveHandleWarnCommand(void)
{
    BatterySyspam *data;
    QByteArray   value;
    quint16  value_int = 0;
    QString alarm;
    while((data = list->get()) != NULL){
        if(NO_REPLY_DATA != data->arry[0]){
            value = ReplyDataCal(data->arry,data->cmd);
            qDebug()<<"Warin Value ======" << value.toHex();
        }
        switch(data->cmd){
            case DEVICE_OVERHEAT:
                if((quint8)value[0] != 0x00){
                    qDebug()<<"Over Heat!";
                    BatteryWarn_buffer.DeviceOverHeat = ((0x14) << 16) + (quint8)value[0];
                    alarm = QString::number(BatteryWarn_buffer.DeviceOverHeat);
                    emit to_alarm(alarm);
                }
                break;
            case BATTERY_STATUS:
                value_int = (quint16)(((quint8)value[0] << 8) + (quint8)value[1]);
                if(value_int != 0x00){
                    qDebug()<<"BATTERY_STATUS ERROR!";
                    BatteryWarn_buffer.BatteryStatus = ((0x15) << 16) + value_int;
                    alarm = QString::number(BatteryWarn_buffer.BatteryStatus);
                    emit to_alarm(alarm);
                }
                break;
            case CHARGE_STATUS:
                value_int = (quint16)(((quint8)value[0] << 8) + (quint8)value[1]);
                if(value_int != 0x01){
                    qDebug()<<"CHARGE_STATUS ERROR!";
                    BatteryWarn_buffer.ChargeStatus = ((0x16) << 16) + value_int;
                    alarm = QString::number(BatteryWarn_buffer.ChargeStatus);
                    emit to_alarm(alarm);
                }
                break;
            case DISCHARGE_STATUS:
                value_int = (quint16)(((quint8)value[0] << 8) + (quint8)value[1]);
                if(value_int != 0x01){
                    qDebug()<<"DISCHARGE_STATUS ERROR!";
                    BatteryWarn_buffer.DischargeStatus = ((0x17) << 16) + value_int;
                    alarm = QString::number(BatteryWarn_buffer.DischargeStatus);
                    emit to_alarm(alarm);
                }
                break;
        }
        list->remove();
    }
}
void BatteryManger::BatteryIntAndStringHandle(BatteryPara &Battery_data)
{
    QString BattaryPara;
    QString Battry_File_name = CONFIG_FILE;
    static bool frist = false;
    for(int num = DAY_OR_NIGHT; num < BATTERY_PARA_NUM;num++){
        switch(num){
            case DAY_OR_NIGHT:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","dayOrNight_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","dayOrNight_last",Battery_data.dayOrNight);
                if(BattaryPara.compare(Battery_data.dayOrNight) == 0 && frist)
                    Battery_data.dayOrNight = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","dayOrNight",Battery_data.dayOrNight);
                break;
            case ARRAY_VOLTAGE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","arrayVoltage_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","arrayVoltage_last",Battery_data.arrayVoltage);
                if(BattaryPara.compare(Battery_data.arrayVoltage) == 0 && frist)
                    Battery_data.arrayVoltage = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","arrayVoltage",Battery_data.arrayVoltage);
                break;
            case ARRAY_CURRENT:
                 BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","arrayCurrent_last").toString();
                 BatteryFIleSet(Battry_File_name,"Battary","arrayCurrent_last",Battery_data.arrayCurrent);
                 if(BattaryPara.compare(Battery_data.arrayCurrent) == 0 && frist)
                     Battery_data.arrayCurrent = "-1";
                 BatteryFIleSet(Battry_File_name,"Battary","arrayCurrent",Battery_data.arrayCurrent);
                 break;
            case ARRAY_POWER:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","arrayPower_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","arrayPower_last",Battery_data.arrayPower);
                if(BattaryPara.compare(Battery_data.arrayPower) == 0 && frist)
                    Battery_data.arrayPower = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","arrayPower",Battery_data.arrayPower);
                break;
            case VOLTAGE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","voltage_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","voltage_last",Battery_data.voltage);
                if(BattaryPara.compare(Battery_data.voltage) == 0 && frist)
                    Battery_data.voltage = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","voltage",Battery_data.voltage);
                break;
            case CURRENT:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","current_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","current_last",Battery_data.current);
                if(BattaryPara.compare(Battery_data.current) == 0 && frist)
                    Battery_data.current = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","current",Battery_data.current);
                break;
            case POWER:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","power_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","power_last",Battery_data.power);
                if(BattaryPara.compare(Battery_data.power) == 0 && frist)
                    Battery_data.power = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","power",Battery_data.power);
                break;
            case BATTERY_TEMPERATURE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","batteryTemperature_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","batteryTemperature_last",Battery_data.batteryTemperature);
                if(BattaryPara.compare(Battery_data.batteryTemperature) == 0 && frist)
                    Battery_data.batteryTemperature = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","batteryTemperature",Battery_data.batteryTemperature);
                break;
            case TEMPERATURE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","temperature_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","temperature_last",Battery_data.temperature);
                if(BattaryPara.compare(Battery_data.temperature) == 0 && frist)
                    Battery_data.temperature = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","temperature",Battery_data.temperature);
                break;
            case BATTERY_POWER:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","batteryPower_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","batteryPower_last",Battery_data.batteryPower);
                if(BattaryPara.compare(Battery_data.batteryPower) == 0 && frist)
                    Battery_data.batteryPower = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","batteryPower",Battery_data.batteryPower);
                break;
            case BATTERY_VOLTAG_ELEVEL:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","batteryVoltageLevel_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","batteryVoltageLevel_last",Battery_data.batteryVoltageLevel);
                if(BattaryPara.compare(Battery_data.batteryVoltageLevel) == 0 && frist)
                    Battery_data.batteryVoltageLevel = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","batteryVoltageLevel",Battery_data.batteryVoltageLevel);
                break;
            case MAX_VOLTAGE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","maxVoltage_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","maxVoltage_last",Battery_data.maxVoltage);
                if(BattaryPara.compare(Battery_data.maxVoltage) == 0 && frist)
                    Battery_data.maxVoltage = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","maxVoltage",Battery_data.maxVoltage);
                break;
            case MIN_VOLTAGE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","minVoltage_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","minVoltage_last",Battery_data.minVoltage);
                if(BattaryPara.compare(Battery_data.minVoltage) == 0 && frist)
                    Battery_data.minVoltage = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","minVoltage",Battery_data.minVoltage);
                break;
            case DAY_POWER_DISCHARGE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","dayPowerDischarge_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","dayPowerDischarge_last",Battery_data.dayPowerDischarge);
                if(BattaryPara.compare(Battery_data.dayPowerDischarge) == 0 && frist)
                    Battery_data.dayPowerDischarge = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","dayPowerDischarge",Battery_data.dayPowerDischarge);
                break;
            case MONTH_POWER_DISCHARGE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","monthPowerDischarge_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","monthPowerDischarge_last",Battery_data.monthPowerDischarge);
                if(BattaryPara.compare(Battery_data.monthPowerDischarge) == 0 && frist)
                    Battery_data.monthPowerDischarge = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","monthPowerDischarge",Battery_data.monthPowerDischarge);
                break;
            case YEAR_POWER_DISCHARGE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","yearPowerDischarge_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","yearPowerDischarge_last",Battery_data.yearPowerDischarge);
                if(BattaryPara.compare(Battery_data.yearPowerDischarge) == 0 && frist)
                    Battery_data.yearPowerDischarge = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","yearPowerDischarge",Battery_data.yearPowerDischarge);
                break;
            case TOTAL_POWER_DISCHARGE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","totalPowerDischarge_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","totalPowerDischarge_last",Battery_data.totalPowerDischarge);
                if(BattaryPara.compare(Battery_data.totalPowerDischarge) == 0 && frist)
                    Battery_data.totalPowerDischarge = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","totalPowerDischarge",Battery_data.totalPowerDischarge);
                break;
            case DAY_POWER_CHARGE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","dayPowerCharge_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","dayPowerCharge_last",Battery_data.dayPowerCharge);
                if(BattaryPara.compare(Battery_data.dayPowerCharge) == 0 && frist)
                    Battery_data.dayPowerCharge = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","dayPowerCharge",Battery_data.dayPowerCharge);
                break;
            case MONTH_POWER_CHARGE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","monthPowerCharge_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","monthPowerCharge_last",Battery_data.monthPowerCharge);
                if(BattaryPara.compare(Battery_data.monthPowerCharge) == 0 && frist)
                    Battery_data.monthPowerCharge = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","monthPowerCharge",Battery_data.monthPowerCharge);
                break;
            case YEAR_POWER_CHARGE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","yearPowerCharge_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","yearPowerCharge_last",Battery_data.yearPowerCharge);
                if(BattaryPara.compare(Battery_data.yearPowerCharge) == 0 && frist)
                    Battery_data.yearPowerCharge = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","yearPowerCharge",Battery_data.yearPowerCharge);
                break;
            case TOTAL_POWER_CHARGE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","totalPowerCharge_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","totalPowerCharge_last",Battery_data.totalPowerCharge);
                if(BattaryPara.compare(Battery_data.totalPowerCharge) == 0 && frist)
                    Battery_data.totalPowerCharge = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","totalPowerCharge",Battery_data.totalPowerCharge);
                break;
            case BATTERY_VOLTAGE:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","batteryVoltage_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","batteryVoltage_last",Battery_data.batteryVoltage);
                if(BattaryPara.compare(Battery_data.batteryVoltage) == 0 && frist){
                    Battery_data.batteryVoltage = "-1";
                    //qDebug()<<"same value!!!!"<<Battery_data.batteryVoltage;
                }
                BatteryFIleSet(Battry_File_name,"Battary","batteryVoltage",Battery_data.batteryVoltage);
                break;
            case BATTERY_CURRENT:
                BattaryPara = BatteryFIleGet(Battry_File_name,"Battary","batteryCurrent_last").toString();
                BatteryFIleSet(Battry_File_name,"Battary","batteryCurrent_last",Battery_data.batteryCurrent);
                if(BattaryPara.compare(Battery_data.batteryCurrent) == 0 && frist)
                    Battery_data.batteryCurrent = "-1";
                BatteryFIleSet(Battry_File_name,"Battary","batteryCurrent",Battery_data.batteryCurrent);
                break;
        }
    }
    frist = true;
}

QVariant BatteryManger::BatteryFIleGet(QString file_name, QString NodeName, QString KeyName)
{
    Battery_file = new QSettings(file_name,QSettings::IniFormat);
    QVariant vaule = Battery_file->value(QString("/%1/%2").arg(NodeName).arg(KeyName));
    delete Battery_file;
    Battery_file = 0;
    return vaule;
}

void BatteryManger::BatteryFIleSet(QString file_name, QString NodeName, QString KeyName, QVariant vaule)
{
    Battery_file = new QSettings(file_name,QSettings::IniFormat);
    Battery_file->beginGroup(NodeName);
    Battery_file->setValue(KeyName,vaule);
    Battery_file->endGroup();
    delete Battery_file;
    Battery_file = 0;
}

float BatteryManger::ReplyDataCalcul(const QByteArray& Data, quint16 Cmd)
{
   qint16 value_int_L;
   qint16 value_int_H;
   qint32 value_int_32;
   float   value_float = -1;
   if(Cmd < ARRAY_POWER){
       for(int i = 0;i < Data[2];i++){
           value_int_L = quint16((quint8)Data[3] << 8) + (quint8)Data[4];
           value_float = (float)value_int_L;
           if(Cmd != BATTERY_POWER)
               value_float /= 100;
           return   value_float;
       }
   }else if(Cmd < BATTERY_PARA_NUM){
       if(Cmd == BATTERY_CURRENT){
           value_int_L  = qint16((quint8)Data[3] << 8) + (quint8)Data[4];
           if(value_int_L&0x8000) {
               value_int_L &= 0x7fff;
               value_float = value_int_L-32768;
           } else {
               value_float = value_int_L;
           }
  //         qDebug() << "BATTERY_CURRENT = "<<QString::number(value_float);
       } else {
           value_int_L  = qint16((quint8)Data[3] << 8) + (quint8)Data[4];
           value_int_H  = qint16((quint8)Data[5] << 8) + (quint8)Data[6];
           value_int_32 = qint32(value_int_H << 16) + value_int_L;
           value_float = (float)value_int_32;
       }
       value_float /= 100;
       return value_float;
   }
   return value_float;
}

void BatteryManger::ReadBatteryParam(quint16 addr, quint16 ParamNum)
{
    BatterySyspam ReadBattery;
    Timer_Send->stop();//在发送B指令时把A指令的发送关闭
    SendSem.acquire(2);
    ReadBattery.arry[0]  = DEV_ID;
    ReadBattery.arry[1]  = FUNC_READ;
    ReadBattery.arry[2]  = (addr & 0xff00) >> 8;
    ReadBattery.arry[3]  = quint8(addr);
    ReadBattery.arry[4]  = (ParamNum & 0xff00) >> 8;
    ReadBattery.arry[5]  = quint8(ParamNum);
    ReadBattery.crc      = CRC16_Modbus(ReadBattery.arry);

    //Last_Comannd         = addr;
    LastAOrB             = B_COMMAND;
    Cmd_OverTime         = NORMAL;
    ReplyTimer           = REPLY_OVERTIME;
    ReadBattery.arry.append(SystemUtils::_u16ToQByteArray(ReadBattery.crc));
    if(BatterySerial->isOpen())
        BatterySerial->write(ReadBattery.arry);
    SendSem.release(1);
}


void BatteryManger::Basic_TimeOut()
{
    static quint16 HeartBeatTimer = 5000;
    static quint16 Warning_Timer = 5000;
    if(SendSwitch == CLOSE){
        if(HeartBeatTimer > 0){
            HeartBeatTimer--;
        }else{
            HeartBeatTimer = 5000;
            SendSwitch  = OPEN;
        }
    }
    if(SendSwitch == CLOSE && SendSwitch_Warn == CLOSE){
        if(Warning_Timer > 0) {
            Warning_Timer--;
        } else{
            Warning_Timer = 5000;
            SendSwitch_Warn  = OPEN;
            CmdIndex = DEVICE_OVERHEAT;
            Update_SendPack(CmdIndex);
        }
    }
    if(SendSem.available() == 3 || SendSem.available() == 2){
        if(ReplyTimer > 0){
            ReplyTimer--;
        }else{
            if(LastAOrB == B_COMMAND)
                Cmd_OverTime = OVER_TIME;
            else{
                if(CmdIndex < BATTERY_PARA_NUM)
                    OverTimeTab[CmdIndex][1] = OVER_TIME;
            }
            emit RecieveOver();
        }
    }
}

void BatteryManger::Send_TimeOut()
{
    if(SendSwitch == OPEN || SendSwitch_Warn == OPEN){
        if(SendSem.available() == 4){
            SendPack(Send_Pack);
        }
    } else if(SendSwitch_Write == OPEN) {
        if(SendSem.available() == 4) {
            WriteBatteryParam(send_write_pack.addr,send_write_pack.addr_num,
                              send_write_pack.data);

        }
    }
}
void BatteryManger::WriteBatteryParam(quint16 addr, quint16 addr_num, const QByteArray& data)
{
    BatterySyspam WriteSyspam;
    SendSem.acquire(2);
    WriteSyspam.arry[0] = DEV_ID;
    WriteSyspam.arry[1] = FUNC_WRITE;
    WriteSyspam.arry[2] = (addr & 0xff00) >> 8;
    WriteSyspam.arry[3] = quint8(addr);
    WriteSyspam.arry[4] = (addr_num & 0xff00) >> 8;
    WriteSyspam.arry[5] = quint8(addr_num);
    WriteSyspam.arry[6] = quint8(data.size());

    for(int i = 7, j = 0;i < data.size() + 7;i++){
        WriteSyspam.arry[i] = data[j++];
    }
    WriteSyspam.crc = CRC16_Modbus(WriteSyspam.arry);
    WriteSyspam.arry.append(SystemUtils::_u16ToQByteArray(WriteSyspam.crc));
 //   qDebug()<<"write command = "<<WriteSyspam.arry.toHex();
    BatterySerial->write(WriteSyspam.arry);

    //Last_Comannd         = addr;
    LastAOrB             = B_COMMAND;
    Cmd_OverTime         = NORMAL;
    ReplyTimer           = REPLY_OVERTIME;
    SendSem.release(1);
}
void BatteryManger::DataSendHandle()
{
    BatterySyspam data_;
    if(LastAOrB == A_COMMAND){
        LastAOrB = B_COMMAND;
        if(OverTimeTab[CmdIndex][1] == OVER_TIME){
           // qDebug()<<"recieve over time!";
            //qWarning("CmdIndex: %d", CmdIndex);
            data_.cmd = CmdIndex;
            data_.arry.append(NO_REPLY_DATA);
            data_.crc = 0;
            list->add(data_);
        }else if(OverTimeTab[CmdIndex][1] == NORMAL){
            /*提取从串口接收的数据*/
            QByteArray data = BatterySerial->readAll();
            if(!data.isEmpty()){
                if(data.at(0) != DEV_ID)  //太阳能电池管理  0x7e既作为设备ID也可以作为帧头
                    return;
            } else {
                return;
            }
  //          qDebug() << "@@data@@:" <<data.toHex();
            if(!data.isEmpty()){
                if(DEV_ID == data[0]){
                    data_.cmd = CmdIndex;
                    quint16 data_len = data[2];
                    quint16 i = 0;
                    for(i = 0;i < data_len + 3;++i){
                        data_.arry.append(data[i]);
                    }
                    data_.crc = ((quint16)data[i + 1]<<8) + data[i];
                    if(CRC16_Modbus(data_.arry) == data_.crc){
                        list->add(data_);

                    }
                }
            }

        }
        CmdIndex++;
        if(CmdIndex == BATTERY_PARA_NUM) {
            SendSwitch = CLOSE;
            CmdIndex = DAY_OR_NIGHT;
            DataRecieveHandle();
            qDebug()<<"send all the data and handle the data!";
        } else if(CmdIndex == WARING_COMMAND_NUM) {
            SendSwitch_Warn = CLOSE;
            CmdIndex = DAY_OR_NIGHT;
            DataRecieveHandleWarnCommand();
        }
        Update_SendPack(CmdIndex);
        SendSem.release(1);
    }else if(LastAOrB == B_COMMAND){
        LastAOrB = A_COMMAND;
        SendSwitch_Write = CLOSE;
        if(Cmd_OverTime == NORMAL){
            QByteArray data = BatterySerial->readAll();
            if(!data.isEmpty()){//协议处理
                if(data[1] = FUNC_READ){//读命令

                }else if(data[1] = FUNC_WRITE){//写命令

                }
             //   qDebug()<<"write command data"<<data.toHex();
            }
        }else{
          //  qDebug()<<"超时啦!!!!!!!!!!!!";
        }
        SendSem.release(1);
    }
}
void BatteryManger::SetBatteryTime(const QString &data)
{
    if(SendSwitch_Write == OPEN)
        return;
    QString year = data.mid(2,2);
    QString month = data.mid(5,2);
    QString day = data.mid(8,2);

    QString hour = data.mid(11,2);
    QString min = data.mid(14,2);
    QString sec = data.mid(17,2);

    send_write_pack.addr = 0x9013;
    send_write_pack.addr_num = 3;
    send_write_pack.data.resize(6);

    send_write_pack.data[0] = min.toInt();
    send_write_pack.data[1] = sec.toInt();
    send_write_pack.data[2] = day.toInt();
    send_write_pack.data[3] = hour.toInt();
    send_write_pack.data[4] = year.toInt();
    send_write_pack.data[5] = month.toInt();
    SendSwitch_Write = OPEN;
}
