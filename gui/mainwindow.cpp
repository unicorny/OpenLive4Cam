#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(CInterface* in, QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), mInterface(in), mCapture(NULL), mTimer(this)
{
    ui->setupUi(this);
    mCapture = new CaptureCom(in, this);

    mCapture->updateCamera(this->findChild<QComboBox*>("source_comboBox"));
    mCapture->updateResolution(this->findChild<QComboBox*>("resolution_comboBox"));

    connect(&mTimer, SIGNAL(timeout()), mCapture, SLOT(nextFrame()));

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

    if(!toggle)
    {
        QComboBox* source =  this->findChild<QComboBox*>("source_comboBox");
        QComboBox* res = this->findChild<QComboBox*>("resolution_comboBox");
        int source_index = source->currentIndex();
        int res_index = res->currentIndex();

        mCapture->startStreaming(source->itemData(source_index).toInt(), res->itemData(res_index).toInt());
        mTimer.start(34);
    }
    else
    {
        mCapture->stopStream();
        mTimer.stop();
    }
    toggle = !toggle;
}
