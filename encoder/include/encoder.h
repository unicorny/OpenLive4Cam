#ifndef __OPEN_LIVE_4_CAM_ENCODER_LIB__
#define __OPEN_LIVE_4_CAM_ENCODER_LIB__

#include "../../interface/interface.h"
#include "../../interface/frame.h"

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "x264.h"


#define MAX_PARAMETER_COUNT 8
const char g_modulname[] = "encoder";


#ifdef _WIN32
    #ifdef BUILD_DLL
        //#define ENCODER_API __declspec(dllexport)
                #define ENCODER_API extern
    #else
        //#define ENCODER_API __declspec(dllimport)
		#define ENCODER_API
    #endif
#else
    #define ENCODER_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

ENCODER_API int  init();
ENCODER_API void ende();
ENCODER_API void setParameter(const char* name, int value);
ENCODER_API int  getParameter(const char* name);
//return 0 okay, -1 camera not open
ENCODER_API int start();
ENCODER_API int stop();

ENCODER_API unsigned char* getFrame(int *size);
ENCODER_API int encodeFrame();

#ifdef __cplusplus
}
#endif


#endif //__OPEN_LIVE_4_CAM_ENCODER_LIB__