#include "e-paper.h"
#include <QDebug>
#include "e-paper/IT8951_USB.h"
#include "e-paper/miniGUI.h"

EPaper::EPaper()
{
    //init_device();
}

//(2) run()重新实现
void EPaper::run()
{
    while(true)
    {
        int r;
        r = dis_epd(AUTO_MODE);
        usleep(800000);//线程 睡眠一秒一次
        qWarning("r = %d",r);
    }
}
