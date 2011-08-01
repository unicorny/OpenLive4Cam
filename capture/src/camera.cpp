#include "camera.h"
#include "../../interface/mutex.h"
#include <stdio.h>

#include <cv.h>
#include <highgui.h>


using namespace cv;
using namespace std;

//! \brief buffer for camera names
char gBuffer[256];

//! \brief return camera resolution
int camera_resolution(string* params, int number)
{
    if(params[0] == string("count"))
    {
        if(mutex_unlock(mutex))
            g_Messages.push(string("camera_resolution</b> <font color='red'>Fehler bei mutex_unlock 1</font>"));
        return 1;
    }
   //return (int)"null";        
    if(params[0] == string("0")) //resolution number
    {
        VideoCapture cap(number);
        if(!cap.isOpened())
        {
            if(mutex_unlock(mutex))
            g_Messages.push(string("camera_resolution</b> <font color='red'>Fehler bei mutex_unlock 2</font>"));
            return 0;
        }
        
        double v = 0.0;
        if(params[1] == string("x") || params[1] == string("width"))
            v = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        else if(params[1] == string("y") || params[1] == string("height"))
            v = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
        
        cap.release();
        // falls ein Fehler auftritt und v unmöglich hoch ist
        // falls auflösungen mit einer seite größer als 5000 unterstützt werden sollen
        // muss das hier abgeändert werden
        if(v > 5000)
        {
            if(mutex_unlock(mutex))
                g_Messages.push(string("camera_resolution</b> <font color='red'>Fehler bei mutex_unlock 3</font>"));;
            return 0;    
        }
        sprintf(gBuffer, "%.0f", v);
        if(mutex_unlock(mutex))
            g_Messages.push(string("camera_resolution</b> <font color='red'>Fehler bei mutex_unlock 4</font>"));
        return (int)gBuffer;       
    }
    if(mutex_unlock(mutex))
            g_Messages.push(string("camera_resolution</b> <font color='red'>Fehler bei mutex_unlock 5</font>"));;
    return 0;
    
}
//! \brief return camera count
int camera_count()
{
    int i = 0;
    while(true)
    {
        VideoCapture cap(i); // try to open a camera
        if(!cap.isOpened())  // check if we succeeded
        {
            if(mutex_unlock(mutex))
                g_Messages.push(string("camera_count</b> <font color='red'>Fehler bei mutex_unlock 1</font>"));
            return i;
        }
        cap.release();
        i++;
    }
    if(mutex_unlock(mutex))
            g_Messages.push(string("camera_count</b> <font color='red'>Fehler bei mutex_unlock 2</font>"));
    return i;
}


//! \brief return camera name or call camera_resolution()
int camera_number(int number, string* params)
{
   /* VideoCapture cap(number); // open the choosen camera
    if(!cap.isOpened())  // check if we succeeded
        return 0;
    * */
    
    if(params[0] == string("name"))
    {
        if(number == 0)
        {
            //cap.release();
            if(mutex_unlock(mutex))
                g_Messages.push(string("camera_number</b> <font color='red'>Fehler bei mutex_unlock 1</font>"));
            return (int)"0 (default)";
        }
        else 
        {
            sprintf(gBuffer, "%d", number);
           // cap.release();
            if(mutex_unlock(mutex))
                g_Messages.push(string("camera_number</b> <font color='red'>Fehler bei mutex_unlock 2</font>"));
            return (int)gBuffer;
        }
    }
    else if(params[0] == string("resolution"))
    {
       // cap.release();
        return camera_resolution(&params[1], number);
    }
   // cap.release();
    if(mutex_unlock(mutex))
        g_Messages.push(string("camera_number</b> <font color='red'>Fehler bei mutex_unlock 3</font>"));
    return 0;
}
//! \brief call camera_count() or camera_number()
int camera_getParameter(std::string* params)
{
    printf("Camera get parameter: %s\n", params[0].data());
    
    if(params[0] == string("count"))
        return camera_count();
      
    return camera_number(atoi(params[0].data()), &params[1]);
    
//    return 0;
}
