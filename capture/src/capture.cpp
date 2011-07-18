#include "capture.h"
#include "camera.h"
#include <stdio.h>
#include <stack>
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h> 
#endif

#include "../../interface/interface.h"
//#include "../../interface/picture.h"



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

//! \brief to split parameter string and save result as array (used by getParameter())
std::string g_Parameters[MAX_PARAMETER_COUNT];
//! \brief save error-messages for getParameter("getLastMessage")
std::stack<std::string> g_Messages;

//! \brief mutex for threadsave working
#ifdef _WIN32
void* mutex = NULL;
#else
pthread_mutex_t* mutex = NULL;//PTHREAD_MUTEX_INITIALIZER; 
#endif

//! \brief buffer for last error message, used by getParameter()
char g_MessagesBuffer[256];
//! \brief save running state of stream
bool g_run = false;

//! \brief save configuration of camera (number, resolution)
Config g_cfg;
//! \brief object with access to the camera, used by getPicture()
VideoCapture g_capture;
//! \brief buffer for rgb-Picture
SPicture g_rgbPicture;
//! \brief buffer for yuv-Picture
SPicture g_yuvPicture;

//! \brief init mutex and Picture structures
//! \return -2 bei mutex init error
//! \return 42 (no problems)
int init()
{	
    picture_init(&g_rgbPicture);
    picture_init(&g_yuvPicture);
    g_rgbPicture.rgb = 1;
    g_yuvPicture.rgb = 0;
    printf("Capture Modul init!\n");
#ifdef _WIN32
	mutex = CreateMutex(NULL, FALSE, NULL);
	if(mutex == NULL)
	{
		printf("capture.init Fehler: %d bei mutex init\n", GetLastError());
		return -2;
	}
#else
	mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if(pthread_mutex_init(mutex, NULL))
    {
        printf("capture.init fehler bei mutex init\n");
        return -2;
    }
#endif

    
    return 42;
}

//! \brief release pictures, release mutex
//!
//! <b>Important!</b> call stop before ende
void ende()
{
    picture_release(&g_rgbPicture);
    picture_release(&g_yuvPicture);
    printf("capture.ende: called\n");
#ifdef _WIN32
	CloseHandle(mutex);
#else
    pthread_mutex_destroy(mutex);
	free(mutex);
#endif
}
//! \brief lock mutex of capture, or wait until it is unlocked and lock it than
//! \param mutex pointer to mutex (in this case the global var mutex)
//! \return 0 without error
//! \return 1 by error in windows
//! \return return-value of pthread_mutex_lock(pthread_mutex_t*) in linux
int lock_mutex(void* mutex)
{
	int ret = 1;
#ifdef _WIN32
	DWORD waitResult = WaitForSingleObject(mutex, INFINITE);
	if(waitResult == WAIT_OBJECT_0)
		ret = 0;
	else if(waitResult == WAIT_FAILED)
		printf("Fehler %d bei lock mutex\n", GetLastError());

#else
    ret = pthread_mutex_lock((pthread_mutex_t*)mutex);
#endif
    if(ret)
        printf("capture.lock_mutex fehler bei lock mutex\n");
    return ret;        
}
//! \brief unlock mutex of capture
//! \param mutex pointer to mutex (in this case the global var mutex)
//! \return 0 without error
//! \return 1 by error in windows
//! \return return-value of pthread_mutex_unlock(pthread_mutex_t*) in linux
int unlock_mutex(void* mutex)
{
#ifdef _WIN32
	int ret = !ReleaseMutex(mutex);
#else
    int ret = pthread_mutex_unlock((pthread_mutex_t*)mutex);
#endif
    
    if(ret)
        printf("capture.unlock_mutex fehler bei unlock mutex\n");
    return ret;
}
/*! \brief set parameter for capture, threadsave 
 * 
 * by function call, mutex will be locked or if already locked, call wait until mutex is unlocked 
 * <br>
 * <table><tr><th colspan='3'>Possible Parameters to set:</th></tr>
 * <tr><th>name</th><th>value</th><th>description</th></tr>
 * <tr><td>capture.camera.choose</td><td>int</td><td>
 *    - value is used to deside which camera will be choosen
 *     (if they are more then one camera connected to the computer)<br>
 *    - working with active stream</td></tr>
 * <tr><td>capture.resolution.x<br>capture.resolution.width</td>
 *     <td>int</td><td>
 *     - try to set width of camera resolution</td></tr>
 * <tr><td>capture.resolution.y<br>capture.resolution.height</td>
 *     <td>int</td><td>
 *     - try to set height of camera resolution</td></tr>     
 * </table>
 * <br>
 *  
 * \param name parameter name
 * \param value parameter value (number or pointer)
 */
