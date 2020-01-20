#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include<QWidget>
#include<QByteArray>
#include<QMutex>
#include<QSocketNotifier>
//#include <QSocketNotifier>
//#include <linux/fs.h>
//#include<fcntl.h>
//#include<errno.h>
//#include<termio.h>
//#include<sys/ioctl.h>
//#include<sys/stat.h>
//#include<sys/types.h>
//#include<stdlib.h>
//#include<unistd.h>
//#include <stdio.h>
//#include<QTimer>
//#include<QMutex>
//#include<QWidget>
#define TIME_OUT 10
enum BaudRateType
{
    BAUD50,                //POSIX ONLY
    BAUD75,                //POSIX ONLY
    BAUD110,
    BAUD134,               //POSIX ONLY
    BAUD150,               //POSIX ONLY
    BAUD200,               //POSIX ONLY
    BAUD300,
    BAUD600,
    BAUD1200,
    BAUD1800,              //POSIX ONLY
    BAUD2400,
    BAUD4800,
    BAUD9600,
    BAUD14400,             //WINDOWS ONLY
    BAUD19200,
    BAUD38400,
    BAUD56000,             //WINDOWS ONLY
    BAUD57600,
    BAUD76800,             //POSIX ONLY
    BAUD115200,
    BAUD128000,            //WINDOWS ONLY
    BAUD256000             //WINDOWS ONLY
};

enum DataBitsType
{
    DATA_5,
    DATA_6,
    DATA_7,
    DATA_8
};

enum ParityType
{
    PAR_NONE,
    PAR_ODD,
    PAR_EVEN,
    PAR_MARK,               //WINDOWS ONLY
    PAR_SPACE
};

enum StopBitsType
{
    STOP_1,
    STOP_1_5,               //WINDOWS ONLY
    STOP_2
};

enum FlowType
{
    FLOW_OFF,
    FLOW_HARDWARE,
    FLOW_XONXOFF
};

class serial : public QObject
{
    Q_OBJECT
public:
    explicit serial();
private:
    int fd;
    QSocketNotifier *m_notifier;
    QByteArray  *rev_buf;
    termios NewSerialPar;
    QMutex mutex;
public:
    bool openPort(QString portName,BaudRateType baundRate,DataBitsType dataBits,ParityType parity,
                     StopBitsType stopBits, FlowType flow ,int time_out);
    void closePort();
    int writeData(QByteArray buf,int len);
    QByteArray readData();
    static serial * serialInt();
signals:
    void hasdata(QString ip,qint32 port,qint32 dev_id);
    void BatteryHasData(void);
public slots:
    void remoteDateInComing();
};

#endif // SERIAL_H
