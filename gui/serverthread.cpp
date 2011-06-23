#include "serverthread.h"

ServerThread::ServerThread(int (*tick)(), QObject *parent) :
    QThread(parent), tickFunc(tick)
{

}

ServerThread::~ServerThread()
{
    quit();
    mutex.lock();
    condition.wakeOne();
    mutex.unlock();

    wait();
}

void ServerThread::run()
{
    if(!tickFunc) return;
    forever {
        mutex.lock();
        for(int i = 0; i < 100; i++)
               tickFunc();
        mutex.unlock();
        usleep(10);
    }
}
