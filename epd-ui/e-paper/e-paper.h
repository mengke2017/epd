#ifndef EPAPER_H
#define EPAPER_H
#include <QThread>

class EPaper : public QThread
{
public:
    EPaper();
private:
    virtual void run();//任务处理线程
signals:

public slots:

};
#endif // EPAPER_H
