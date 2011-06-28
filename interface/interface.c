#include <malloc.h>
#include <memory.h>
#include "interface.h"
#include <stdio.h>


/*
    int (*init)(void);
    void (*exit)(void);
    void (*setParameter)(const char*,int);
    int (*getParameter)(const char*);
    int (*start)(void);
    int (*stop)(void);
  */

#ifdef _WIN32
WINBASEAPI FARPROC WINAPI interface_loadFunction(HINSTANCE instance,LPCSTR lpcString)
{
    return GetProcAddress(instance, lpcString);
}

#else
extern void *interface_loadFunction(void *__handle,
                                    __const char *__name)
{
    return dlsym(__handle, __name);
}

#endif

struct SInterface* interface_loadDll(const char* dllname)
{
    struct SInterface* in = (SInterface*)malloc(sizeof(SInterface));
    memset(in, 0, sizeof(SInterface));
    //try to load from dll
    if(!in->dll)
    {
#ifdef _WIN32
        //in->dll = LoadLibraryA(dllname);
        in->dll = LoadLibrary(L".\\libcapture.dll");
#else
        in->dll = dlopen(dllname, RTLD_LAZY);
#endif
    }
    if(in->dll)
    {
        if(!in->init)
            in->init = (int (*)())(interface_loadFunction(in->dll, "init"));
        if(!in->ende)
            in->ende = (void (*)())(interface_loadFunction(in->dll, "ende"));
        if(!in->start)
            in->start = (int (*)())(interface_loadFunction(in->dll, "start"));
        if(!in->stop)
            in->stop = (int (*)())(interface_loadFunction(in->dll, "stop"));
        if(!in->setParameter)
            in->setParameter = (void (*)(const char*,int))(interface_loadFunction(in->dll, "setParameter"));
        if(!in->getParameter)
            in->getParameter = (int (*)(const char*))(interface_loadFunction(in->dll, "getParameter"));

        if(!in->init || !in->ende || !in->start || !in->stop || !in->getParameter || !in->setParameter)
        {
            free(in);
            return NULL;
        }
    }
    else
    {
        free(in);
        return NULL;
    }
    return in;

}
//
void interface_close(SInterface* in)
{
    if(in->dll)
    {
#ifdef _WIN32
        FreeLibrary(in->dll);
#else
        dlclose(in->dll);
#endif
        memset(in, 0, sizeof(SInterface));
        free(in);
        in = NULL;
    }
}
