#ifndef __LIVE_STREAM_4_CAM_INTERFACE__
#define __LIVE_STREAM_4_CAM_INTERFACE__

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif //_WIN32

//! Makro to easy and save delete array (only array which are created with new)
#define SAVE_DELETE(X) if(X) {delete X; X = 0;}

#ifdef __cplusplus
extern "C"
{
#endif

//! \brief interface structur for access to other modul
//! 
//! structur contains pointer to dynamic loaded dll or so and 
//! function-pointer for standard functions of all modules
typedef struct SInterface{
#ifdef _WIN32
    //! dll HANDLE for windows
    HMODULE dll;
#else
    //! dll (so) pointer for linux
    void* dll;
#endif
    //! \brief pointer to init function
    //!
    //! init called only once by start of programm
    //! \return 42 (no problems)
    //! \return < 0 (error, the doku of modul)    
    int (*init)(void);
    
    //! \brief pointer to ende function
    //! 
    //! ende called only once by end of programm
    void (*ende)(void);
    
    //! \brief pointer to setParameter function
    //!
    //! call to set Paremeter for any connected modul
    //! \param char* name of parameter (with name of modul, dot-seperated list)
    //! \param int value or pointer to paremeter value
    void (*setParameter)(const char*,int);
    
    //! \brief pointer to getParemeter function
    //! 
    //! call to get Paremeter from any connected modul
    //! \param char* name of parameter (with name of modul, dot-seperated list)
    //! \return value or pointer to value (see modul doku)
    int (*getParameter)(const char*);
    
    //! \brief pointer to start function
    //! 
    //! called every time, the stream are startet 
    //! \return 0 (without error)
    //! \return < 0 (error, see modul-doku)     
    int (*start)(void);
    
    //! \brief pointer to stop function
    //! 
    //! called every time, the stream is stopped (start must be called bevor)
    //! \return 0
    int (*stop)(void);
} SInterface;

/*! \brief structur to save a picture in an independent format
 * 
 * contain picture in rgb-Format or in yuv-Format, depend on value of rgb
 * 
 */
typedef struct SPicture{
    //! for rgb = 1, SPicture contain a RGB-Picture<br>
    //! for rgb = 0, SPicture contain a YUV-Picture (YUV 4:2:0)
    int rgb;
    union
    {
        struct {
            //! point to rgb-Picture for rgb = 1 <br>
            //! point to Y-Picture for rgb = 0
            unsigned char* channel1;
            //! point to U-Picture for rgb = 0 with half size<br>
            //! NULL for rgb = 1
            unsigned char* channel2;
            //! point to V-Picture for rgb = 0 with half size<br>
            //! NULL for rgb = 1
            unsigned char* channel3;
        };
        unsigned char* channel[3];
    };
    //! width count of pixel (full size)
    int width;
    //! height count of pixel (full size)
    int height;
    //! size of pixel in byte (4 Byte for rgb = 1, 1 Byte for rgb = 0)
    int pixelsize;    
} SPicture;

/*! \brief contain one frame
 * 
 */
typedef struct SFrame{
    //! pointer to data
    unsigned char* data;
    //! size of datas in bytes
    int size;
} SFrame;

//! \brief create interface and try to load dll
//!
//! init also the function-pointers from interface
//! \param dllname name of dll- or so-File to load
//! \return Pointer to new Interface (created with malloc)
//! \return NULL if error occured by open dll or any of the interface functions
SInterface* interface_loadDll(const char* dllname);

//! \brief close Interface and free memory
//!
//! close dll and all function-pointers
//! \param in pointer to Interface created with interface_loadDll
void interface_close(SInterface* in);


#ifdef __cplusplus
}
#endif

#endif //__LIVE_STREAM_4_CAM_INTERFACE__
