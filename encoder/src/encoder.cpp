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

std::string g_Parameters[MAX_PARAMETER_COUNT];
SInterface* capture;
bool g_run = false;


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
    int count = 0;
    while (pch != NULL)
    {
        g_Parameters[count++] = pch;
        pch = strtok (NULL, ".\0");
        
    }
    //if(g_Parameters[0] != string(g_modulname))
            //TODO: weiterleiten
    
    
        
}

int getParameter(const char* name)
{
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
    
    return 0;
  
}

int start()
{
   
    return 0;
}

int stop()
{
    return 0;
}