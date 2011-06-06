#include <QtGui/QApplication>
#include "mainwindow.h"

#include "CInterface.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CInterface* in = new CInterface;
    if(in->init())
    {
        MainWindow w(in);
        w.show();
        return a.exec();
    }
    return 42;
}
