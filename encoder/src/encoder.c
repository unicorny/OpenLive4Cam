#include "encoder.h"

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

SInterface* capture;
int g_run = 0;


int init()
{	
    printf("encoder Modul init\n");
#ifdef _WIN32
#else
    capture = interface_loadDll("libcapture.so");
#endif
    if(!capture)
    {
        printf("Fehler beim laden des Capture Modules\n");
        return -1;
    }
    if(capture->init() < 0)
    {
        printf("Fehler beim initalisieren von Capture!\n");
        return -2;
    }
    return 42;
}

void ende()
{
    if(capture)
        capture->ende();
    interface_close(capture);
    printf("Encoder Modul ende\n");
}



void setParameter(const char* name, int value)
{
    char buffer[256];
    sprintf(buffer, "%s", name);    
  
    char * pch;
    pch = strtok (buffer, ".\0");
    /*int count = 0;
    while (pch != NULL)
    {
        g_Parameters[count++] = pch;
        pch = strtok (NULL, ".\0");
        
    }
        */
    if(strcmp(pch, g_modulname) != 0 && capture)
            capture->setParameter(name, value); //TODO: weiterleiten    
        
}

int getParameter(const char* name)
{
    char buffer[256];
    sprintf(buffer, "%s", name);    
  
    char * pch;
    pch = strtok (buffer, ".\0");
   /* int count = 0;
    while (pch != NULL)
    {
        g_Parameters[count++] = pch;
        pch = strtok (NULL, ".\0");
        
    }*/
    if(strcmp(pch, g_modulname) != 0)
    {
       printf("pch: %s\n", pch); 
        if(capture)
            return capture->getParameter(name);
        //TODO: weiterleiten
        return 0;
    }
    else if(strcmp(name, "encoder.generateSDPFunc") == 0)
        return (int)generateSDP;
        
    
    return 0;
  
}

int start()
{
    //char resolution[256];
    //sprintf(resolution, "--input-res %dx%d", capture->getParameter("capture.resolution.x"), capture->getParameter("capture.resolution.y"));
    char* argv[5];
    const char progname[] = "encoder";
    const char profile[] = "--profile baseline";
    const char resolution[] = "--input-res 360x192";
    const char output[] = "-o rtp://192.168.1.51:5004";
    const char input[] = "/media/Videos/jumper.yuv";
    
    argv[0] = progname;
    argv[1] = input;    
    argv[2] = output; 
    argv[3] = resolution;    
    argv[4] = profile;
    
    if(capture) capture->start();
    
    start_x264(3, argv);
    return 0;
}

int stop()
{
    if(capture) capture->stop();
    return 0;
}

const char* generateSDP()
{
    return "SDP";
}