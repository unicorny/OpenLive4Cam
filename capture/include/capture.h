#ifndef __CAPTURE_LIB__
#define __CAPTURE_LIB__

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv/cv.h>
#include <opencv2/opencv.hpp>


#include <iostream>
#include <ctype.h>

#include "../../interface/picture.h"

using namespace cv;
using namespace std;

struct Config
{
    Config()
    : cameraNr(0), resolutionNr(0), width(-1), height(-1) {}
    int cameraNr;
    int resolutionNr;
    int width;
    int height;    
        
};

#define MAX_PARAMETER_COUNT 8
const char g_modulname[] = "capture";


#ifdef _WIN32
#define CAPTURE_API
    #ifdef BUILD_DLL
		#undef CAPTURE_API
		#define CAPTURE_API extern
	#endif
	#ifdef VC_BUILD
		#undef CAPTURE_API
		#define CAPTURE_API __declspec(dllexport)
	#endif
#else
	#ifdef BUILD_DLL
		#define CAPTURE_API extern
	#else	
		#define CAPTURE_API
	#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

CAPTURE_API int  init();
CAPTURE_API void ende();
CAPTURE_API void setParameter(const char* name, int value);
CAPTURE_API int  getParameter(const char* name);
//return 0 okay, -1 camera not open
CAPTURE_API int start();
CAPTURE_API SPicture* getPicture(int rgb = 0, int removeFrame = 1);
CAPTURE_API int stop();


//CAPTURE_API 


#ifdef __cplusplus
}
#endif

#endif //  __CAPTURE_LIB__

