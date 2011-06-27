#include "serverthread.h"

ServerThread::ServerThread(CInterface* interface, QObject *parent) :
    QThread(parent), mInterface(interface),mLogger(NULL)
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
   // mLogger->append(QString("ServerThread::run start!"));
            
    //forever {
        mutex.lock();

        tick = (int (*)())mInterface->getParameter("server.getTickFunc");
        if(tick) tick();

        mutex.unlock();
        usleep(100);
        qDebug("Serverthread ende");
      //  mLogger->append(QString("ServerThread::run ende!"));
    //}
}
