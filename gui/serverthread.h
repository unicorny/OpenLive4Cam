#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class ServerThread : public QThread
{
Q_OBJECT
public:
    explicit ServerThread(int (*tick)(), QObject *parent = 0);
    ~ServerThread();

signals:

public slots:

protected:
    void run();

private:
    QMutex mutex;
    QWaitCondition condition;
   int (*tickFunc)();

};

#endif // SERVERTHREAD_H
