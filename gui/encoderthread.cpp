#include "encoderthread.h"

EncoderThread::EncoderThread(CInterface* _interface, QObject *parent) :
    QThread(parent), mInterface(_interface)
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
    mInterface->start();
    return;

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

}
