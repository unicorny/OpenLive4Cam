#ifndef __CAPTURE_LIB__
#define __CAPTURE_LIB__

#ifdef _WIN32
    #ifdef BUILD_DLL
        #define CAPTURE_API __declspec(dllexport)
    #else
        #define CAPTURE_API __declspec(dllimport)
    #endif
#else
    #define CAPTURE_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

CAPTURE_API int  init();
CAPTURE_API void ende();
CAPTURE_API void setParameter(const char* name, int value);
CAPTURE_API int  getParameter(const char* name);


#ifdef __cplusplus
}
#endif

#endif //  __CAPTURE_LIB__

