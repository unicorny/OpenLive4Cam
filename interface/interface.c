#include "malloc.h"
#include "memory.h"
#include "interface.h"


/*
    int (*init)(void);
    void (*exit)(void);
    void (*setParameter)(const char*,int);
    int (*getParameter)(const char*);
  */
SInterface* interface_loadDll(const char* dllname)
{
    SInterface* interface = (SInterface*)malloc(sizeof(SInterface));
    memset(interface, 0, sizeof(SInterface));
    //try to load from dll
    if(!interface->dll)
    {
#ifdef _WIN32
        interface->dll = LoadLibrary(dllname);
#else
        interface->dll = dlopen(dllname, RTLD_LAZY);
#endif
    }
    if(interface->dll)
    {
#ifdef _WIN32
        if(!interface->init)
            interface->init = (int (*)())(GetProcAddress(interface->dll, "init"));
        if(!interface->ende)
            interface->ende = (void (*)())(GetProcAddress(interface->dll, "ende"));
        if(!interface->setParameter)
            interface->setParameter = (void (*)(const char*,int))(GetProcAddress(interface->dll, "setParameter"));
        if(!interface->getParameter)
            interface->getParameter = (int* (*)(const char*))(GetProcAddress(interface->dll, "getParameter"));

#else
        if(!interface->init)
            interface->init = (int (*)())(dlsym(interface->dll, "init"));
        if(!interface->ende)
            interface->ende = (void (*)())(dlsym(interface->dll, "ende"));
        if(!interface->setParameter)
            interface->setParameter = (void (*)(const char*,int))(dlsym(interface->dll, "setParameter"));
        if(!interface->getParameter)
            interface->getParameter = (int* (*)(const char*))(dlsym(interface->dll, "getParameter"));
#endif
        if(!interface->init || !interface->ende || !interface->getParameter || !interface->setParameter)
        {
            free(interface);
            return NULL;
        }
    }
    else
    {
        free(interface);
        return NULL;
    }
    return interface;

}

void interface_close(SInterface* interface)
{
    if(interface->dll)
    {
#ifdef _WIN32
        FreeLibrary(interface->dll);
#else
        dlclose(interface->dll);
#endif
        memset(interface, 0, sizeof(SInterface));
        free(interface);
    }
}
