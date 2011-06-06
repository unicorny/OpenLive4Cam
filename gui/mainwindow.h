#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <CInterface.h>
#include <capturecom.h>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(CInterface* interface, QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    CInterface     *mInterface;
    CaptureCom     *mCapture;

private slots:
    void on_startButton_clicked();

};

#endif // MAINWINDOW_H
