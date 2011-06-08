#include "CInterface.h"


CInterface::CInterface()
    : mCapture(NULL), mServer(NULL)
{

}

CInterface::~CInterface()
{
   if(mCapture->ende)
        mCapture->ende();
   interface_close(mCapture);
   if(mServer)
    interface_close(mServer);
   mCapture = NULL;
   mServer = NULL;

}

int CInterface::init()
{
#ifdef _WIN32
   mCapture = interface_loadDll("libcapture.dll");
#else
   mCapture = interface_loadDll("libcapture.so");
#endif

   // mServer = interface_loadDll("OpenLive4Cam_server");
    if(!mCapture)// || !mServer)
    {
        QString error;

#ifndef _WIN32
        error.sprintf("Fehler beim laden einer Bibliothek (*.so)\n%s",  dlerror());
#else
        if(GetLastError() == 126)
            error.sprintf("Fehler beim laden einer dll\nDLL wurde nicht gefunden. ");
        else
            error.sprintf("Fehler beim laden einer dll\nerror nr: %d ", (int)GetLastError());
#endif
        QMessageBox::critical(NULL, QString("OpenLive4Cam"), error, QMessageBox::Abort);
        return false;
    }
    int ret = mCapture->init();

    return ret;
    //return true;

}

int CInterface::getParameter(QString name)
{
    return mCapture->getParameter(qPrintable(name));
}
