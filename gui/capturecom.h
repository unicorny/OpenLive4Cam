#ifndef CAPTURECOM_H
#define CAPTURECOM_H

#include <QObject>
#include <QComboBox>
#include <CInterface.h>

class CaptureCom : public QObject
{
Q_OBJECT
public:
    explicit CaptureCom(CInterface* in, QObject *parent = 0);
    ~CaptureCom();

    void updateCamera(QComboBox* target);
    void updateResolution(QComboBox* target, QComboBox* camera);
    int startStreaming(int cameraNr, int resolutionNr);
    void stopStream();

signals:
    //void addNewCamera(QString name);
    //void setPicture(SPicture* pic);
    void setPicture(QImage* pic);


public slots:
    void nextFrame();


private:
    CInterface* mInterface;
    int (*getPictureFunc)(bool,bool);
    QImage* mImage;


};

#endif // CAPTURECOM_H
