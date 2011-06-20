#include "picture.h"
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

/*
 bool rgb;
    unsigned char* channel1;
    unsigned char* channel2;
    unsigned char* channel3;
    int width;
    int height;
    int pixelsize;    
 */
void picture_init(SPicture* data)
{       
    data->rgb = 0;
    data->channel1 = NULL;
    data->channel2 = NULL;
    data->channel3 = NULL;
    data->width = 0;
    data->height = 0;
    data->pixelsize = 1;
    
}

void picture_release(SPicture* data)
{
    if(data->channel1)
    {
        free(data->channel1);
        data->channel1 = NULL;
    }
    if(data->channel2)
    {
        free(data->channel2);
        data->channel2 = NULL;
    }
    if(data->channel3)
    {
        free(data->channel3);
        data->channel3 = NULL;
    }
}

int picture_create(SPicture* data, int width, int height, int pixelsize)
{
    size_t size = 0;

    data->width = width;
    data->height = height;
    data->pixelsize = pixelsize;
    size = sizeof(unsigned char)* picture_getSize(data);

    data->channel1 = (unsigned char*)malloc(size);
    if(!data->channel1) return -1;
    memset(data->channel1, 0, size);
    printf("channel 1: %d bytes allociert\n", size);
    if(data->rgb)
    {
        /*data->channel2 = (unsigned char*)malloc(size);
        data->channel3 = (unsigned char*)malloc(size);
        memset(data->channel2, 0, size);
        memset(data->channel3, 0, size);
        if(!data->channel2) return -2;
        if(!data->channel3) return -3;
         */
    }
    else
    {
        size_t sizeHalf = sizeof(unsigned char)*picture_getSize(data)/4;
        data->channel2 = (unsigned char*)malloc(sizeHalf);
        data->channel3 = (unsigned char*)malloc(sizeHalf);
        memset(data->channel2, 0, sizeHalf);
        memset(data->channel3, 0, sizeHalf);
        if(!data->channel2) return -4;
        if(!data->channel3) return -5;
        printf("channel 2+3: %d bytes allociert\n", sizeHalf);
    }
    return 0;
        
}

int  picture_getSize(SPicture* data)
{
    return (data->width*data->height)*data->pixelsize;
}
