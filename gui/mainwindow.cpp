#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), mCapture(NULL), mTimer(this), mStreamingRunning(false)
{
    if(!mInterface.init())
        QCoreApplication::exit();
    ui->setupUi(this);
    QTextEdit* line = this->findChild<QTextEdit*>("textLog");

    mCapture = new CaptureCom(&mInterface, line, this);

    mCapture->updateCamera(this->findChild<QComboBox*>("source_comboBox"));
    mCapture->updateResolution(this->findChild<QComboBox*>("resolution_comboBox"), this->findChild<QComboBox*>("source_comboBox"));

    connect(&mTimer, SIGNAL(timeout()), mCapture, SLOT(nextFrame()));
    connect(mCapture, SIGNAL(setPicture(QImage*)), this->findChild<VideoView*>("Picture"), SLOT(newPicture(QImage*)));

}

MainWindow::~MainWindow()
{
    delete ui;
    if(mCapture)
        delete mCapture;
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
    QComboBox* res = this->findChild<QComboBox*>("resolution_comboBox");
    if(!toggle)
        this->findChild<QPushButton *>("startButton")->setText("Stop Streaming");
    else
        this->findChild<QPushButton *>("startButton")->setText("Start Streaming");

    if(!toggle)
    {
        QComboBox* source =  this->findChild<QComboBox*>("source_comboBox");
        int source_index = source->currentIndex();
        int res_index = res->currentIndex();

        if(!mCapture->startStreaming(source->itemData(source_index).toInt(), res->itemData(res_index).toInt()))
        {
            mTimer.start(34);
            mStreamingRunning = true;
        }
        res->setEnabled(false);
    }
    else
    {
        mTimer.stop();
        mCapture->stopStream();
        mStreamingRunning = false;
        res->setEnabled(true);
    }
    toggle = !toggle;
}

void MainWindow::on_source_comboBox_currentIndexChanged(int index)
{
    mCapture->updateResolution(this->findChild<QComboBox*>("resolution_comboBox"), this->findChild<QComboBox*>("source_comboBox"));
    mCapture->chooseCurrentCamera(index);

}
