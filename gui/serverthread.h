#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "CInterface.h"

class ServerThread : public QThread
{
Q_OBJECT
public:
    explicit ServerThread(CInterface* interface, QObject *parent = 0);
    ~ServerThread();

signals:

public slots:

protected:
    void run();

private:
    QMutex mutex;
    QWaitCondition condition;
   //int (*tickFunc)();
    CInterface* mInterface;

};

#endif // SERVERTHREAD_H
