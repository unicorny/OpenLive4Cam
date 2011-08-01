#ifndef __CAPTURE_LIB__
#define __CAPTURE_LIB__

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv/cv.h>
#include <opencv2/opencv.hpp>


#include <iostream>
#include <ctype.h>
#include <stack>

#include "../../interface/picture.h"
#include "../../interface/mutex.h"

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
extern std::stack<std::string> g_Messages;


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
//! \brief init mutex and Picture structures
//! \return -2 bei mutex init error, else 42 (no problems)
CAPTURE_API int  init();
//! \brief release pictures, release mutex (wait if mutex is currently locked)
CAPTURE_API void ende();
/*! \brief set parameter for capture 
 * Possible Parameters to set:
 * 'capture.camera.choose', value is a int
 *    value is used to deside which camera will be choosen
 *    (if they are more then one camera connected to the computer)
 *    working with stream activated
 * '
 * 
 * \params name name of parameter
 * \params value of parameter (number or pointer)
*/
CAPTURE_API void setParameter(const char* name, int value);
CAPTURE_API int  getParameter(const char* name);
//return 0 okay, -1 camera not open
CAPTURE_API int start();
CAPTURE_API SPicture* getPicture(int rgb = 0, int removeFrame = 1);
CAPTURE_API int stop();


//CAPTURE_API 
//! \brief mutex for threadsave working
extern Mutex* mutex;


#ifdef __cplusplus
}
#endif

#endif //  __CAPTURE_LIB__

