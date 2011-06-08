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
        return 42;
}

void ende()
{
    picture_release(&g_rgbPicture);
    picture_release(&g_yuvPicture);
    printf("Ende called\n");
}



void setParameter(const char* name, int value)
{
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
    /*
    Mat temp;
    g_capture >> temp;
    double width = g_capture.get(CV_CAP_PROP_FRAME_WIDTH);
    double height = g_capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    
    g_rgbPicture.width = g_yuvPicture.width;
    g_rgbPicture.height = g_yuvPicture.height;
     * */
    
    printf("start called\n");

   // namedWindow("LIVE",1);
 
    return 0;
}

int getPicture(bool rgb/* = false*/, bool removeFrame/* = true*/)
{
    Mat m, m2;
    if(!removeFrame)
        g_capture.retrieve(m);
    else
        g_capture >> m; // get a new frame from camera
    //imshow("LIVE", frame);
    if(m.depth() != CV_8U)
    {
        printf("Error, depth != unsigned char\n");
        return 0;
    }
    //m.convertTo(m2, )
    Mat* matrices = new Mat[m.channels()];
    
    if(rgb)
    {
        split(m, matrices);
        int oldSize = picture_getSize(&g_rgbPicture);
        g_rgbPicture.width = m.cols;
        g_rgbPicture.height = m.rows;
        int newSize = picture_getSize(&g_rgbPicture);
        
        if(oldSize != newSize)
        {
            picture_release(&g_rgbPicture);
            if(picture_create(&g_rgbPicture, m.cols, m.rows, 1))
            {
                printf("Fehler beim speicher reservieren in getPicture!\n");
                return 0;
            }
        }
        size_t size = m.cols*m.rows;
        memcpy(g_rgbPicture.channel1, matrices[0].data, size);
        memcpy(g_rgbPicture.channel2, matrices[1].data, size);
        memcpy(g_rgbPicture.channel3, matrices[2].data, size);

        return (int)&g_rgbPicture;
        
    }
    else
    {
        cvtColor(m, m2, CV_BGR2YCrCb);
        split(m2, matrices);
        
    }
    return 0;
}
int stop()
{
    
    return 0;
}

