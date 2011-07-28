#include "frame.h"
#ifdef _WIN32
#include <stdio.h>
#endif


int stack_lock(void* mutex)
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
        printf("interface.frame::stack_lock fehler bei lock Stack mutex\n");
    return ret;        
}

int stack_unlock(void* mutex)
{
#ifdef _WIN32
	int ret = !ReleaseMutex(mutex);
#else
    int ret = pthread_mutex_unlock(mutex);
#endif
    
    if(ret)
        printf("interface.frame::stack_unlock fehler bei unlock Stack mutex\n");
    return ret;
}

SFrame_stack* stack_init(unsigned char* data, int size)
{
	SFrame_stack* s = (SFrame_stack*)malloc(sizeof(SFrame_stack));
	if(!s) return 0;
#ifdef _WIN32
		s->mutex = CreateMutex(NULL, FALSE, NULL);
		if(s->mutex == NULL) printf("Fehler: %d bei CreateMutex", GetLastError());
#else
		s->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(s->mutex, NULL);
#endif
        stack_lock(s->mutex);

        s->top = (SFrame_stack_element*)malloc(sizeof(SFrame_stack_element));
        s->bottom = s->top;
        
        s->top->top = 0;
        s->top->bottom = 0;
	s->top->frame = (SFrame*)malloc(sizeof(SFrame));	
        s->top->frame->size = size;
        s->top->frame->data = (unsigned char*)malloc(size);
        memmove(s->top->frame->data, data, size);
        s->count = 1;
        stack_unlock(s->mutex);
        
	return s;
}
int count_stack(SFrame_stack* s)
{
    if(!s) return 0;
    stack_lock(s->mutex);
    int count = s->count;
    stack_unlock(s->mutex);
    return count;
}

//von unten hinzufügen
void stack_push(SFrame_stack* s, SFrame* f)
{
	SFrame_stack_element* new_s = NULL;
    if(!s)
    {
        if(f)
           delete_frame(f);
        return;
    }
    stack_lock(s->mutex);
	new_s = (SFrame_stack_element*)malloc(sizeof(SFrame_stack_element));
        if(!new_s)
            return;
        
	new_s->top = s->bottom;
	new_s->bottom = NULL;
        if(s->bottom)
                s->bottom->bottom = new_s;
        s->bottom = new_s;
	new_s->frame = f;
        if(!s->top) s->top = new_s;
        s->count++;
        stack_unlock(s->mutex);
}

void stack_pop(SFrame_stack* s, SFrame** pf)
{
	SFrame_stack_element* old = NULL;
    if(!s || !s->top)
    {
        if(pf)
           *pf = NULL;
        return;
    }
    stack_lock(s->mutex);
    if(pf)
        *pf = s->top->frame;
    
    old = s->top;
    if(old == s->bottom) s->bottom = 0;
    s->top = old->bottom;
    if(s->top)
        s->top->top = 0;
    //SFrame_stack_element* cur_s = s->top;
    if(old)
        free(old);
    s->count--;
    stack_unlock(s->mutex);   
}

void frame_to_stack(SFrame_stack* s, unsigned char* data, int size)
{
	SFrame* f = NULL;
    if(!data) return;
    if(!s) return;
    
    f = (SFrame*)malloc(sizeof(SFrame));
    f->size = size;
    f->data = (unsigned char*)malloc(size);
    if(f->data)
      memmove(f->data, data, size);
    
   stack_push(s, f); 	
}

void delete_frame(SFrame* f)
{
    if(!f) return;
    if(f->data)
    {
        free(f->data);
        f->data = 0;
    }
    free(f);	
}

void clear_stack(SFrame_stack* g_s)
{
	SFrame * temp;
    if(!g_s) return;   
    
    while(g_s->top != 0 && g_s->bottom != 0)
    {
        stack_pop(g_s, &temp);
        delete_frame(temp);
    }
#ifdef _WIN32
	CloseHandle(g_s->mutex);
#else
    pthread_mutex_destroy(g_s->mutex);
	free(g_s->mutex);
#endif
    free(g_s);
}

