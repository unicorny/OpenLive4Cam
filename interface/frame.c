#include "frame.h"

int stack_lock(void* mutex)
{
    int ret = pthread_mutex_lock(mutex);
    if(ret)
        printf("interface.frame::stack_lock fehler bei lock Stack mutex\n");
    return ret;        
}

int stack_unlock(void* mutex)
{
    int ret = pthread_mutex_unlock(mutex);
    if(ret)
        printf("interface.frame::stack_unlock fehler bei unlock Stack mutex\n");
    return ret;
}

SFrame_stack* stack_init(unsigned char* data, int size)
{
	SFrame_stack* s = (SFrame_stack*)malloc(sizeof(SFrame_stack));
	if(!s) return 0;
        
        pthread_mutex_init(&s->mutex, NULL);
        stack_lock(&s->mutex);

        s->top = (SFrame_stack_element*)malloc(sizeof(SFrame_stack_element));
        s->bottom = s->top;
        
        s->top->top = 0;
        s->top->bottom = 0;
	s->top->frame = (SFrame*)malloc(sizeof(SFrame));	
        s->top->frame->size = size;
        s->top->frame->data = (unsigned char*)malloc(size);
        memcpy(s->top->frame->data, data, size);
        s->count = 1;
        stack_unlock(&s->mutex);
        
	return s;
}

//von unten hinzufügen
void stack_push(SFrame_stack* s, SFrame* f)
{
    if(!s)
    {
        if(f)
           delete_frame(f);
        return;
    }
    stack_lock(&s->mutex);
	SFrame_stack_element* new_s = (SFrame_stack_element*)malloc(sizeof(SFrame_stack_element));
        if(!new_s)
            return;
        
	new_s->top = s->bottom;
        if(s->bottom)
                s->bottom->bottom = new_s;
        s->bottom = new_s;
	new_s->frame = f;
        if(!s->top) s->top = new_s;
        s->count++;
        stack_unlock(&s->mutex);
}

void stack_pop(SFrame_stack* s, SFrame** pf)
{
    if(!s || !s->top)
    {
        if(pf)
           *pf = NULL;
        return;
    }
    stack_lock(&s->mutex);
    if(pf)
        *pf = s->top->frame;
    
    SFrame_stack_element* old = s->top;
    if(old == s->bottom) s->bottom = 0;
    s->top = old->bottom;
    if(s->top)
        s->top->top = 0;
    //SFrame_stack_element* cur_s = s->top;
    if(old)
        free(old);
    s->count--;
    stack_unlock(&s->mutex);   
}

void frame_to_stack(SFrame_stack* s, unsigned char* data, int size)
{
    if(!data) return;
    if(!s) return;
    
    SFrame* f = (SFrame*)malloc(sizeof(SFrame));
    f->size = size;
    f->data = (unsigned char*)malloc(size);
    if(f->data)
      memcpy(f->data, data, size);
    
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
    if(!g_s) return;
    
    SFrame * temp;
    while(g_s->top != 0 && g_s->bottom != 0)
    {
        stack_pop(g_s, &temp);
        delete_frame(temp);
    }
    
    pthread_mutex_destroy(&g_s->mutex);
    free(g_s);
}

