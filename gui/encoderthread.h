#ifndef ENCODERTHREAD_H
#define ENCODERTHREAD_H

#include <QThread>
#include <QMutex>
#include <CInterface.h>

class EncoderThread : public QThread
{
Q_OBJECT
public:
    explicit EncoderThread(CInterface* interface, QObject *parent = 0);
    ~EncoderThread();

signals:

public slots:
protected:
    void run();

private:
     QMutex mutex;
    CInterface* mInterface;
};

#endif // ENCODERTHREAD_H
