#include "batterylist.h"

BatteryList::BatteryList(QObject *parent) :
    QObject(parent)
{

}

BatteryList *BatteryList::GetInstans()
{
    static BatteryList intans;
    return &intans;
}

BatterySyspam *BatteryList::get()
{
    if(!Blist.empty()){
        LockMutex.lockForRead();
        BatterySyspam* info = &Blist[0];
        LockMutex.unlock();
        return info;
    }
    return NULL;
}

bool BatteryList::add(const BatterySyspam &info)
{
    bool ret = false;
    LockMutex.lockForWrite();
    if(Blist.size() < LIST_MAX_SIZE){
        Blist.append(info);
        ret = true;
    }
    LockMutex.unlock();
    ret = false;
    return ret;
}

void BatteryList::remove()
{
    LockMutex.lockForWrite();
    if(!Blist.empty()){
        Blist.removeAt(0);
    }
    LockMutex.unlock();
}
