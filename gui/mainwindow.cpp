#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), mInterface(NULL), mCapture(NULL),mServer(NULL), mTimer(this), mStreamingRunning(false)
{
    mInterface = new CInterface();
    int ret = 0;
    ret = mInterface->init();
    if(!ret)
    {
        QCoreApplication::exit();
    }
    else if(ret <= 0)
    {
        QMessageBox::critical(NULL, QString("OpenLive4Cam"), QString("Fehler beim laden einer Sub-Bibliothek, für mehr Infos schaue auf die Fehlerausgabe auf der Konsole.\n"), QMessageBox::Abort);
        QCoreApplication::exit();
    }

    ui->setupUi(this);
    QTextEdit* line = this->findChild<QTextEdit*>("textLog");

#ifdef _WIN32
    //Bedienung an Windows anpassen
    QGroupBox* group = this->findChild<QGroupBox*>("group_box");
    QPushButton* button = this->findChild<QPushButton*>("chooseKam");
    group->setEnabled(false);
    button->setEnabled(true);

#endif
    mServer = new ServerThread(mInterface, this);
    mEncoderThread = new EncoderThread(mInterface, this);
    mCapture = new CaptureCom(mInterface, line, this);

    mCapture->updateCamera(this->findChild<QComboBox*>("source_comboBox"));
    mCapture->updateResolution(this->findChild<QComboBox*>("resolution_comboBox"), this->findChild<QComboBox*>("source_comboBox"));

    connect(&mTimer, SIGNAL(timeout()), mCapture, SLOT(nextFrame()));
    connect(mCapture, SIGNAL(setPicture(QImage*)), this->findChild<VideoView*>("Picture"), SLOT(newPicture(QImage*)));

    mServer->setLogger(line);
    mEncoderThread->setLogger(line);
    mServer->start();
}

MainWindow::~MainWindow()
{

    SAVE_DELETE(ui);
    SAVE_DELETE(mCapture);
    mInterface->ende();
    SAVE_DELETE(mEncoderThread);
    SAVE_DELETE(mServer);
    SAVE_DELETE(mInterface);
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
    QSpinBox*  port   = this->findChild<QSpinBox*>("port_spinBox");

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
            //Start Stream
            mTimer.start(30);
            mStreamingRunning = true;
            mEncoderThread->start();
            res->setEnabled(false);
            port->setEnabled(false);
        }
        else
        {
            this->findChild<QPushButton *>("startButton")->setText("Start Streaming");
            toggle = !toggle;
        }

    }
    else //Stop Stream
    {
        mTimer.stop();
        mCapture->stopStream();
        mStreamingRunning = false;
        res->setEnabled(true);
        port->setEnabled(true);
    }
    toggle = !toggle;
}

void MainWindow::on_source_comboBox_currentIndexChanged(int index)
{
    mCapture->updateResolution(this->findChild<QComboBox*>("resolution_comboBox"), this->findChild<QComboBox*>("source_comboBox"));
    mCapture->chooseCurrentCamera(index);

}

void MainWindow::on_chooseKam_clicked()
{
    mCapture->chooseCurrentCamera(0);
}

void MainWindow::on_port_spinBox_valueChanged(int value)
{
    mInterface->setParameter("server.port", value);
}
