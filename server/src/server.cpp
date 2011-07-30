#include "server.h"



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


std::string g_Parameters[MAX_PARAMETER_COUNT];
SInterface* encoder = NULL;
std::stack<std::string> g_Messages;
char g_MessagesBuffer[256];
int g_Port = 8554;
unsigned char* (*getFrameFunc)(int*) = NULL;
int (*getStackCount)(void) = NULL;

//live
UsageEnvironment* env = NULL;
TaskScheduler* scheduler = NULL;
RTSPServer* rtspServer = NULL;
FramedSource* eds = NULL;
UserAuthenticationDatabase* authDB = NULL;

bool g_run = false;
char g_watch = 0;
#ifdef ACCESS_CONTROL
  // To implement client access control to the RTSP server, do the following:
  authDB = new UserAuthenticationDatabase;
  authDB->addUserRecord("username1", "password1"); // replace these with real strings
  // Repeat the above with each <username>, <password> that you wish to allow
  // access to the server.
#endif
  
Boolean reuseFirstSource = True;

int init()
{
    printf("server Modul init\n");
#ifdef _WIN32
	encoder = interface_loadDll("libencoder.dll");
#else
    encoder = interface_loadDll("libencoder.so");
#endif
    if(!encoder)
    {
        printf("server::init Fehler beim laden des Encoder Modules\n");
#ifndef _WIN32
        printf("Fehler beim laden einer Bibliothek (*.so)\n%s\n",  dlerror());
#else
        if(GetLastError() == 126)
            printf("Fehler beim laden einer dll\nDLL wurde nicht gefunden. ");
        else
            printf("Fehler beim laden einer dll\nerror nr: %d ", (int)GetLastError());
#endif
        return -1;
    }
    if(encoder->init() < 0)
    {
        printf("server::init Fehler beim initalisieren von Capture!\n");
        return -2;
    }
    //getFrame function holen von encoder
    getFrameFunc = (unsigned char* (*)(int*))encoder->getParameter("encoder.getFrameFunc");
    if(!getFrameFunc)
    {
        printf("Error, encoder.getFrameFunc didn't work as exceptet!");
        return -4;
    }
    getStackCount = (int (*)())encoder->getParameter("encoder.getStackCountFunc");
    if(!getStackCount)
    {
        printf("Error, encoder.getStackCountFunc didn't work as exceptet!");
        return -5;
    }
 //return 0;    
    //live starten
    scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);
    if(!scheduler || ! env)
    {
        printf("server::init fehler, live konnte nicht initalisisert werden!");
        return -1;
    }
    return 0;
}

int run()
{
    if(env)
    {
        g_watch = 0;
        printf("g_watch: %d\n", (int)g_watch);
        
        env->taskScheduler().doEventLoop(&g_watch);
        //((BasicTaskScheduler0)env->taskScheduler()).SingleStep(0);
    }
	return 0;
}

void checkIfNewDataAvailable()
{    
    if(getStackCount() >= 2)
        signalNewFrameData();
}

void ende()
{
    g_run = 0;
    g_watch = 1;
//    SAVE_DELETE(env);    
    if(encoder)
    {
        encoder->ende();
        interface_close(encoder);
    }
    encoder = NULL;
    printf("Server Modul ende\n");
}

