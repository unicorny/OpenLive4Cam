#include "capturecom.h"

CaptureCom::CaptureCom(CInterface* in, QObject *parent) :
    QObject(parent), mInterface(in), getPictureFunc(NULL), mImage(NULL)
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
}

void CaptureCom::updateResolution(QComboBox* target)
{
    //Auflösung
    int count = mInterface->getParameter("capture.camera.0.resolution.count");
    for(int i = 0; i < count; i++)
    {
        QString name;
        QString x(QString((const char*)mInterface->getParameter(name.sprintf("capture.camera.0.resolution.%d.x", i))));
        QString y(QString((const char*)mInterface->getParameter(name.sprintf("capture.camera.0.resolution.%d.y", i))));
        target->addItem(x+"x"+y, QVariant(i));
    }
    if(count == 0)
    {
        target->addItem(trUtf8("Keine Auflösung!"), QVariant(-1));
        QMessageBox::critical(NULL, QString("OpenLive4Cam"), trUtf8("Es existiert keine Auflösung!"), QMessageBox::Abort);
    }
}

void CaptureCom::startStreaming(int cameraNr, int resolutionNr)
{
    mInterface->setParameter("capture.camera.choose", cameraNr);
    QString res;
    mInterface->setParameter(res.sprintf("capture.camera.%d.resolution.choose", cameraNr), 1);
    mInterface->start();
}

void CaptureCom::stopStream()
{
    mInterface->stop();
}

void CaptureCom::nextFrame()
{
    if(!getPictureFunc)
        getPictureFunc = (int (*)(bool, bool))mInterface->getParameter("capture.getPictureFunc");

    SPicture* pic = (SPicture*)getPictureFunc(true, true);
    if(!pic) return;

    if(mImage &&(mImage->width() != pic->width || mImage->height() != pic->height))
    {
        delete mImage;
        mImage = NULL;
    }
    if(!mImage)
    {
        mImage = new QImage(pic->width, pic->height, QImage::Format_RGB32);
        qDebug("Size for memcpy: %d, adress: %d\n", pic->width*pic->height*4, pic->channel1);
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
