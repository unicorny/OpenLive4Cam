#ifndef __OPEN_LIVE_4_CAM_INTERFACE_PICTURE__
#define __OPEN_LIVE_4_CAM_INTERFACE_PICTURE__

#include "interface.h"

#ifdef __cplusplus
extern "C"
{
#endif

void picture_init(SPicture *data);
int picture_create(SPicture* data, int width, int height, int pixelsize);
void picture_release(SPicture *data);
int  picture_getSize(SPicture* data);

#ifdef __cplusplus
}
#endif




#endif //__OPEN_LIVE_4_CAM_INTERFACE_PICTURE__
