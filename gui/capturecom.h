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
    void updateCamera(QComboBox* target);
    void updateResolution(QComboBox* target);
    void startStreaming(int cameraNr, int resolutionNr);
    void stopStream();

signals:
    void addNewCamera(QString name);

public slots:
    void nextFrame();


private:
    CInterface* mInterface;
    int (*getPictureFunc)(bool,bool);


};

#endif // CAPTURECOM_H