void setParameter(const char* name, int value)
{
    lock_mutex(mutex);
    //printf("capture.setParameter: name: %s, value: %d\n", name, value);
    if(string(name) == string("capture.camera.choose"))
    {
      //  printf("Kamera Nummer: %d ausgewaehlt\n", value);
        if(g_run && (value != g_cfg.cameraNr || value == -1))
        {
            printf("value: %d", value);
            g_run = false;
            if(g_capture.isOpened())
                g_capture.release();
            g_capture.open(value);
            if(!g_capture.isOpened())
            {
                g_capture.open(g_cfg.cameraNr);
                g_run = true;
                unlock_mutex(mutex);
                return;
            }
            g_run = true;
        }
        g_cfg.cameraNr = value;
        g_capture.set(CV_CAP_PROP_FRAME_WIDTH, g_cfg.width);
        g_capture.set(CV_CAP_PROP_FRAME_HEIGHT, g_cfg.height);
    }
    if(!g_run)
    {
        if(string(name) == string("capture.resolution.x") || 
        string(name) == string("capture.resolution.width"))
            g_cfg.width = value;
        else if(string(name) == string("capture.resolution.y")||
                string(name) == string("capture.resolution.height"))
            g_cfg.height = value;
    }
    unlock_mutex(mutex);
        
}
/*! \brief get parameter from capture 
 * 
 * 
 * <br>
 * <table><tr><th colspan='3'>Possible Parameters to get:</th></tr>
 * <tr><th>name</th><th>return</th><th>description</th></tr>
 * <tr><td>getLastMessage</td><td>char[256]</td><td>
 *    - return pointer to char-buffer with last error-message from capture</td></tr>
 * <tr><td>capture.getPictureFunc</td>
 *     <td>(SPicture* (*)(int, int))</td><td>
 *     - return pointer to getPicture(int rgb, int removeFrame)</td></tr>
 * <tr><td>capture.camera.count</td><td>int</td><td>
 *     - return number of cameras currently connected to the Computer</td></tr>
 * <tr><td>capture.camera.x.name</td><td>char[]</td><td>
 *     - return name of camera x, where x is a number between 0 and capture.camera.count</td></tr>
 * <tr><td>capture.camera.x.resolution.count</td><td>int</td><td>
 *     - return number of resolutions for camera x (where x is a number between 0 and capture.camera.count)</td></tr>
 * <tr><td>capture.camera.i.resolution.j.x<br>capture.camera.i.resolution.j.width</td><td>int</td><td>
 *     - return resolution width of camera i, of resolution j 
 *      (where i is a number between 0 and capture.camera.count 
 *       and j is a number between 0 and capture.camera.i.resolution.count)</td></tr>
 * <tr><td>capture.camera.i.resolution.j.y<br>capture.camera.i.resolution.j.height</td><td>int</td><td>
 *     - return resolution height of camera i, of resolution j 
 *      (where i is a number between 0 and capture.camera.count 
 *       and j is a number between 0 and capture.camera.i.resolution.count)</td></tr>
 * <tr><td>capture.resolution.x<br>capture.resolution.width</td>
 *     <td>int</td><td>
 *     - return current width of camera resolution</td></tr>
 * <tr><td>capture.resolution.y<br>capture.resolution.height</td>
 *     <td>int</td><td>
 *     - return current height of camera resolution</td></tr>     
 * </table>
 * <br>
 *  
 * \param name parameter name
 * \return parameter value (number or pointer) if succeed
 * \return NULL if parameter is unknown
 */
