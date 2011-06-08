#ifndef __LIVE_STREAM_4_CAM_INTERFACE__
#define __LIVE_STREAM_4_CAM_INTERFACE__

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif //_WIN32



#ifdef __cplusplus
extern "C"
{
#endif

typedef struct SInterface{
#ifdef _WIN32
    HMODULE dll;
#else
    void* dll;
#endif
    int (*init)(void);
    void (*ende)(void);
    void (*setParameter)(const char*,int);
    int (*getParameter)(const char*);
    int (*start)(void);
    int (*stop)(void);
} SInterface;

/*!
 * bei rgb = 1:
 * channel1 = red, channel2 = green, channel3 = blue
 * bei rgb = 0
 * channel1 = Y, channel2 und channel3 nur halb so groß (YUV 4:2:0)
 * channel2 = U, channel3 = V
 * width = breite in pixeln
 * height = höhe in pixeln
 * pixelsize = size of one pixel in bytes
 */
typedef struct SPicture{
    int rgb;
    unsigned char* channel1;
    unsigned char* channel2;
    unsigned char* channel3;
    int width;
    int height;
    int pixelsize;    
} SPicture;

//! \return 0 alles okay, -1 bei fehler
struct SInterface* interface_loadDll(const char* dllname);
void interface_close(struct SInterface* in);

#ifdef __cplusplus
}
#endif

#endif //__LIVE_STREAM_4_CAM_INTERFACE__
