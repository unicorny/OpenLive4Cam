#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <CInterface.h>
#include <capturecom.h>
#include <serverthread.h>
#include <encoderthread.h>

#define SAVE_DELETE(x) {if(x){delete x; x = NULL;}}
#define SAVE_DELETE_ARRAY(x) {if(x){delete [] x; x = NULL;}}

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    CInterface     *mInterface;
    CaptureCom     *mCapture;
    ServerThread   *mServer;
    EncoderThread  *mEncoderThread;

    QTimer          mTimer;
    bool            mStreamingRunning;

private slots:
    void on_port_spinBox_valueChanged(int );
    void on_chooseKam_clicked();
    void on_source_comboBox_currentIndexChanged(int index);
    void on_startButton_clicked();

};

#endif // MAINWINDOW_H
