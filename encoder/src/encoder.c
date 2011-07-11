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

SInterface* capture;
SPicture* (*getPictureFunc)(int,int);
encoder_datas en_data;
SFrame_stack* g_FrameBuffer = NULL;
int g_run = 0;
#ifdef _WIN32
void* mutex = NULL;
#else
pthread_mutex_t* mutex; 
#endif

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


