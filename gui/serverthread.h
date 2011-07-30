#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QTextEdit>
#include <QWaitCondition>
#include "CInterface.h"

class ServerThread : public QThread
{
Q_OBJECT
public:
    explicit ServerThread(CInterface* _interface, QObject *parent = 0);
    ~ServerThread();

signals:
     void appendLog(QString logText);

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
