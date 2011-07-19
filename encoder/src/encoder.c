#include "encoder.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#ifdef _WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to proces
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread

            break;
    }
    return TRUE; // succesful
}
#endif //_WIN32

//! \brief pointer to capture-modul (shared library)
SInterface* capture;
//! \brief pointer to getPictureFunc, used in raw_input
SPicture* (*getPictureFunc)(int,int);
//! \brief save datas used for encoding
encoder_datas en_data;
//! \brief pointer to stack for saving encoded frames
SFrame_stack* g_FrameBuffer = NULL;
int g_run = 0;
//! brief mutex for threadsafe
#ifdef _WIN32
void* mutex = NULL;
#else
pthread_mutex_t* mutex; 
#endif

//! \brief lock mutex of encoder, or wait until it is unlocked and lock it than
//! \return 0 without error
//! \return 1 by error in windows
//! \return return-value of pthread_mutex_lock(pthread_mutex_t*) in linux
int lock()
{
	int ret = 1;
#ifdef _WIN32
	DWORD waitResult = WaitForSingleObject(mutex, INFINITE);
	if(waitResult == WAIT_OBJECT_0)
		ret = 0;
	else if(waitResult == WAIT_FAILED)
		printf("Fehler %d bei lock mutex\n", GetLastError());

#else
    ret = pthread_mutex_lock(mutex);
#endif
    if(ret)
        printf("capture.lock_mutex fehler bei lock mutex\n");
    return ret;        
}

//! \brief unlock mutex of encoder
//! \return 0 without error
//! \return 1 by error in windows
//! \return return-value of pthread_mutex_unlock(pthread_mutex_t*) in linux
int unlock()
{
#ifdef _WIN32
	int ret = !ReleaseMutex(mutex);
#else
    int ret = pthread_mutex_unlock(mutex);
#endif
    
    if(ret)
        printf("capture.unlock_mutex fehler bei unlock mutex\n");
    return ret;
}

//! \brief load capture-Modul (Shared Library, *.so or *.dll), init mutex
//! \return -1 if capture open failed
//! \return -2 if capture.init failed
//! \return -3 if capture hasn't a getPictureFunc
//! \return -4 if mutex open failed
//! \return 42 (no problem)
int init()
{	
    printf("encoder Modul init\n");
#ifdef _WIN32
        capture = interface_loadDll("capture.dll");
#else
    capture = interface_loadDll("libcapture.so");
#endif
    if(!capture)
    {
        printf("encoder::init Fehler beim laden des Capture Modules\n");
        return -1;
    }
    if(capture->init() < 0)
    {
        printf("encoder::init Fehler beim initalisieren von Capture!\n");
        return -2;
    }
    getPictureFunc = (SPicture* (*)(int, int))capture->getParameter("capture.getPictureFunc");
    if(!getPictureFunc)
    {
        printf("encoder::init Fehler, bei Aufruf von capture.getPictureFunc!\n");
        return -3;
    }
    en_data.h = NULL;
#ifdef _WIN32
	mutex = CreateMutex(NULL, FALSE, NULL);
	if(mutex == NULL)
	{
		printf("encoder.init Fehler: %d bei mutex init\n", GetLastError());
		return -4;
	}
#else
	mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if(pthread_mutex_init(mutex, NULL))
    {
        printf("encoder.init fehler bei mutex init\n");
        return -4;
    }
#endif
    return 42;
}

//! \brief call stop, free mutex, clear stack, free capture-Modul
void ende()
{
    lock();
    g_run = 0;
    unlock();
    stop();
    lock();
    //löschen des letzten Frames
    getFrame(NULL);
    unlock();
#ifdef _WIN32
	CloseHandle(mutex);
#else
    pthread_mutex_destroy(mutex);
	free(mutex);
#endif
 
    clear_stack(g_FrameBuffer);
    if(capture)
    {
        capture->ende();
        interface_close(capture);
    }
    capture = NULL;
    printf("Encoder Modul ende\n");
}


/*! \brief set parameter for encoder 
 * 
 * no parameter to set, only forwarding function-call to capture-Modul
 *  
 * \param name parameter name
 * \param value parameter value (number or pointer)
 */
