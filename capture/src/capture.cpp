#include "capture.h"
#include "camera.h"
#include <stdio.h>

#include "../../interface/interface.h"
#include "../../interface/picture.h"



#ifdef _WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to proces
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread

            break;
    }
    return TRUE; // succesful
}
#endif //_WIN32

std::string g_Parameters[MAX_PARAMETER_COUNT];
bool g_run = false;
Config g_cfg;
VideoCapture g_capture;
SPicture g_rgbPicture;
SPicture g_yuvPicture;


int init()
{	
    picture_init(&g_rgbPicture);
    picture_init(&g_yuvPicture);
    g_rgbPicture.rgb = 1;
    g_yuvPicture.rgb = 0;
    printf("Capture Modul init!\n");
    
    return 42;
}

void ende()
{
    picture_release(&g_rgbPicture);
    picture_release(&g_yuvPicture);
    printf("capture.ende: called\n");
}



void setParameter(const char* name, int value)
{
    //printf("capture.setParameter: name: %s, value: %d\n", name, value);
    if(string(name) == string("capture.camera.choose"))
    {
      //  printf("Kamera Nummer: %d ausgewaehlt\n", value);
        if(g_run && value != g_cfg.cameraNr)
        {
            g_run = false;
            if(g_capture.isOpened())
                g_capture.release();
            g_capture.open(value);
            if(!g_capture.isOpened())
            {
                g_capture.open(g_cfg.cameraNr);
                g_run = true;
                return;
            }
            g_run = true;
        }
        g_cfg.cameraNr = value;
        g_capture.set(CV_CAP_PROP_FRAME_WIDTH, g_cfg.width);
        g_capture.set(CV_CAP_PROP_FRAME_HEIGHT, g_cfg.height);
    }
    if(!g_run)
    {
        if(string(name) == string("capture.resolution.x") || 
        string(name) == string("capture.resolution.width"))
            g_cfg.width = value;
        else if(string(name) == string("capture.resolution.y")||
                string(name) == string("capture.resolution.height"))
            g_cfg.height = value;
    }
        
}

int getParameter(const char* name)
{
    printf("Name: %s\n", name);
    char buffer[256];
    sprintf(buffer, "%s", name);
    
  
    char * pch;
    pch = strtok (buffer, ".\0");
    int count = 0;
    while (pch != NULL)
    {
        g_Parameters[count++] = pch;
        pch = strtok (NULL, ".\0");
        
    }
    if(g_Parameters[0] != string(g_modulname))
            //TODO: weiterleiten
            return 0;
    if(g_Parameters[1] == string("camera"))
            return camera_getParameter(&g_Parameters[2]);
    else if(g_Parameters[1] == string("getPictureFunc") )
        return (int)getPicture;
    
    for(int i = 0; i < MAX_PARAMETER_COUNT; i++)
    {
        printf("%d: %s\n", i, g_Parameters[i].data());
    }
    return 0;
  
}

int start()
{
    g_capture.open(g_cfg.cameraNr);    
    if(!g_capture.isOpened())  // check if we succeeded
        return -1;
    
    g_capture.set(CV_CAP_PROP_FRAME_WIDTH, g_cfg.width);
    g_capture.set(CV_CAP_PROP_FRAME_HEIGHT, g_cfg.height);
    
    Mat temp;
    g_capture >> temp;
    g_cfg.width = temp.cols;
    g_cfg.height = temp.rows;
    
    g_run = true;
    printf("start called\n");

   // namedWindow("LIVE",1);
 
    return 0;
}

int getPicture(bool rgb/* = false*/, bool removeFrame/* = true*/)
{
    if(!g_run || !g_capture.isOpened()) return 0;
    
    Mat m, m2, m3;
    if(!removeFrame)
        g_capture.retrieve(m);
    else
        g_capture >> m; // get a new frame from camera
    //m = cvLoadImage("test.jpg");
    if(m.depth() != CV_8U)
    {
        printf("Error, depth != unsigned char\n");
        return 0;
    }
    //m.convertTo(m2, )
    Mat matrices[4];    
    if(rgb)
    {        
        
        IplImage src = m;
        IplImage* scaled = cvCreateImage(cvSize(g_cfg.width, g_cfg.height), IPL_DEPTH_8U, 3);
        cvResize( &src, scaled, CV_INTER_LINEAR );
        imshow("LIVE", scaled);
        split(scaled, matrices);
        matrices[3] = matrices[0].clone();
        matrices[3] = Scalar(255);
        merge(matrices, 4, m2);
        
        int oldSize = picture_getSize(&g_rgbPicture);
        g_rgbPicture.width = m2.cols;
        g_rgbPicture.height = m2.rows;
        int newSize = picture_getSize(&g_rgbPicture);  
                
        if(oldSize != newSize)
        {
            picture_release(&g_rgbPicture);
            if(picture_create(&g_rgbPicture, m2.cols, m2.rows, 4))
            {
                printf("Fehler beim speicher reservieren in getPicture rgb!\n");
                return 0;
            }
        }
        
        //return 0;
        size_t size = m2.cols*m2.rows*4;
        memcpy(g_rgbPicture.channel1, m2.data, size);
 
        cvReleaseImage(&scaled);
        
        return (int)&g_rgbPicture;
        
    }
    else
    {
        cvtColor(m, m2, CV_BGR2YCrCb);
        split(m2, matrices);
        
        IplImage* U = cvCreateImage(cvSize(m.cols/2, m.rows/2), IPL_DEPTH_8U, 1);
        IplImage* V = cvCreateImage(cvSize(m.cols/2, m.rows/2), IPL_DEPTH_8U, 1);
        IplImage uSrc = matrices[1];
        IplImage vSrc = matrices[2];
        cvResize(&uSrc, U, CV_INTER_LINEAR);
        cvResize( &vSrc, V, CV_INTER_LINEAR );
        imshow("Y", matrices[0]);
        imshow("U", U);
        imshow("V", V);
        
        int oldSize = picture_getSize(&g_yuvPicture);
        g_yuvPicture.width = m.cols;
        g_yuvPicture.height = m.rows;
        int newSize = picture_getSize(&g_yuvPicture);

        if(oldSize != newSize)
        {
            picture_release(&g_yuvPicture);
            if(picture_create(&g_yuvPicture, m2.cols, m2.rows, 1))
            {
                printf("Fehler beim speicher reservieren in getPicture yuv!\n");
                return 0;
            }
        }
        //return 0;
        size_t size = m2.cols*m2.rows;
        memcpy(g_yuvPicture.channel1, matrices[0].data, size);
        memcpy(g_yuvPicture.channel2, U->imageData, size/2);
        memcpy(g_yuvPicture.channel3, V->imageData, size/2);
        
        cvReleaseImage(&U);
        cvReleaseImage(&V);
        
        return (int)&g_yuvPicture;

        
    }
    return 0;
}
int stop()
{
    g_run = false;
    g_capture.release();
    return 0;
}

