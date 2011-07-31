#include "encoderthread.h"
#include "CInterface.h"

EncoderThread::EncoderThread(CInterface* _interface, QObject *parent) :
    QThread(parent), mImage(NULL), mInterface(_interface)
{
}

EncoderThread::~EncoderThread()
{
    mutex.lock();
    quit();
    mutex.unlock();
    wait();

    if(mImage)
    {
        delete mImage;
        mImage = NULL;
    }
    qDebug("EncoderThread beendet!\n");
}

void EncoderThread::run()
{
    int ret = 0;
    QString str;
    if((ret = mInterface->start()))
    {
        emit appendLog(str.sprintf("<b>gui.EncoderThread::run</b> <font color='red'>Server konnte nicht gestartet werden!, Fehler nummer: %d</font>", ret));
        char* m = NULL;
        while((m = (char*)mInterface->getParameter("getLastMessage")) != NULL)
        {
            emit appendLog(m);
        }
        //stopStream();
        return;
    }
    //return;

    int (*encodeFrame)();
    SPicture* (*getPictureFunc)(int, int);
    void (*checkIfNewDataAvailable)();

    encodeFrame = (int (*)())mInterface->getParameter("encoder.EncodeFrameFunc");
    getPictureFunc = (SPicture* (*)(int, int))mInterface->getParameter("capture.getPictureFunc");
    checkIfNewDataAvailable = (void (*)())mInterface->getParameter("server.checkIfNewDataAvailableFunc");

    if(!encodeFrame || !getPictureFunc || !checkIfNewDataAvailable)
    {
        emit appendLog("<b>gui.EncoderThread::run</b> <font color='red'>Einer der Funktionen ist zero!</font>");
        return;
    }

 //   return;
    forever
    {
        mutex.lock();
        if(encodeFrame())
        {
            mutex.unlock();
            break;
        }
        checkIfNewDataAvailable();
        SPicture* pic = getPictureFunc(1, 0);

        if(pic <= 0)
        {
            emit appendLog(str.sprintf("<b>gui.EncoderThread::run</b><font color='red'> Picture is %d!</font>", (int)pic));
            break;
        }

        if(mImage &&(mImage->width() != pic->width || mImage->height() != pic->height))
        {
            delete mImage;
            mImage = NULL;
        }
        if(!mImage)
        {
            mImage = new QImage(pic->width, pic->height, QImage::Format_RGB32);
            qDebug("gui.CaptureCom::nextFrame Size for memcpy: %d, adress: %d\n", pic->width*pic->height*4, (int)pic->channel1);
        }
       // qDebug("StepSize: %d\n", pic->pixelsize);
        if(pic->pixelsize == 4)
        {
            for(int i = 0; i < pic->height; i++)
            {
                memcpy(mImage->scanLine(i), &pic->channel1[i*pic->width*4], pic->width*4);
            }
            //memcpy(mImage->bits(), pic->channel1, pic->width*pic->height*4);
        }
        else
        {

            emit appendLog("<b>gui.EncoderThread::run</b> <font color='red'>falsches Imageformat von capture Komponente, pixelsize von 4 erwartet</font>\n");
        }
        //mImage.*/
        emit setPicture(mImage);

        mutex.unlock();
        usleep(1000);
    }
//*/
}
