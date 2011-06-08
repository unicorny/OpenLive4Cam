#include "capturecom.h"

CaptureCom::CaptureCom(CInterface* in, QObject *parent) :
    QObject(parent), mInterface(in)
{
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