void setParameter(const char* name, int value)
{
    char buffer[256];
    sprintf(buffer, "%s", name);    
  
    char * pch;
    pch = strtok (buffer, ".\0");
    /*int count = 0;
    while (pch != NULL)
    {
        g_Parameters[count++] = pch;
        pch = strtok (NULL, ".\0");
        
    }
        */
    if(strcmp(pch, g_modulname) != 0 && capture)
            capture->setParameter(name, value); //TODO: weiterleiten    
        
}
/*! \brief get parameter from encoder 
 * 
 * <br>
 * <table><tr><th colspan='3'>Possible Parameters to get:</th></tr>
 * <tr><th>name</th><th>return</th><th>description</th></tr>
 * <tr><td>encoder.getFrameFunc</td><td>unsigned char* (*getFrameFunc)(int*)</td><td>
 *    - return pointer to getFrame()</td></tr>
 * <tr><td>encoder.EncodeFrameFunc</td><td>int (*encodeFrame)(void)</td><td>
 *    - return Pointer to encodeFrame()</td></tr>
 * <tr><td><i>all other</i></td><td><i>type</i></td><td>
 *    - forwarding to capture </td></tr>
 * </table>
 * <br>
 *  
 * \param name parameter name
 * \param value parameter value (number or pointer)
 * \return value or pointer to value if succeed
 * \return NULL if parameter is unknown
 */
int getParameter(const char* name)
{
    char buffer[256];
    sprintf(buffer, "%s", name);    
 //   printf("encoder.getParameter: name = %s\n", name);
  
    char * pch;
    pch = strtok (buffer, ".\0");
   /* int count = 0;
    while (pch != NULL)
    {
        g_Parameters[count++] = pch;
        pch = strtok (NULL, ".\0");
        
    }*/
    if(strcmp(pch, g_modulname) != 0)
    {
      // printf("pch: %s\n", pch); 
        if(capture)
            return capture->getParameter(name);
        //TODO: weiterleiten
        return 0;
    }
    else if(strcmp(name, "encoder.getFrameFunc") == 0)
        return (int)getFrame;    
    else if(strcmp(name, "encoder.EncodeFrameFunc") == 0)
        return (int)encodeFrame;
    
    return 0;
  
}
//! \brief startet capture, init data for encoding
//! \return return-value of start_x264(), threadsafe
int start()
{
    char resolution[256];
     
    if(capture) capture->start();
    sprintf(resolution, "%dx%d", capture->getParameter("capture.resolution.x"), capture->getParameter("capture.resolution.y"));
    printf("resolution: %s\n", resolution);
    
    lock();
    //reset capture data
    en_data.last_dts = 0;
    en_data.prev_dts = 0;
    en_data.first_dts = 0;
    en_data.i_frame = 0;
    en_data.i_frame_size;
    en_data.i_end, en_data.i_previous = 0, en_data.i_start = 0;
    en_data.i_file = 0;
    en_data.largest_pts = -1;
    en_data.second_largest_pts = -1;
    en_data.pts_warning_cnt = 0;
    en_data.i_frame_output = 0;
   
    int r = start_x264(resolution);
    g_run = 1;
    unlock();
    printf("encoder::start return von start: %d\n", r);
    return r;
}
//! \brief encode one frame and safe it to stack, threadsafe
//! \return 0 for okay
//! \return -1 if encode_frames() failed
int encodeFrame()
{
    lock();
    if(!g_run)
    {
        unlock();
        return 1;
    }
    if(encode_frames())
    {
        printf("encoder: Fehler bei encode_frames\n");
        unlock();
        return -1;
    }
    unlock();
    return 0;
}

//! \brief stop encoder and capture
//! \return 0
int stop()
{
    lock();
    g_run = 0;
    encoder_stop_frames();
    unlock();
    /*printf("en_data.h: %d\n", (int)en_data.h);
     if( en_data.h )
     {
        x264_encoder_close( en_data.h );
        en_data.h = NULL;
     }
     * */
    if(capture) capture->stop();
    return 0;
}


