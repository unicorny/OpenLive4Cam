#ifndef __OPEN_LIVE_4_CAM_INTERFACE_PICTURE__
#define __OPEN_LIVE_4_CAM_INTERFACE_PICTURE__

#include "interface.h"

void picture_init(SPicture *data);
int picture_create(SPicture* data, int width, int height, int pixelsize);
void picture_release(SPicture *data);
int  picture_getSize(SPicture* data);





#endif //__OPEN_LIVE_4_CAM_INTERFACE_PICTURE__
