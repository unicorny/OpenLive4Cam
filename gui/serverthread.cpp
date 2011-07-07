#include "serverthread.h"

ServerThread::ServerThread(CInterface* _interface, QObject *parent) :
    QThread(parent), mLogger(NULL), mInterface(_interface)
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
  //  mLogger->append(QString("ServerThread::run start!"));
            
    //forever {
        mutex.lock();

        tick = (int (*)())mInterface->getParameter("server.getTickFunc");
        if(tick) tick();

        mutex.unlock();
        usleep(100);
//        mLogger->append(QString("ServerThread::run ende!"));
    //}
}