void setParameter(const char* name, int value)
{
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
    if(g_Parameters[0].compare(string(g_modulname)) != 0 && encoder)
            encoder->setParameter(name, value); //TODO: weiterleiten  
    
	if(g_Parameters[1].compare(string("port")) == 0)
    {
        g_Port = value;
    }
}
int getParameter(const char* name)
{
    char buffer[256];
    sprintf(buffer, "%s", name);   
  //  printf("server.getParameter: name = %s\n", name);
  
    char * pch;
    pch = strtok (buffer, ".\0");
    int count = 0;
    while (pch != NULL)
    {
        g_Parameters[count++] = pch;
        pch = strtok (NULL, ".\0");
        
    }
    
    if(g_Parameters[0].compare(string("getLastMessage")) == 0)
    {
        if(g_Messages.size())
        {
          sprintf(g_MessagesBuffer,"server: %s", g_Messages.top().data());
          g_Messages.pop();
          return (int)g_MessagesBuffer;  
        }
        else
        {
            if(!encoder) return 0;
            return encoder->getParameter(name);
        }
    }
    else if(g_Parameters[0].compare(string(g_modulname)) != 0 && encoder)
    {
        return encoder->getParameter(name);
    }
    
    if(g_Parameters[1].compare(string("port")) == 0)
    {
        return g_Port;
    }
    else if(g_Parameters[1].compare(string("getTickFunc"))== 0)
    {
        return (int)run;
    }
    else if(g_Parameters[1].compare(string("checkIfNewDataAvailableFunc")) == 0)
    {
        return (int)checkIfNewDataAvailable;
    }
    
    
    return 0;
}
static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName) {
  char* url = rtspServer->rtspURL(sms);
  string a;
  a += streamName;
  a += "\" stream, from camera \"";
  a += "\"\n";
  a += "Play this stream using the URL \"";
  a += url;
  a += "\"\n";
  
  g_Messages.push(a);
  delete[] url;
}

int start()
{
    int ret = 0;
    if(encoder)
    {
       ret = encoder->start();
       if(ret)
       {
           g_Messages.push(string("server.start Fehler beim starten des Encoders!"));
           return ret;
       }
    }
	g_Messages.push(string("server.start Encoder gestartet"));
 //   return 0;
    int encoderPort = encoder->getParameter("encoder.port");
    
    RTSPServer* rtspServer = RTSPServer::createNew(*env, g_Port, authDB);
    if (rtspServer == NULL) {
        g_Messages.push(string("Failed to create RTSP server: ").append(env->getResultMsg()));
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        return -1;
  }    
    char const* descriptionString
    = "Session streamed by \"server\"";
    
    // A H.264 video elementary stream:
  {
    char const* streamName = "h264";
    char const* inputFileName = "./jumper2.h264";
    

    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    //EncoderDeviceSource* eds = 
      //      EncoderDeviceSource::createNew(*env, EncoderDeviceParameters(getFrameFunc, &g_run));
 //   H264VideoStreamDiscreteFramer* dF = 
   //         H264VideoStreamDiscreteFramer::createNew(*env, eds);
    //Groupsock* gr = new Groupsock(*env, inet_makeaddr(127,1), Port(encoderPort), 0);
    //gr->changeDestinationParameters()
    //H264VideoRTPSource* rtp = 
      //      H264VideoRTPSource::createNew(*env, gr, 96);
   // sms->addSubsession(H264VideoEncoderServerMediaSubsession
	//	       ::createNew(*env, dF, reuseFirstSource));
    //*/
   /* sms->addSubsession(H264VideoFileServerMediaSubsession
		       ::createNew(*env, inputFileName, reuseFirstSource));
   //*/
    //ByteStreamFileSource* fileSource = ByteStreamFileSource::createNew(*env, inputFileName);
    
    sms->addSubsession(H264VideoEncoderServerMediaSubsession
		       ::createNew(*env, &eds, reuseFirstSource));
    //* */
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName);
   
  }

 
    

  // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
  // Try first with the default HTTP port (80), and then with the alternative HTTP
  // port numbers (8000 and 8080).

  char t[256];
  if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
    //*env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
    sprintf(t, "(We use port %d for optional RTSP-over-HTTP tunneling.)", rtspServer->httpServerPortNum());
  } else {
    //*env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
      sprintf(t, "(RTSP-over-HTTP tunneling is not available.)");
  }
    g_Messages.push(string(t));
    g_run = true;
  
    return 0;
}
int stop()
{
    g_run = false;
   // rtspServer->close(*env, "h264");
    if(encoder)
        encoder->stop();
    
     //g_watch = 1;
    return 0;
}