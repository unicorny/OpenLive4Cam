#ifndef __OPEN_LIVE_4_CAM_SERVER_LIB__
#define __OPEN_LIVE_4_CAM_SERVER_LIB__

#include <iostream>
#include <ctype.h>
#include <cstring>
#include <stack>
#include <stdio.h>

#include "interface.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

using namespace std;
#define MAX_PARAMETER_COUNT 8
const char g_modulname[] = "server";


#ifdef _WIN32
    #ifdef BUILD_DLL
        //#define SERVER_API __declspec(dllexport)
		#define SERVER_API extern
    #else
        //#define SERVER_API __declspec(dllimport)
		#define SERVER_API
    #endif
#else
    #define SERVER_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

SERVER_API int  init();
SERVER_API void ende();
SERVER_API void setParameter(const char* name, int value);
SERVER_API int  getParameter(const char* name);
SERVER_API int start();
SERVER_API int stop();
SERVER_API int run();

 


#ifdef __cplusplus
}
#endif

#endif //  __OPEN_LIVE_4_CAM_SERVER_LIB__



