#include "e-paper.h"
#include <QDebug>
#include "e-paper/IT8951_USB.h"
#include "e-paper/miniGUI.h"

EPaper::EPaper()
{
#if LINUX_32BIT
    init_device();
#endif
}

//(2) run()重新实现
void EPaper::run()
{
    while(true)
    {
        int r;
        r = dis_epd(AUTO_MODE);
        usleep(300000);//线程 睡眠一秒一次
        if(r ==0)
            qWarning("r = %d",r);
    }
}
