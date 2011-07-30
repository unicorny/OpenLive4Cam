#include "encoderthread.h"

EncoderThread::EncoderThread(CInterface* _interface, QObject *parent) :
    QThread(parent), mLogger(NULL),  mInterface(_interface)
{
}

EncoderThread::~EncoderThread()
{
    mutex.lock();
    quit();
    mutex.unlock();
    wait();
    qDebug("EncoderThread beendet!\n");
}

void EncoderThread::run()
{

    int ret = 0;
    if((ret = mInterface->start()))
    {
        QString res;
        mLogger->append(res.sprintf("Fehler, Server konnte nicht gestartet werden!, Fehler nummer: %d", ret));
        char* m = NULL;
        while((m = (char*)mInterface->getParameter("getLastMessage")) != NULL)
        {
            mLogger->append(m);
        }
        //stopStream();
    }
    return;
/*
    int (*encodeFrame)();
    encodeFrame = (int (*)())mInterface->getParameter("encoder.EncodeFrameFunc");
 //   return;
    forever
    {
        mutex.lock();
        if(encodeFrame())
        {
            mutex.unlock();
            break;
        }
        mutex.unlock();
        usleep(100);
    }
//*/
}
