#include "encoder.h"
#include "../../interface/mutex.h"

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

int getStackCount();

//! \brief pointer to capture-modul (shared library)
SInterface* capture;
//! \brief pointer to getPictureFunc, used in raw_input
SPicture* (*getPictureFunc)(int,int);
//! \brief save datas used for encoding
encoder_datas en_data;
//! \brief pointer to stack for saving encoded frames
SFrame_stack* g_FrameBuffer = NULL;
int g_run = 0;
int port = 0;
//! brief mutex for threadsafe
Mutex* mutex;


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
    mutex = mutex_init();
    if(!mutex)
    {
        printf("encoder.init Fehler bei mutex init\n");
        return -4;
    }

    return 42;
}

//! \brief call stop, free mutex, clear stack, free capture-Modul
void ende()
{
    if(mutex_lock(mutex))
        printf("encoder.ende Fehler beu mutex_lock 1\n");
    g_run = 0;
    if(mutex_unlock(mutex))
        printf("encoder.ende Fehler bei mutex_unlock 1\n");
    stop();
    if(mutex_lock(mutex))
        printf("encoder.ende Fehler bei mutex_lock 2\n");
    //löschen des letzten Frames
    getFrame(NULL, NULL);
    if(mutex_unlock(mutex))
        printf("encoder.ende Fehler bei mutex_unlock 2\n");

    mutex_close(mutex);
    mutex = NULL;
 
    printf("Frames on Stack: %d\n", count_stack(g_FrameBuffer));
    clear_stack(g_FrameBuffer);
    if(capture)
    {
        capture->ende();
        interface_close(capture);
    }
    capture = NULL;
    printf("encoder.ende Encoder Modul ende\n");
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
 * <tr><td>encoder.getStackCountFunc</td></tr>int (*getStackCount)(void)</td><td>
 *    - return Frame Count on Frame Stack</td></tr>
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
    else if(strcmp(name, "encoder.getStackCountFunc") == 0)
        return (int)getStackCount;
    else if(strcmp(name, "encoder.port") == 0)
        return port;
    
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
    
    if(mutex_lock(mutex))
        printf("encoder.start Fehler bie mutex_lock\n");
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
    if(mutex_unlock(mutex))
        printf("encoder.start Fehler bei mutex_unlock\n");
    printf("encoder::start return von start: %d\n", r);
    return r;
}
//! \brief encode one frame and safe it to stack, threadsafe
//! \return 0 for okay
//! \return 1 fur stop
//! \return -1 if encode_frames() failed
int encodeFrame()
{
    if(mutex_lock(mutex))
        printf("encoder.encodeFrame Fehler bei mutex_lock\n");
    if(!g_run)
    {
        encoder_stop_frames();
        if(mutex_unlock(mutex))
            printf("encoder.encodeFrame Fehler bei mutex_unlock 1\n");        
        return 1;
    }
    if(encode_frames())
    {
        //printf("encoder: Fehler bei encode_frames\n");
        g_run = 0;
        encoder_stop_frames();
        if(mutex_unlock(mutex))
            printf("encoder.encodeFrame Fehler bei mutex_unlock 2\n");
        return -1;
    }
  /*  while(getStackCount() > 20)
    {
      //  stack_delete_top(g_FrameBuffer);
        
    }
   //*/     
    if(mutex_unlock(mutex))
            printf("encoder.encodeFrame Fehler bei mutex_unlock 3\n");
    return 0;
}

int getStackCount()
{
    //if(!g_FrameBuffer) return 0;
    return count_stack(g_FrameBuffer);
}

//! \brief stop encoder and capture
//! \return 0
int stop()
{
    if(capture) capture->stop();
 //   return 0;
    
    if(mutex_lock(mutex))
        printf("encoder.stop Fehler bei mutex_lock\n");
    g_run = 0;
    if(mutex_unlock(mutex))
        printf("encoder.stop Fehler bei mutex_unlock\n");
    /*printf("en_data.h: %d\n", (int)en_data.h);
     if( en_data.h )
     {
        x264_encoder_close( en_data.h );
        en_data.h = NULL;
     }
     * */
  
    return 0;
}


