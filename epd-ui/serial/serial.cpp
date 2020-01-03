#include "serial.h"
#include "tcp/client.h"
#include "QDebug"
serial::serial(QObject *parent) :
    QObject(parent)
{
    fd = -1;
}

bool serial::openPort(QString portName, BaudRateType baundRate, DataBitsType dataBits, ParityType parity, StopBitsType stopBits, FlowType flow, int time_out)
{
    if(fd != -1){
        qDebug()<<"fd error!";
        return false;
    }
    fd = open(portName.toLatin1(),O_RDWR|O_NONBLOCK);
    if(fd == -1){
        qDebug()<<"portName error!";
        return false;
    }
    switch(baundRate){
        case BAUD4800:
            NewSerialPar.c_cflag |= B4800;
            break;
        case BAUD9600:
            NewSerialPar.c_cflag |= B9600;
            break;
        case BAUD19200:
            NewSerialPar.c_cflag |= B19200;
            break;
        case BAUD57600:
            NewSerialPar.c_cflag |= B57600;
            break;
        case BAUD115200:
            NewSerialPar.c_cflag |= B115200;
            break;
        default:NewSerialPar.c_cflag &=(~CSTOPB);
            NewSerialPar.c_cflag |= B9600;
            break;
    }
    switch(dataBits){
        case DATA_5:
            NewSerialPar.c_cflag |= CS5;
            break;
        case DATA_6:
            NewSerialPar.c_cflag |= CS6;
            break;
        case DATA_7:
            NewSerialPar.c_cflag |= CS7;
            break;
        case DATA_8:
            NewSerialPar.c_cflag |= CS8;
            break;
        default:
            NewSerialPar.c_cflag |= CS8;
            break;
    }
    switch(parity){
        case PAR_NONE:
            NewSerialPar.c_cflag &= (~PARENB);
            break;
        case PAR_ODD:
            NewSerialPar.c_cflag |= (PARODD|PARENB);
            break;
        case PAR_EVEN:
            NewSerialPar.c_cflag &= (~PARODD);
            NewSerialPar.c_cflag |= PARENB;
            break;
        default:
            NewSerialPar.c_cflag &= (~PARENB);
            break;
    }
    switch(stopBits){
        case STOP_1:
            NewSerialPar.c_cflag &= (~CSTOPB);
            break;
        case STOP_1_5:

            break;
        case STOP_2:
            NewSerialPar.c_cflag |= CSTOPB;
            break;
        default:
            NewSerialPar.c_cflag &= (~CSTOPB);
            break;
    }
    switch(flow){
        case FLOW_OFF:
            NewSerialPar.c_cflag &= (~CRTSCTS);
            NewSerialPar.c_cflag &= (~(IXON|IXOFF|IXANY));
            break;
        case FLOW_HARDWARE:
            NewSerialPar.c_cflag &= (~CRTSCTS);
            NewSerialPar.c_cflag |= (IXON|IXOFF|IXANY);
            break;
        case FLOW_XONXOFF:
            NewSerialPar.c_cflag |= ~CRTSCTS;
            NewSerialPar.c_cflag &=(~(IXON|IXOFF|IXANY));
            break;
        default:
            NewSerialPar.c_cflag &= (~CRTSCTS);
            NewSerialPar.c_cflag &= (~(IXON|IXOFF|IXANY));
            break;
    }
    NewSerialPar.c_oflag=0;
    NewSerialPar.c_cc[VTIME]=time_out/100;
    tcflush(fd,TCIFLUSH);
    int result = tcsetattr(fd,TCSANOW,&NewSerialPar);
    if(result == -1){
        qDebug()<<"set portName error";
        return false;
    }
    m_notifier = new QSocketNotifier(fd,QSocketNotifier::Read,0);
    connect(m_notifier,SIGNAL(activated(int)),SLOT(remoteDateInComing()));
    return true;
}

void serial::closePort()
{

}

int serial::writeData(QByteArray data)
{
    mutex.lock();
    int ret;
    ret = 0;
    if(fd != -1){
        ret = write(fd,data.data(),data.length());
    }
    mutex.unlock();
    return ret;
}

QByteArray serial::readData()
{
//    mutex.lock();
//    QByteArray returnArry;
//    if(rev_buf->isEmpty()){
//        returnArry.append("111");
//        returnArry.clear();
//    }else{
//        returnArry.append(*rev_buf);
//        rev_buf->clear();
//    }
//    mutex.unlock();
    return 0;
}

void serial::remoteDateInComing()
{
    char buf[500];
    QString buf_string;
    QString ip_tcp;
    qint32 port_tcp;
    qint32 dev_id_tcp;
    QString str_data;
    mutex.lock();
    if(fd != -1){
        read(fd,buf,sizeof(buf));
        if(buf[0] == '$'){
          str_data = QString(QLatin1String(buf));
          do{
             str_data.remove(str_data.right(1));
          }while(str_data.right(1) != "~");
          str_data.remove(str_data.right(1));
          str_data.remove(str_data.left(1));
        }
        else{
            mutex.unlock();
            return;
        }

    }
    qDebug()<<str_data;
    if(!str_data.isEmpty()){
        QStringList TCP_pam = str_data.split(",");
        if(TCP_pam.size()>0){
            buf_string = TCP_pam.at(0);
            if(buf_string.left(3) == "ip:")
                ip_tcp = buf_string.mid(3);
            buf_string = TCP_pam.at(1);
            if(buf_string.left(5) == "port:")
                port_tcp = (buf_string.mid(5)).toInt();
            buf_string = TCP_pam.at(2);
           // qDebug()<<device_id;
            if(buf_string.left(3) == "id:")
                dev_id_tcp = (buf_string.mid(3)).toInt();
        }
    }
    emit hasdata(ip_tcp,port_tcp,dev_id_tcp);
    mutex.unlock();
}

