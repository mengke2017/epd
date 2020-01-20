#ifndef RECV_THREAD_H
#define RECV_THREAD_H

#include <QThread>
#include "batterymanager.h"

class RecvThread : public QThread
{
public:
    RecvThread();
private:
    virtual void run();//任务处理线程
signals:

public slots:

};

#endif // RECV_THREAD_H
