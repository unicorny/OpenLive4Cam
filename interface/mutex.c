#include "mutex.h"

#include <stdlib.h>


Mutex* mutex_init()
{
    Mutex* m = (Mutex*)malloc(sizeof(Mutex));
    if(!m) return NULL;
#ifdef _WIN32
	m->mutex = CreateMutex(NULL, FALSE, NULL);
	if(m->mutex == NULL)
	{
	    //printf("Fehler: %d bei mutex init", GetLastError());
            free(m);
            return NULL;
	}
#else
	m->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if(pthread_mutex_init(m->mutex, NULL))
    {
        //printf("capture.init fehler bei mutex init\n");
        free(m);
        return NULL;
    }
#endif
        
    return m;
}

int mutex_lock(Mutex* m)
{
	int ret = 1;
#ifdef _WIN32
	DWORD waitResult = 0;
#endif
    if(!m) return -1;
    if(!m->mutex) return -2;
    
#ifdef _WIN32
	waitResult = WaitForSingleObject(m->mutex, INFINITE);
	if(waitResult == WAIT_OBJECT_0)
        {
		ret = 0;
                //printf("capture mutex locked\n");
        }
	else if(waitResult == WAIT_FAILED)
        {
		//printf("Fehler %d bei lock mutex\n", GetLastError());
        }

#else
    ret = pthread_mutex_lock((pthread_mutex_t*)m->mutex);
#endif
    //if(ret)
        //printf("capture.lock_mutex fehler bei lock mutex\n");
    return ret;  
}

int mutex_unlock(Mutex* m)
{
	int ret = 0;

    if(!m) return -1;
    if(!m->mutex) return -2;
#ifdef _WIN32   
    ret = !ReleaseMutex(m->mutex);
#else
    ret = pthread_mutex_unlock((pthread_mutex_t*)m->mutex);
#endif
    
    //if(ret)
        //printf("capture.unlock_mutex fehler bei unlock mutex\n");
    return ret;
}

void mutex_close(Mutex* m)
{
    if(!m) return;
    if(m->mutex)    
    {
#ifdef _WIN32
	CloseHandle(m->mutex);
#else
        pthread_mutex_destroy(m->mutex);
	free(m->mutex);
#endif
    }
    free(m);
}
