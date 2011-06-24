#include <stdio.h>
#include <string.h>
#include "interface.h"
#include "frame.h"


SFrame_stack* stack;

int main(int argc, char* argv[])
{
    printf("Start stack Test\n");
    char TestBuffer1[] = "Dies wird ein schöner und laaaanger Text.";
    char TestBuffer2[] = "Dies ist der 2. Testbuffer Text";
    stack = stack_init(TestBuffer1, strlen(TestBuffer1));
    printf("stack count 1 erwartet: %d\n", stack->count);
    if(!stack)
        printf("error: stack wurde nicht angelegt!\n");
    if(stack->bottom != stack->top)
        printf("error: Nach stack init sind top != bottom\n");
    if(strcmp((char*)stack->top->frame->data, TestBuffer1) != 0)
        printf("error: Daten auf dem Stack sind korruptet!\n");
    frame_to_stack(stack, TestBuffer2, strlen(TestBuffer2));
    printf("stack count 2 erwartet: %d\n", stack->count);
    if(strcmp((char*)stack->top->frame->data, TestBuffer2) == 0)
        printf("error: Neue Daten sind an der falschen Stelle gelandet!\n");
    if(strcmp((char*)stack->bottom->frame->data, TestBuffer2) != 0)
        printf("error: Neue Daten sind korrputet!\n");
    if(stack->top == stack->bottom)
        printf("error: nachdem zwei Datensätze auf dem Stack liegen, sind top == bottom");
    SFrame* frame = NULL;
    stack_pop(stack, &frame);
    printf("stack count 1 erwartet: %d\n", stack->count);
    printf("Daten vom Stack: size: %d, text: %s, vergleichstext: %s\n", frame->size, frame->data, TestBuffer1);
    delete_frame(frame);
    
    stack_pop(stack, &frame);
    delete_frame(frame);
    printf("stack count 0 erwartet: %d\n", stack->count);
    
    frame_to_stack(stack, TestBuffer1, strlen(TestBuffer1));
    printf("stack count 1 erwartet: %d\n", stack->count);
    
    stack_pop(stack, &frame);
    delete_frame(frame);
    
    stack_pop(stack, &frame);
    if(frame)
        printf("leerer Eintrag: %d, %s\n",frame->size, frame->data );
    printf("stack count 0 erwartet: %d\n", stack->count);
  
    clear_stack(stack);
    return 0;
}
