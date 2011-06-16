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

void CaptureCom::updateResolution(QComboBox* target, QComboBox* camera)
{
    //Auflösung
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
}

void CaptureCom::chooseCurrentCamera(int cameraNr)
{
    mInterface->setParameter("capture.camera.choose", cameraNr);
}

int CaptureCom::startStreaming(int cameraNr, int resolutionNr)
{
    chooseCurrentCamera(cameraNr);
    QString res;
    mInterface->setParameter(res.sprintf("capture.camera.%d.resolution.choose", cameraNr), resolutionNr);
    if(mInterface->start())
    {
        qDebug("Fehler, Capture konnte nicht gestartet werden!");
        return -1;
    }
    return 0;
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
