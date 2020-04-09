#ifndef BATTERYLIST_H
#define BATTERYLIST_H

#include <QObject>
#include <QList>
#include <QReadWriteLock>
#define LIST_MAX_SIZE   30
//typedef struct Batter_data_info_{
//    quint16 serial;
//    QString cmd;
//    QString str;
//}Batter_data_info;
typedef struct BatterySysPam{
    quint16 cmd;
    quint16 crc;
    QByteArray arry;
}BatterySyspam;
class BatteryList : public QObject
{
    Q_OBJECT
public:
    explicit BatteryList(QObject *parent = 0);
    static BatteryList* GetInstans();
    BatterySyspam* get();
    bool add(const BatterySyspam& info);
    void remove();
private:
    QReadWriteLock LockMutex;
    QList<BatterySyspam> Blist;
signals:

public slots:

};

#endif // BATTERYLIST_H
