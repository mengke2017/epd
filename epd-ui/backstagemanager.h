#ifndef BACKSTAGEMANAGER_H
#define BACKSTAGEMANAGER_H

#include <QObject>

class BackstageManager : public QObject
{
    Q_OBJECT

public:
    explicit BackstageManager(QObject *parent = 0);
};

#endif // BACKSTAGEMANAGER_H
