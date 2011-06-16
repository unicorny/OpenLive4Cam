#include "camera.h"
#include <stdio.h>

#include <cv.h>
#include <highgui.h>

using namespace cv;
using namespace std;

char gBuffer[256];

int camera_resolution(string* params, int number)
{
    if(params[0] == string("count"))
        return 1;
   //return (int)"null";        
    if(params[0] == string("0")) //resolution number
    {
        VideoCapture cap(number);
        if(!cap.isOpened()) return 0;
        
        double v = 0.0;
        if(params[1] == string("x") || params[1] == string("width"))
            v = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        else if(params[1] == string("y") || params[1] == string("height"))
            v = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
        
        cap.release();
            
        sprintf(gBuffer, "%.0f", v);
        return (int)gBuffer;       
    }
    return 0;
    
}

int camera_count()
{
    int i = 0;
    while(true)
    {
        VideoCapture cap(i); // open the default camera
        if(!cap.isOpened())  // check if we succeeded
                return i;
        cap.release();
        i++;
    }
    
    return i;
}



int camera_number(int number, string* params)
{
    VideoCapture cap(number); // open the choosen camera
    if(!cap.isOpened())  // check if we succeeded
        return 0;
    
    if(params[0] == string("name"))
    {
        if(number == 0)
        {
            cap.release();
            return (int)"0 (default)";
        }
        else 
        {
            sprintf(gBuffer, "%d", number);
            cap.release();
            return (int)gBuffer;
        }
    }
    else if(params[0] == string("resolution"))
    {
        cap.release();
        return camera_resolution(&params[1], number);
    }
    cap.release();
    return 0;
}

int camera_getParameter(std::string* params)
{
    printf("Camera get parameter: %s\n", params[0].data());
    
    if(params[0] == string("count"))
        return camera_count();
      
    return camera_number(atoi(params[0].data()), &params[1]);
    
//    return 0;
}
