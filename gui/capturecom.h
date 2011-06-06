#ifndef CAPTURECOM_H
#define CAPTURECOM_H

#include <QObject>
#include <QComboBox>
#include <CInterface.h>

class CaptureCom : public QObject
{
Q_OBJECT
public:
    explicit CaptureCom(CInterface* interface, QObject *parent = 0);
    void updateCamera(QComboBox* target);
    void updateResolution(QComboBox* target);

signals:
    void addNewCamera(QString name);


private:
    CInterface* mInterface;


};

#endif // CAPTURECOM_H
