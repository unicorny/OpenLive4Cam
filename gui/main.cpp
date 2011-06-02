#include <QtGui/QApplication>
#include "mainwindow.h"
#include "../interface/interface.h"
 #include <QMessageBox>

#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    if(interface_loadDll("capture") == -1)
    {
        QString error;

#ifndef _WIN32
        error.sprintf("Fehler bei dll (capture) Load: %s",  dlerror());
        //DRLog.writeToLog("Fehler bei dll (%s) Load: %s", dllname, dlerror());
#else
        error.sprintf("Fehler bei dll (capture) error nr: %d ", GetLastError());
        //DRLog.writeToLog("Fehler bei dll (%s) Load, error nr: %d", dllname, GetLastError());
#endif
        //LOG_ERROR("Fehler bei dll load", DR_ERROR);
        QMessageBox::critical(&w, QString("OpenLive4Cam"), error, QMessageBox::Abort);
    }
    else
    {
        w.show();
        return a.exec();
    }
    return 42;

}
