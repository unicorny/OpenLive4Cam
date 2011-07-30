#include "capturecom.h"
#include <stdio.h>

CaptureCom::CaptureCom(CInterface* in, QTextEdit* logger, QObject *parent) :
    QObject(parent), mInterface(in), getPictureFunc(NULL), checkIfNewDataAvailable(NULL), getFrameFunc(NULL), encodeFrame(NULL), generateSDP(NULL), mImage(NULL), mLogger(logger)
{
}

CaptureCom::~CaptureCom()
{
    if(mImage)
    {
        delete mImage;
        mImage = NULL;
    }
}

void CaptureCom::updateCamera(QComboBox* target)
{
    //QMessageBox::warning(NULL, QString("OpenLive4Cam"), QString("udpateCam"), QMessageBox::Abort);
    //Kameras
#ifndef _WIN32
    int count = mInterface->getParameter("capture.camera.count");
    for(int i = 0; i < count; i++)
    {
        QString name;
        target->addItem(QString((const char*)mInterface->getParameter(name.sprintf("capture.camera.%d.name", i))), QVariant(i));
    }
    if(count == 0)
    {
        target->addItem(QString("Keine Kamera gefunden!"), QVariant(-1));
        QMessageBox::critical(NULL, QString("OpenLive4Cam"), trUtf8("Es konnte leider keine Kamera gefunden werden. Bitte überprüfen sie die Anschlüsse!"), QMessageBox::Abort);
    }
#endif
}

void CaptureCom::updateResolution(QComboBox* target, QComboBox* camera)
{
    //Auflösung
#ifndef _WIN32
    QString getCount;
    int choosenKamera = camera->itemData(camera->currentIndex()).toInt();
    getCount.sprintf("capture.camera.%d.resolution.count", choosenKamera);
    int count = mInterface->getParameter(getCount);
    target->clear();
    for(int i = 0; i < count; i++)
    {
        QString name;
        QString x(QString((const char*)mInterface->getParameter(name.sprintf("capture.camera.%d.resolution.%d.x", choosenKamera, i))));
        QString y(QString((const char*)mInterface->getParameter(name.sprintf("capture.camera.%d.resolution.%d.y", choosenKamera, i))));
        target->addItem(x+"x"+y, QVariant(i));
    }
    if(count == 0)
    {
        target->addItem(trUtf8("Keine Auflösung!"), QVariant(-1));
        QMessageBox::critical(NULL, QString("OpenLive4Cam"), trUtf8("Es existiert keine Auflösung!"), QMessageBox::Abort);
    }
#endif
}

void CaptureCom::chooseCurrentCamera(int cameraNr)
{
    mInterface->setParameter("capture.camera.choose", cameraNr);
    QString text;
    mLogger->append(text.sprintf("Kamera: %d wurde ausgewaehlt", cameraNr));
}

int CaptureCom::startStreaming(int cameraNr, int resolutionNr)
{
    chooseCurrentCamera(cameraNr);
    QString res;
#ifndef _WIN32
    mInterface->setParameter(res.sprintf("capture.camera.%d.resolution.choose", cameraNr), resolutionNr);
#endif
/*
    int ret = 0;
    if((ret = mInterface->start()))
    {
        mLogger->append(res.sprintf("Fehler, Server konnte nicht gestartet werden!, Fehler nummer: %d", ret));
        char* m = NULL;
        while((m = (char*)mInterface->getParameter("getLastMessage")) != NULL)
        {
            mLogger->append(m);
        }
        stopStream();
        return -1;
    }
/*

    mLogger->append("Der Stream wurde gestartet!");
//*/
    FILE* f = fopen("video.264", "wb");
    if(f)
        fclose(f);
    return 0;
}

void CaptureCom::stopStream()
{
    mInterface->stop();
    mLogger->append("Der Stream wurde angehalten!");
}

void CaptureCom::nextFrame()
{
    char* m = NULL;
    static int count = 0;
    while((m = (char*)mInterface->getParameter("getLastMessage")) != NULL)
    {
        mLogger->append(m);
    }
  //  return;
    QString str;
   // mLogger->append(str.sprintf("Frame: %d", count++));

    if(!getPictureFunc)
        getPictureFunc = (SPicture* (*)(int, int))mInterface->getParameter("capture.getPictureFunc");
    if(!getFrameFunc)
        getFrameFunc = (unsigned char* (*)(int*))mInterface->getParameter("encoder.getFrameFunc");
    if(!encodeFrame)
        encodeFrame = (int (*)())mInterface->getParameter("encoder.EncodeFrameFunc");
    if(!checkIfNewDataAvailable)
        checkIfNewDataAvailable = (void (*)())mInterface->getParameter("server.checkIfNewDataAvailableFunc");

    if(!getPictureFunc || !getFrameFunc  || !encodeFrame || !checkIfNewDataAvailable)
    {
        qDebug("CaptureCom::nextFrame()  capture.getPictureFunc oder encoder.getFrameFunc oder server.getTickFunc fehlgeschlagen!\n");
        return;
    }
    int size = 0;
    unsigned char* data = NULL;//getFrameFunc(&size);

 /*   FILE* f = fopen("video.264", "ab");
    if(f && data)
    {
        fwrite(data, size, 1, f);
        fclose(f);
     //   qDebug("write frame, size: %d", size);
    }
    else if(!data)
    {
       // qDebug("data is zero, size: %d", size);
    }
    */
    //qDebug("current_Frame: %d\n", size);
    encodeFrame();

    checkIfNewDataAvailable();
    SPicture* pic = getPictureFunc(1, 0);

    if(!pic) return;

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

        qDebug("Fehler, falsches Imageformat von capture Komponente\n");
    }
    //mImage.*/
    emit setPicture(mImage);


    
}
