#ifndef __GUI_INTERFACE_H
#define __GUI_INTERFACE_H

 #include <QMessageBox>
#include "../interface/interface.h"

class CInterface
{
public:
    CInterface();
    ~CInterface();

    //! \return 1 true, 0 bei fehler
    int init();
    int getParameter(QString name);


private:
    SInterface* mCapture;
    SInterface* mServer;
};

#endif // __GUI_INTERFACE_H
