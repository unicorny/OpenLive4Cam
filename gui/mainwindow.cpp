#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(CInterface* in, QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), mInterface(in)
{
    ui->setupUi(this);
    mCapture = new CaptureCom(in, this);

    mCapture->updateCamera(this->findChild<QComboBox*>("source_comboBox"));
    mCapture->updateResolution(this->findChild<QComboBox*>("resolution_comboBox"));
}

MainWindow::~MainWindow()
{
    delete ui;
    if(mCapture)
        delete mCapture;
    if(mInterface)
        delete mInterface;

}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_startButton_clicked()
{
    static bool toggle = false;
    if(!toggle)
        this->findChild<QPushButton *>("startButton")->setText("Stop Streaming");
    else
        this->findChild<QPushButton *>("startButton")->setText("Start Streaming");
    toggle = !toggle;
}
