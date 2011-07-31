#ifndef ENCODERTHREAD_H
#define ENCODERTHREAD_H

#include <QThread>
#include <QMutex>
#include <CInterface.h>

class EncoderThread : public QThread
{
Q_OBJECT
public:
    explicit EncoderThread(CInterface* _interface, QObject *parent = 0);
    ~EncoderThread();

signals:
    void appendLog(QString logText);
    void setPicture(QImage* pic);

public slots:
protected:
    void run();

private:
     QMutex mutex;
     QImage* mImage;
    CInterface* mInterface;
};

#endif // ENCODERTHREAD_H
