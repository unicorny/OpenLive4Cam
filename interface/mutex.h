/* 
 * File:   mutex.h
 * Author: dario
 *
 * Created on 1. August 2011, 12:51
 */

#ifndef __OPEN_LIVE_4_CAM_INTERFACE_MUTEX__
#define	__OPEN_LIVE_4_CAM_INTERFACE_MUTEX__

#include <memory.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h> 
#endif

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct Mutex{
#ifdef _WIN32
	//void* mutex;
    CRITICAL_SECTION mutex;
#else
        pthread_mutex_t* mutex; 
#endif
} Mutex;

Mutex* mutex_init();

int mutex_lock(Mutex* m);

int mutex_unlock(Mutex* m);

void mutex_close(Mutex* m);


#ifdef	__cplusplus
}
#endif

#endif	/* __OPEN_LIVE_4_CAM_INTERFACE_MUTEX__ */