int getParameter(const char* name)
{
    lock_mutex(mutex);
   // printf("Name: %s\n", name);
    char buffer[256];
    sprintf(buffer, "%s", name);
    
  
    char * pch;
    pch = strtok (buffer, ".\0");
    int count = 0;
    while (pch != NULL)
    {
        g_Parameters[count++] = pch;
        pch = strtok (NULL, ".\0");
        
    }
    if(g_Parameters[0] == string("getLastMessage"))
    {
        if(g_Messages.size())
        {
          sprintf(g_MessagesBuffer,"capture: %s", g_Messages.top().data());
          g_Messages.pop();
          unlock_mutex(mutex);
          return (int)g_MessagesBuffer;  
        }
        else
        {
            unlock_mutex(mutex);
            return 0;
        }
    }
    else if(g_Parameters[0] != string(g_modulname))
    {
            //TODO: weiterleiten
            unlock_mutex(mutex);
            return 0;
    }
    if(g_Parameters[1] == string("camera"))
    {
            return camera_getParameter(&g_Parameters[2]);
    }
    else if(g_Parameters[1] == string("getPictureFunc") )
    {   
        unlock_mutex(mutex);
        return (int)getPicture;
    }
    else if(string(name) == string("capture.resolution.x") ||
            string(name) == string("capture.resolution.width"))
    {
        
        
        int width = g_cfg.width;
        unlock_mutex(mutex);
        return width;
    }
    else if(string(name) == string("capture.resolution.y") ||
            string(name) == string("capture.resolution.height"))
    {
        int height = g_cfg.height;    
        unlock_mutex(mutex);        
        return height;
    }
        
    
  /*  for(int i = 0; i < MAX_PARAMETER_COUNT; i++)
    {
        printf("%d: %s\n", i, g_Parameters[i].data());
    }*/
    return 0;
  
}
//! \brief start capturing, threadsafe
//!
//! open choosen camera or if not choosen, default camera
//! \return 0 okay
//! \return -7 if camera not open
int start()
{
    lock_mutex(mutex);
    g_capture.open(g_cfg.cameraNr);    
    if(!g_capture.isOpened())  // check if we succeeded
    {
        //printf("Kamera konnte nicht geöffnet werden!");
        g_Messages.push(string("Fehler, Kamera konnte nicht geoeffnet werden!"));
        unlock_mutex(mutex);
        return -7;
    }
    
    g_capture.set(CV_CAP_PROP_FRAME_WIDTH, g_cfg.width);
    g_capture.set(CV_CAP_PROP_FRAME_HEIGHT, g_cfg.height);
    
    Mat temp;
    g_capture >> temp;
    g_cfg.width = temp.cols;
    g_cfg.height = temp.rows;
    
    g_run = true;
    printf("start called\n");

   // namedWindow("LIVE",1);
    unlock_mutex(mutex);
    return 0;
}

/*! \brief return current camera picture, as rgb or yuv picture, threadsave
 * 
 * \param rgb if set to zero (default), yuv-Picture will return, <br>
 *            if set to other value, rgb-Picture will return
 * \param removeFrame if set to 1 (default) the next picture will be catch <br>
 *                    if set to zero, the last picture will be returned
 * \return SPicture pointer to lokal buffer, which didn't change until the next call from getPicture with
 *         the same rgb-parameter
 * \return NULL if any error occured
 */
