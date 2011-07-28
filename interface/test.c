#include <stdio.h>
#include <string.h>
#include "interface.h"
#include "frame.h"

void try_load_dll(const char* filename)
{
    SInterface* p = interface_loadDll(filename);
	int errornr = 0;
    if(!p)  
    {
        printf("Fehler beim Laden von %s\n", filename);
#ifndef _WIN32
        printf("Fehler beim laden einer Bibliothek (*.so)\n%s\n",  dlerror());
#else
        errornr = GetLastError();
        if(errornr == 126)
            printf("DLL wurde nicht gefunden. \n");
        else if(errornr == 127)
            printf("Eine Funktion in der DLL wurde nicht gefunden!\n");
        else
            printf("Fehler beim laden einer dll\nerror nr: %d \n", (int)GetLastError());
#endif
    }
    else
    {
        printf("DLL: %s wurde erfolgreich geladen!\n", filename);
        interface_close(p);
    }
    
}

SFrame_stack* stack;

int main(int argc, char* argv[])
{
    unsigned char TestBuffer1[] = "Dies wird ein schoener und laaaanger Text.\0";
    unsigned char TestBuffer2[] = "Dies ist der 2. Testbuffer Text\0";
	SFrame* frame = NULL;
	SFrame f;
	unsigned* mem = NULL;
	size_t s_mem = 0;

    stack = stack_init(TestBuffer1, strlen(TestBuffer1)+1);
	printf("Start stack Test\n");
    printf("stack count 1 erwartet: %d\n", stack->count);
    if(!stack)
        printf("error: stack wurde nicht angelegt!\n");
    if(stack->bottom != stack->top)
        printf("error: Nach stack init sind top != bottom\n");
    if(strcmp((char*)stack->top->frame->data, TestBuffer1) != 0)
        printf("error: Daten auf dem Stack sind korruptet!\n");
    frame_to_stack(stack, TestBuffer2, strlen(TestBuffer2)+1);
    printf("stack count 2 erwartet: %d\n", stack->count);
    if(strcmp((char*)stack->top->frame->data, TestBuffer2) == 0)
        printf("error: Neue Daten sind an der falschen Stelle gelandet!\n");
    if(strcmp((char*)stack->bottom->frame->data, TestBuffer2) != 0)
        printf("error: Neue Daten sind korrputet!\n");
    if(stack->top == stack->bottom)
        printf("error: nachdem zwei Datensätze auf dem Stack liegen, sind top == bottom");
    
    stack_pop(stack, &frame);
    printf("stack count 1 erwartet: %d\n", stack->count);
    printf("Daten vom Stack: size: %d, text: %s, vergleichstext: %s\n", frame->size, frame->data, TestBuffer1);
    delete_frame(frame);
    stack_pop(stack, &frame);
//    return 1;
    delete_frame(frame);

    printf("stack count 0 erwartet: %d\n", stack->count);
    
    frame_to_stack(stack, TestBuffer1, strlen(TestBuffer1)+1);
    printf("stack count 1 erwartet: %d\n", stack->count);
    
    stack_pop(stack, &frame);
    delete_frame(frame);
    
    stack_pop(stack, &frame);
    if(frame)
        printf("leerer Eintrag: %d, %s\n",frame->size, frame->data );
    printf("stack count 0 erwartet: %d\n", stack->count);
    
    printf("\n--- malloc test ---\n");

    f.size = 30925;
    f.data = (unsigned char*)malloc(f.size);
    memset(f.data,0 ,f.size);
    mem = (unsigned*)f.data;
    s_mem = f.size/sizeof(unsigned);
    printf("anfang memory block: %d%d%d%d\n", mem[0], mem[1], mem[2], mem[3]);
    printf("ende memory block: %d%d%d%d\n", mem[s_mem-4], mem[s_mem-3], mem[s_mem-2], mem[s_mem-1]);    
  
    clear_stack(stack);
    
    printf("--- ende Stack Test ---\n\n--- Beginn DLL Tests ---\n");
    
    
    try_load_dll("server.dll");    
    try_load_dll("capture.dll");   
    try_load_dll("./libserver.so");
    try_load_dll("./libcapture.so"); 
    try_load_dll("./libencoder.so"); 
    try_load_dll("libencoder.dll");
    try_load_dll("libcapture.dll");
	
	    
    return 0;
}
