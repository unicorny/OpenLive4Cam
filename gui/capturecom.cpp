#include "capturecom.h"

CaptureCom::CaptureCom(CInterface* in, QTextEdit* logger, QObject *parent) :
    QObject(parent), mInterface(in), getPictureFunc(NULL), generateSDP(NULL), mImage(NULL), mLogger(logger)
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
    if(mInterface->start())
    {
        qDebug("Fehler, Capture konnte nicht gestartet werden!");
        return -1;
    }
   /* if(!generateSDP)
        generateSDP = (const char*(*)())mInterface->getParameter("encoder.generateSDPFunc");
    if(!generateSDP)
    {
        qDebug("CaptureCom::startStreaming() encoder.generateSDPFunc fehlgeschlagen!\n");
        return -1;
    }
    */

    mLogger->append("Der Stream wurde gestartet!");
    return 0;
}

void CaptureCom::stopStream()
{
    mInterface->stop();
    mLogger->append("Der Stream wurde angehalten!");
}

void CaptureCom::nextFrame()
{
    if(!getPictureFunc)
        getPictureFunc = (int (*)(bool, bool))mInterface->getParameter("capture.getPictureFunc");
    if(!getPictureFunc)
    {
        qDebug("CaptureCom::nextFrame()  capture.getPictureFunc fehlgeschlagen!\n");
        return;
    }

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
