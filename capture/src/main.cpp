#include "capture.h"
#include "camera.h"
#include <stdio.h>

using namespace std;

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

int init()
{	
	return 42;
}

void ende()
{
}

std::string g_Parameters[MAX_PARAMETER_COUNT];

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
    
    for(int i = 0; i < MAX_PARAMETER_COUNT; i++)
    {
        printf("%d: %s\n", i, g_Parameters[i].data());
    }
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

