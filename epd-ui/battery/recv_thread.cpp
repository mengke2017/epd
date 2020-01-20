#include <QDebug>
#include "recv_thread.h"

BatteryManager   *battery;

RecvThread::RecvThread()
{
    battery = new BatteryManager();
}

//(2) run()重新实现
void RecvThread::run()
{
    while(true)
    {
        BatteryPara batteryPara;
        battery->ReadBatteryPara(&batteryPara);
        sleep(5);
    }
}

