#include <QtGui/QApplication>
#include "mainwindow.h"

#include "CInterface.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    a.exec();
    qDebug("Programm Ende..\n");
    return 0;
}
