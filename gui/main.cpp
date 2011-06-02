#include <QtGui/QApplication>
#include "mainwindow.h"

#include "CInterface.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    CInterface in;
    if(in.init())
    {
        w.show();
        return a.exec();
    }
    return 42;
}
