#include "server.h"



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
SInterface* encoder = NULL;

int init()
{
    printf("server Modul init\n");
#ifdef _WIN32
#else
    encoder = interface_loadDll("libencoder.so");
#endif
    if(!encoder)
    {
        printf("server::init Fehler beim laden des Capture Modules\n");
        return -1;
    }
    if(encoder->init() < 0)
    {
        printf("server::init Fehler beim initalisieren von Capture!\n");
        return -2;
    }
    return 0;
}

void ende()
{
    if(encoder)
        encoder->ende();
    interface_close(encoder);
    printf("Server Modul ende\n");
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
    if(g_Parameters[0] != string(g_modulname) && encoder)
            encoder->setParameter(name, value); //TODO: weiterleiten  
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
    if(g_Parameters[0] != string(g_modulname) && encoder)
        return encoder->getParameter(name);
    
    return 0;
}
int start()
{
    if(encoder)
       return encoder->start();
}
int stop()
{
    if(encoder)
        encoder->stop();
    return 0;
}