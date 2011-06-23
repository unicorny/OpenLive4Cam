#include "serverthread.h"

ServerThread::ServerThread(CInterface* interface, QObject *parent) :
    QThread(parent), mInterface(interface)
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
    int (*tick)();
            
    //forever {
        mutex.lock();

        tick = (int (*)())mInterface->getParameter("server.getTickFunc");
        if(tick) tick();

        mutex.unlock();
        usleep(1000);
    //}
}
