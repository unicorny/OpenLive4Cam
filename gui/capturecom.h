#ifndef CAPTURECOM_H
#define CAPTURECOM_H

#include <QObject>
#include <QComboBox>
#include <QTextEdit>
#include <CInterface.h>

class CaptureCom : public QObject
{
Q_OBJECT
public:
    explicit CaptureCom(CInterface* in, QTextEdit* logger, QObject *parent = 0);
    ~CaptureCom();

    void updateCamera(QComboBox* target);
    void updateResolution(QComboBox* target, QComboBox* camera);
    int startStreaming(int cameraNr, int resolutionNr);
    void stopStream();
    void chooseCurrentCamera(int cameraNr);

signals:
    //void addNewCamera(QString name);
    //void setPicture(SPicture* pic);
    void setPicture(QImage* pic);


public slots:
    void nextFrame();


private:
    CInterface* mInterface;
    SPicture* (*getPictureFunc)(int,int);
    unsigned char* (*getFrameFunc)(int*);
    const char* (*generateSDP)();
    QImage* mImage;
    QTextEdit*   mLogger;
};

#endif // CAPTURECOM_H
