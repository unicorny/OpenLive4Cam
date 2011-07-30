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
    void setLogger(QTextEdit* log) {mLogger = log;}

signals:

public slots:
protected:
    void run();

private:
     QMutex mutex;
     QTextEdit*   mLogger;
    CInterface* mInterface;
};

#endif // ENCODERTHREAD_H
