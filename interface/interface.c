#include "malloc.h"
#include "memory.h"
#include "interface.h"


/*
    int (*init)(void);
    void (*exit)(void);
    void (*setParameter)(const char*,int);
    int (*getParameter)(const char*);
    int (*start)(void);
    int (*stop)(void);
  */

#ifdef _WIN32
//extern void *interface_loadFunction
#else
extern void *interface_loadFunction(void *__handle,
                                    __const char *__name)
{
    return dlsym(__handle, __name);
}

#endif


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
        if(!interface->init)
            interface->init = (int (*)())(interface_loadFunction(interface->dll, "init"));
        if(!interface->ende)
            interface->ende = (void (*)())(interface_loadFunction(interface->dll, "ende"));
        if(!interface->start)
            interface->start = (int (*)())(interface_loadFunction(interface->dll, "start"));
        if(!interface->stop)
            interface->stop = (int (*)())(interface_loadFunction(interface->dll, "stop"));
        if(!interface->setParameter)
            interface->setParameter = (void (*)(const char*,int))(interface_loadFunction(interface->dll, "setParameter"));
        if(!interface->getParameter)
            interface->getParameter = (int* (*)(const char*))(interface_loadFunction(interface->dll, "getParameter"));

        if(!interface->init || !interface->ende || !interface->start || !interface->stop || !interface->getParameter || !interface->setParameter)
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
