#include "camera.h"
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
        unlock_mutex(mutex);
        return 1;
    }
   //return (int)"null";        
    if(params[0] == string("0")) //resolution number
    {
        VideoCapture cap(number);
        if(!cap.isOpened())
        {
            unlock_mutex(mutex);
            return 0;
        }
        
        double v = 0.0;
        if(params[1] == string("x") || params[1] == string("width"))
            v = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        else if(params[1] == string("y") || params[1] == string("height"))
            v = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
        
        cap.release();
        if(v > 5000)
        {
            unlock_mutex(mutex);
            return 0;    
        }
        sprintf(gBuffer, "%.0f", v);
        unlock_mutex(mutex);
        return (int)gBuffer;       
    }
    unlock_mutex(mutex);
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
            unlock_mutex(mutex);
            return i;
        }
        cap.release();
        i++;
    }
    unlock_mutex(mutex);
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
            unlock_mutex(mutex);
            return (int)"0 (default)";
        }
        else 
        {
            sprintf(gBuffer, "%d", number);
           // cap.release();
            unlock_mutex(mutex);
            return (int)gBuffer;
        }
    }
    else if(params[0] == string("resolution"))
    {
       // cap.release();
        return camera_resolution(&params[1], number);
    }
   // cap.release();
    unlock_mutex(mutex);
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
