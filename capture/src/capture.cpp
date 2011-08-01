#include "capture.h"
#include "camera.h"
#include <stdio.h>

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
Mutex* mutex = NULL;

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
    
    mutex = mutex_init();
    if(!mutex)
    {
        printf("capture.init Fehler bei mutex_init\n");
        return -2;
    }
    
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
    
    mutex_close(mutex);
    mutex = NULL;
	
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

    if(mutex_lock(mutex))
        g_Messages.push(string("setParameter</b> <font color='red'>Fehler bei mutex_lock</font>"));
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
                if(mutex_unlock(mutex))
                   g_Messages.push(string("setParameter</b> <font color='red'>Fehler bei mutex_unlock</font>"));    
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
    if(mutex_unlock(mutex))
                   g_Messages.push(string("setParameter</b> <font color='red'>Fehler bei mutex_unlock</font>"));    
        
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
    if(mutex_lock(mutex))
        printf("capture.getParameter Fehler bei lock_mutex\n");
    
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
          sprintf(g_MessagesBuffer,"<b>capture.%s", g_Messages.top().data());
          g_Messages.pop();
          if(mutex_unlock(mutex)) printf("capture.getParameter Fehler bei unlock_mutex 1\n");
          return (int)g_MessagesBuffer;  
        }
        else
        {
            if(mutex_unlock(mutex)) printf("capture.getParameter Fehler bei unlock_mutex 2\n");
            return 0;
        }
    }
    else if(g_Parameters[0] != string(g_modulname))
    {
            //TODO: weiterleiten
            if(mutex_unlock(mutex)) printf("capture.getParameter Fehler bei unlock_mutex 3\n");
            return 0;
    }
    if(g_Parameters[1] == string("camera"))
    {
            return camera_getParameter(&g_Parameters[2]);
    }
    else if(g_Parameters[1] == string("getPictureFunc") )
    {   
        if(mutex_unlock(mutex)) printf("capture.getParameter Fehler bei unlock_mutex 4\n");
        return (int)getPicture;
    }
    else if(string(name) == string("capture.resolution.x") ||
            string(name) == string("capture.resolution.width"))
    {
        
        
        int width = g_cfg.width;
        if(mutex_unlock(mutex)) printf("capture.getParameter Fehler bei unlock_mutex 5\n");
        return width;
    }
    else if(string(name) == string("capture.resolution.y") ||
            string(name) == string("capture.resolution.height"))
    {
        int height = g_cfg.height;    
        if(mutex_unlock(mutex)) printf("capture.getParameter Fehler bei unlock_mutex 6\n");       
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
    if(mutex_lock(mutex))
        g_Messages.push(string("start</b> <font color='red'>Fehler bei mutex_lock</font>"));
    
    g_capture.open(g_cfg.cameraNr);    
    if(!g_capture.isOpened())  // check if we succeeded
    {
        //printf("Kamera konnte nicht geöffnet werden!");
        
        g_Messages.push(string("start</b> <font color='red'>Kamera konnte nicht geoeffnet werden!</font>"));
        if(mutex_unlock(mutex))
             g_Messages.push(string("start</b> <font color='red'>Fehler bei mutex_unlock 1</font>"));
        return -7;
    }
    g_Messages.push(string("start</b> <font color='green'>Kamera wurde erfolgreich geoeffnet!</font>"));
    g_capture.set(CV_CAP_PROP_FRAME_WIDTH, g_cfg.width);
    g_capture.set(CV_CAP_PROP_FRAME_HEIGHT, g_cfg.height);
    
    Mat temp;
    g_capture >> temp;
    g_cfg.width = temp.cols;
    g_cfg.height = temp.rows;
    
    char buffer[256];
    sprintf(buffer, "start</b> <font color='green'>Resolution: %dx%d</font>", g_cfg.width, g_cfg.height);
    g_Messages.push(string(buffer));
    
    g_run = true;
    printf("start called\n");

   // namedWindow("LIVE",1);
    if(mutex_unlock(mutex))
         g_Messages.push(string("start</b> <font color='red'>Fehler bei mutex_unlock 2</font>"));
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
    if(mutex_lock(mutex))
        g_Messages.push(string("getPicture</b> <font color='red'>Fehler bei mutex_lock</font>"));
    
    // if start wasn't called
    if(!g_run)
    {
        while(g_capture.isOpened())
                g_capture.release();
        printf("encoder.getPicture after call g_capture.release\n");
        g_Messages.push(string("getPicture</b> <font color='blue'>Kamera wurde geschlossen</font>"));
        if(mutex_unlock(mutex))
            g_Messages.push(string("getPicture</b> <font color='red'>Fehler bei mutex_unlock 1</font>"));
        return 0;
    }
    // try to open capture
    else if(!g_capture.isOpened())
    {
         g_Messages.push(string("getPicture</b> <font color='red'>keine Kamera geoeffnet!</font>"));
         if(mutex_unlock(mutex))
            g_Messages.push(string("getPicture</b> <font color='red'>Fehler bei mutex_unlock 2</font>"));
         return 0;
    }
    

    //get next or last picture
    Mat m, m2, m3;
    if(!removeFrame)
        g_capture.retrieve(m);
    else
        g_capture >> m; // get a new frame from camer
    
    if(!m.size().area())
    {
        g_Messages.push(string("getPicture</b> <font color='red'>picture from camera is empty</font>"));
        if(mutex_unlock(mutex))
            g_Messages.push(string("getPicture</b> <font color='red'>Fehler bei mutex_unlock 3</font>"));
        return 0;
    }
            
    //m = cvLoadImage("test.jpg");
    if(m.depth() != CV_8U)
    {
        g_Messages.push(string("getPicture</b> <font color='red'>depth != unsigned char</font>\n"));
        if(mutex_unlock(mutex))
            g_Messages.push(string("getPicture</b> <font color='red'>Fehler bei mutex_unlock 4</font>"));
        return 0;
    }
    //m.convertTo(m2, )
    //Scale Picture to choosen resolution (if camera didn't support it)
    Mat matrices[4];    
    //IplImage src = m;
  //  IplImage* scaled = cvCreateImage(cvSize(g_cfg.width, g_cfg.height), IPL_DEPTH_8U, 3);
    
    //cvResize( &src, scaled, CV_INTER_LINEAR );
    m3.create(g_cfg.width, g_cfg.height, m.type());
    resize(m, m3, Size(g_cfg.width, g_cfg.height));
    
    char buffer[256];
    sprintf(buffer, "getPicture</b> <i>breite: %d, hoehe: %d, area: %d</i>", m.size().width, m.size().height, m.size().area());
   // g_Messages.push(string(buffer));
    
    //rgb-output 
    if(rgb)
    {        
        //imshow("LIVE", scaled);
        split(m3, matrices);
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
                g_Messages.push(string("getPicture</b> <font color='red'>Fehler beim speicher reservieren in getPicture rgb!</font>"));
                if(mutex_unlock(mutex))
            g_Messages.push(string("getPicture</b> <font color='red'>Fehler bei mutex_unlock 5</font>"));
                return NULL;
            }
        }
        
        //return 0;
        //copy picture to buffer
        size_t size = m2.cols*m2.rows*4;
        memcpy(g_rgbPicture.channel1, m2.data, size);
        
        //free scaled image
        //cvReleaseImage(&scaled);
        
        if(mutex_unlock(mutex))
            g_Messages.push(string("getPicture</b> <font color='red'>Fehler bei mutex_unlock 6</font>"));
        //return pointer to picture buffer
        return &g_rgbPicture;
        
    }
    // yuv-Output
    else
    {
        //convert and split picture
        cvtColor(m3, m2, CV_BGR2YCrCb);
        split(m2, matrices);
        
        IplImage* U = cvCreateImage(cvSize(m3.cols/2, m3.rows/2), IPL_DEPTH_8U, 1);
        IplImage* V = cvCreateImage(cvSize(m3.cols/2, m3.rows/2), IPL_DEPTH_8U, 1);
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
        g_yuvPicture.width = m3.cols;
        g_yuvPicture.height = m3.rows;
        int newSize = picture_getSize(&g_yuvPicture);

        //compare buffer size and picture size, and make new buffer, if picture size differ
        if(oldSize != newSize)
        {
            picture_release(&g_yuvPicture);
            if(picture_create(&g_yuvPicture, m2.cols, m2.rows, 1))
            {
                g_Messages.push(string("getPicture</b> <font color='red'>Fehler beim speicher reservieren in getPicture yuv!</font>"));
                if(mutex_unlock(mutex))
                        g_Messages.push(string("getPicture</b> <font color='red'>Fehler bei mutex_unlock 7</font>"));
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
 //       cvReleaseImage(&scaled);
    
        if(mutex_unlock(mutex))
            g_Messages.push(string("getPicture</b> <font color='red'>Fehler bei mutex_unlock 8</font>"));
        //return pointer to picture buffer
        return &g_yuvPicture;        
    }
	
    if(mutex_unlock(mutex))
            g_Messages.push(string("getPicture</b> <font color='red'>Fehler bei mutex_unlock 9</font>"));
    return NULL;
}
//! \brief stop capture, threadsafe
//!
//! close camera and set run to false
//! \return 0
int stop()
{
    if(mutex_lock(mutex))
        g_Messages.push(string("stop</b> <font color='red'>Fehler bei mutex_lock</font>"));
    g_run = false;
    //g_capture.release();
    if(mutex_unlock(mutex))
        g_Messages.push(string("stop</b> <font color='red'>Fehler bei mutex_unlock</font>"));
    return 0;
}

