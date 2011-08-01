#ifndef __OPEN_LIVE_4_CAM_INTERFACE_FRAME__
#define __OPEN_LIVE_4_CAM_INTERFACE_FRAME__

#include "interface.h"
#include "mutex.h"
#include <malloc.h>
#include <memory.h>
#ifndef WIN32
#include <sys/time.h>
#else
#include <time.h>
#endif




#ifdef __cplusplus
extern "C"
{
#endif
    

typedef struct SFrame_stack
{
    struct SFrame_stack_element* top;
    struct SFrame_stack_element* bottom;
    int count;
    Mutex* mutex;

} SFrame_stack;

typedef struct SFrame_stack_element
{
    struct SFrame_stack_element* top;
    struct SFrame_stack_element* bottom;
    SFrame* frame;	
    unsigned firstEntry;
    struct timeval captureTime;
} SFrame_stack_element;

SFrame_stack* stack_init(unsigned char* data, int size);
// von unten hinzufügen
void stack_push(SFrame_stack* s, SFrame* f);
// von oben entfernen
void stack_pop(SFrame_stack* s, SFrame** pf, struct timeval* time);

// entfernt den obersten Eintrag wenn er nicht der erste eintrag war
void stack_delete_top(SFrame_stack* s);

void frame_to_stack(SFrame_stack* s, unsigned char* data, int size);
void delete_frame(SFrame* f);

void clear_stack(SFrame_stack* s);

int count_stack(SFrame_stack* s);

#ifdef __cplusplus
}
#endif


#endif //__OPEN_LIVE_4_CAM_INTERFACE_FRAME__
