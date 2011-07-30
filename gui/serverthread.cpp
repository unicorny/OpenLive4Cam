#include "serverthread.h"

ServerThread::ServerThread(CInterface* _interface, QObject *parent) :
    QThread(parent), mInterface(_interface)
{

}

ServerThread::~ServerThread()
{
    quit();

    wait();
    qDebug("ServerThread beendet!");
}

void ServerThread::run()
{
    int (*tick)();
    emit appendLog(QString("ServerThread::run start!"));
            
    //forever {
        mutex.lock();

        tick = (int (*)())mInterface->getParameter("server.getTickFunc");
        if(tick) tick();

        mutex.unlock();
        usleep(100);
        emit appendLog(QString("ServerThread::run ende!"));
    //}
}
