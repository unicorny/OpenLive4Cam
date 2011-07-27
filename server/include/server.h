#ifndef __OPEN_LIVE_4_CAM_SERVER_LIB__
#define __OPEN_LIVE_4_CAM_SERVER_LIB__

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <iostream>
#include <ctype.h>
#include <cstring>
#include <stack>
#include <stdio.h>

#include "interface.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

#include "EncoderDeviceSource.hh"
#include "H264VideoEncoderServerMediaSubsession.h"

using namespace std;
#define MAX_PARAMETER_COUNT 8
const char g_modulname[] = "server";
extern bool g_run;
extern TaskScheduler* scheduler;
extern FramedSource* eds;


#ifdef _WIN32
#define SERVER_API
    #ifdef BUILD_DLL
		#undef SERVER_API
		#define SERVER_API extern
	#endif
	#ifdef VC_BUILD
		#undef SERVER_API
		#define SERVER_API __declspec(dllexport)
	#endif
#else
	#ifdef BUILD_DLL
		#define SERVER_API extern
	#else	
		#define SERVER_API
	#endif
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


extern unsigned char* (*getFrameFunc)(int*);

 


#ifdef __cplusplus
}
#endif

#endif //  __OPEN_LIVE_4_CAM_SERVER_LIB__



