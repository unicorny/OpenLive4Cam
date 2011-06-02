#include "interface.h"

static SInterface interface;
/*
    int (*init)(void);
    void (*exit)(void);
    void (*setParameter)(const char*,int);
    int (*getParameter)(const char*,int);
  */
int interface_loadDll(const char* dllname)
{
    //try to load from dll
    if(!interface.dll)
    {
#ifdef _WIN32
        interface.dll = LoadLibrary(dllname);
#else
        interface.dll = dlopen(dllname, RTLD_LAZY);
#endif
    }
    if(interface.dll)
    {
#ifdef _WIN32
        if(!interface.init)
            interface.init = (int (*)())(GetProcAddress(interface.dll, "init"));
        if(!interface.exit)
            interface.exit = (void (*)())(GetProcAddress(interface.dll, "exit"));
#else
        if(!interface.init)
            interface.init = (int (*)())(dlsym(interface.dll, "init"));
        if(!interface.exit)
            interface.exit = (void (*)())(dlsym(interface.dll, "exit"));
#endif
    }
    else
    {
        return -1;
    }
    return 0;

}

void interface_close()
{

}
