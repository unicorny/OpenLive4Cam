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
   interface_close(mServer);
   mCapture = NULL;
   mServer = NULL;

}

int CInterface::init()
{
    mCapture = interface_loadDll("libcapture.so");
   // mServer = interface_loadDll("OpenLive4Cam_server");
    if(!mCapture)// || !mServer)
    {
        QString error;

#ifndef _WIN32
        error.sprintf("Fehler beim laden einer Bibliothek (*.so)\n%s",  dlerror());
#else
        error.sprintf("Fehler beim laden einer dll\nerror nr: %d ", GetLastError());
#endif
        QMessageBox::critical(NULL, QString("OpenLive4Cam"), error, QMessageBox::Abort);
        return false;
    }


    return true;

}