SPicture* getPicture(int rgb/* = 0*/, int removeFrame/* = 1*/)
{
    lock_mutex(mutex); 
    static int count = 0;
    // if start wasn't called
    if(!g_run)
    {
        unlock_mutex(mutex);
        return 0;
    }
    // try to open capture
    else if(!g_capture.isOpened())
    {
         g_Messages.push(string("getPicture error, weil keine Kamera geoeffnet ist!"));
         unlock_mutex(mutex);
         return 0;
    }
    
    //get next or last picture
    Mat m, m2, m3;
    if(!removeFrame)
        g_capture.retrieve(m);
    else
        g_capture >> m; // get a new frame from camera
    //m = cvLoadImage("test.jpg");
    if(m.depth() != CV_8U)
    {
        printf("Error, depth != unsigned char\n");
        unlock_mutex(mutex);
        return 0;
    }
    //m.convertTo(m2, )
    //Scale Picture to choosen resolution (if camera didn't support it)
    Mat matrices[4];    
    IplImage src = m;
    IplImage* scaled = cvCreateImage(cvSize(g_cfg.width, g_cfg.height), IPL_DEPTH_8U, 3);
    cvResize( &src, scaled, CV_INTER_LINEAR );
    
    //rgb-output 
    if(rgb)
    {        
        //imshow("LIVE", scaled);
        split(scaled, matrices);
        matrices[3] = matrices[0].clone();
        matrices[3] = Scalar(255);
        merge(matrices, 4, m2);
        
        //get current buffer size and required buffer size
        int oldSize = picture_getSize(&g_rgbPicture);
        g_rgbPicture.width = m2.cols;
        g_rgbPicture.height = m2.rows;
        int newSize = picture_getSize(&g_rgbPicture);  
        
        //compare buffer size and picture size, and make new buffer, if picture size differ
        if(oldSize != newSize)
        {
            picture_release(&g_rgbPicture);
            if(picture_create(&g_rgbPicture, m2.cols, m2.rows, 4))
            {
                printf("Fehler beim speicher reservieren in getPicture rgb!\n");
                unlock_mutex(mutex);
                return NULL;
            }
        }
        
        //return 0;
        //copy picture to buffer
        size_t size = m2.cols*m2.rows*4;
        memcpy(g_rgbPicture.channel1, m2.data, size);
        
        //write pictures as jpg for test
        char filename[256];
        sprintf(filename, "picture%04d.jpg", count++);
        imwrite(filename, scaled);
        
        //free scaled image
        cvReleaseImage(&scaled);
        
        unlock_mutex(mutex);
        //return pointer to picture buffer
        return &g_rgbPicture;
        
    }
    // yuv-Output
    else
    {
        //convert and split picture
        cvtColor(scaled, m2, CV_BGR2YCrCb);
        split(m2, matrices);
        
        IplImage* U = cvCreateImage(cvSize(scaled->width/2, scaled->height/2), IPL_DEPTH_8U, 1);
        IplImage* V = cvCreateImage(cvSize(scaled->width/2, scaled->height/2), IPL_DEPTH_8U, 1);
        IplImage uSrc = matrices[1];
        IplImage vSrc = matrices[2];
        //create resized u and v pictures (half-size)
        cvResize(&uSrc, U, CV_INTER_LINEAR);
        cvResize( &vSrc, V, CV_INTER_LINEAR );
    //  imshow("Y", matrices[0]);
      //imshow("U", U);
      //imshow("V", V);
        
        //get current buffer size and required buffer size
        int oldSize = picture_getSize(&g_yuvPicture);
        g_yuvPicture.width = scaled->width;
        g_yuvPicture.height = scaled->height;
        int newSize = picture_getSize(&g_yuvPicture);

        //compare buffer size and picture size, and make new buffer, if picture size differ
        if(oldSize != newSize)
        {
            picture_release(&g_yuvPicture);
            if(picture_create(&g_yuvPicture, m2.cols, m2.rows, 1))
            {
                printf("Fehler beim speicher reservieren in getPicture yuv!\n");
                unlock_mutex(mutex);
                return 0;
            }
        }
        //return 0;
        //copy channels
        size_t size = m2.cols*m2.rows;
        memcpy(g_yuvPicture.channel1, matrices[0].data, size);
        memcpy(g_yuvPicture.channel2, V->imageData, size/4);
        memcpy(g_yuvPicture.channel3, U->imageData, size/4);
        
        //release u and v pictures
        cvReleaseImage(&U);
        cvReleaseImage(&V);
        
        unlock_mutex(mutex);
        //return pointer to picture buffer
        return &g_yuvPicture;        
    }
    unlock_mutex(mutex);
    return NULL;
}
//! \brief stop capture, threadsafe
//!
//! close camera and set run to false
//! \return 0
int stop()
{
    lock_mutex(mutex);
    g_run = false;
    g_capture.release();
    unlock_mutex(mutex);
    return 0;
}

