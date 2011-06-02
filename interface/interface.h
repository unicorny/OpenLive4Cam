
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif //_WIN32

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
#ifdef _WIN32
    HMODULE dll;
#else
    void* dll;
#endif
    int (*init)(void);
    void (*exit)(void);
    void (*setParameter)(const char*,int);
    int (*getParameter)(const char*,int);
	
} SInterface;

//! \return 0 alles okay, -1 bei fehler
int interface_loadDll(const char* dllname);
void interface_close();

    

#ifdef __cplusplus
}
#endif
