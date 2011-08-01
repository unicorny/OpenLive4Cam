#include "frame.h"
#ifdef _WIN32
#include <stdio.h>
#endif


SFrame_stack* stack_init(unsigned char* data, int size)
{
	SFrame_stack* s = (SFrame_stack*)malloc(sizeof(SFrame_stack));
	if(!s) return 0;

        s->mutex = mutex_init();
        if(!s->mutex) 
        {
            printf("interface.stack_init Fehler bei mutex_init!\n");
            free(s);
            return NULL;
        }
       
        if(mutex_lock(s->mutex))
            printf("interface.stack_init Fehler bei mutex_lock\n");

        s->top = (SFrame_stack_element*)malloc(sizeof(SFrame_stack_element));
        s->bottom = s->top;
        
        s->top->top = 0;
        s->top->bottom = 0;
        gettimeofday(&s->top->captureTime, NULL);
        s->top->firstEntry = 1;
	s->top->frame = (SFrame*)malloc(sizeof(SFrame));	
        s->top->frame->size = size;
        s->top->frame->data = (unsigned char*)malloc(size);
        memmove(s->top->frame->data, data, size);
        s->count = 1;
        
        if(mutex_unlock(s->mutex))
            printf("interface.stack_init Fehler bei mutex_unlock\n");
        
	return s;
}
int count_stack(SFrame_stack* s)
{
    if(!s) return 0;
    if(mutex_lock(s->mutex))
        printf("interface.count_stack Fehler bei mutex_lock\n");
    int count = s->count;
    if(mutex_unlock(s->mutex))
        printf("interface.count_stack Fehler bei mutex_unlock\n");
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
    if(mutex_lock(s->mutex))
        printf("interface.stack_push Fehler bei mutex_lock\n");
    new_s = (SFrame_stack_element*)malloc(sizeof(SFrame_stack_element));
    if(!new_s)
        return;
    new_s->firstEntry = 0;
    new_s->top = s->bottom;
    new_s->bottom = NULL;
    if(s->bottom)
            s->bottom->bottom = new_s;
    s->bottom = new_s;
    new_s->frame = f;
    if(!s->top) s->top = new_s;
    gettimeofday(&new_s->captureTime, NULL);
    s->count++;
    
    if(mutex_unlock(s->mutex))
        printf("interface.stack_push Fehler bei mutex_unlock\n");
}

void stack_pop(SFrame_stack* s, SFrame** pf, struct timeval* time)
{
    SFrame_stack_element* old = NULL;
    if(!s || !s->top)
    {
        if(pf)
           *pf = NULL;
        return;
    }
    if(mutex_lock(s->mutex))
        printf("interface.stack_pop Fehler bei mutex_lock\n");
    if(pf)
        *pf = s->top->frame;
    
    old = s->top;
    if(old == s->bottom) s->bottom = 0;
    s->top = old->bottom;
    if(s->top)
        s->top->top = 0;
    if(old && time)
        memcpy(time, &old->captureTime, sizeof(struct timeval));
    //SFrame_stack_element* cur_s = s->top;
    if(old)
        free(old);
    s->count--;
    if(mutex_unlock(s->mutex))
        printf("interface.stack_pop Fehler bei mutex_unlock\n");
}

void stack_delete_top(SFrame_stack* s)
{
    SFrame_stack_element* old = NULL;
    SFrame_stack_element* firstFrame = NULL;
    if(!s || !s->top)
    {
        return;
    }
    if(mutex_lock(s->mutex))
        printf("interface.stack_Delete_top Fehler bei mutex_lock\n");
        
    old = s->top;
    if(old->firstEntry) {old = old->bottom; firstFrame = s->top;}
    if(!old)
    {
        if(mutex_unlock(s->mutex))
            printf("interface.stack_delete_top Fehler bei mutex_unlock\n");
        return;
    }
    
    if(old == s->bottom)
    {
        if(!firstFrame)
            s->bottom = 0;
        else
            s->bottom = firstFrame;
    }
    
    
    if(!firstFrame)
    {
        //printf("!firstFrame\n");
        s->top = old->bottom;
        if(s->top)
                s->top->top = 0;
    }
    else
    {
       // printf("firstFrame: %d\n", firstFrame);
        if(old->bottom)
                old->bottom->top = firstFrame;
    }
    
    //SFrame_stack_element* cur_s = s->top;
    if(old->frame) delete_frame(old->frame);
    if(old) free(old);
    
    s->count--;
    if(mutex_unlock(s->mutex))
        printf("interface.stack_delete_top Fehler bei mutex unlock\n");
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
        stack_pop(g_s, &temp, NULL);
        delete_frame(temp);
    }
    mutex_close(g_s->mutex);
    g_s->mutex = NULL;
    free(g_s);
}

