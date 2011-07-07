#include "CInterface.h"


CInterface::CInterface()
    : mCapture(NULL), mServer(NULL), mEncoder(NULL)
{

}

CInterface::~CInterface()
{
    qDebug("call ende for modules!\n");
    if(mCapture)
    {
        if(mCapture->ende)
             mCapture->ende();
        interface_close(mCapture);
    }
    qDebug("capture shutdown!\n");
   if(mServer)
    {
       if(mServer->ende)
           mServer->ende();
       interface_close(mServer);

    }
   qDebug("Server shutdown!\n");
   if(mEncoder)
   {
        if(mEncoder->ende)
            mEncoder->ende();
        interface_close(mEncoder);
    }
   qDebug("Encoder shutdown!\n");

   mCapture = NULL;
   mServer = NULL;
   mEncoder = NULL;
   qDebug("Cinterface deconstructor ende!\n");

}



int CInterface::init()
{
#ifdef _WIN32
    QString name = "server.dll";
    mServer = interface_loadDll(qPrintable(name));
    //mServer = interface_loadDll("libserver.dll");
    qDebug(qPrintable(name));
#else
 //  mCapture = interface_loadDll("libcapture.so");
   //mEncoder = interface_loadDll("libencoder.so");
   mServer = interface_loadDll("libserver.so");
#endif

   // mServer = interface_loadDll("OpenLive4Cam_server");
    if(! mServer)
    {
        QString error;

#ifndef _WIN32
        error.sprintf("Fehler beim laden einer Bibliothek (*.so)\n%s",  dlerror());
#else
        int errornr = GetLastError();
        if(errornr == 126)
            error.sprintf("Fehler beim laden einer dll\nDLL wurde nicht gefunden. ");
        else if(errornr == 127)
            error.sprintf("Eine Funktion in der DLL wurde nicht gefunden!\n");
        else
            error.sprintf("Fehler beim laden einer dll\nerror nr: %d ", (int)GetLastError());
#endif
        QMessageBox::critical(NULL, QString("OpenLive4Cam"), error, QMessageBox::Abort);
        return false;
    }
    int ret = mServer->init();
    //ret = mEncoder->init();

    return ret;
    //return true;

}

int CInterface::getParameter(QString name)
{
    return mServer->getParameter(qPrintable(name));
}

void CInterface::setParameter(QString name, int parameter)
{
    mServer->setParameter(qPrintable(name), parameter);
}

int CInterface::start()
{
    return mServer->start();
}

void CInterface::stop()
{
    mServer->stop();
}

void CInterface::ende()
{
    mServer->ende();
}
