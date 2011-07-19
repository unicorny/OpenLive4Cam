#ifndef __OPEN_LIVE_4_CAM_INTERFACE_PICTURE__
#define __OPEN_LIVE_4_CAM_INTERFACE_PICTURE__

#include "interface.h"

#ifdef __cplusplus
extern "C"
{
#endif

//! \brief init picture
//!
//! set everything to zero (pixelsize set to 1)
//! \param data pointer to SPicture struct
void picture_init(SPicture *data);

//! \brief create picture, allocate memory and copy params
//!
//! for rgb = 1, allocate space for channel1 (width*height*pixelsize Bytes) <br>
//! for rgb = 0, allocate space for channel1, channel2 and channel3: <br>
//! <ul><li>channel1 = width*height*pixelsize Bytes</li>
//!     <li>channel2 = width*height*pixelszie/2 Bytes</li>
//!     <li>channel3 = width*height*pixelsize/2 Bytes</li></ul>
//! \param data pointer to SPicture struct
//! \param width picture-width in pixels
//! \param height picture-height in pixels
//! \param pixelsize in bytes 
//! \return 0 (no problems)
//! \return -1 by error allocate memory for channel 1
//! \return -4 by error allocate memory for channel 2
//! \return -5 by error allocate memory for channel 3
int picture_create(SPicture* data, int width, int height, int pixelsize);

//! \brief free channel-memory
//!
//! if memory was allocated and pointer != NULL
//! \param data pointer to SPicture struct
void picture_release(SPicture *data);
//! \brief calculate picture size in bytes
//!
//! \param data pointer to SPicture struct
//! \return width*height*pixelsize
int  picture_getSize(SPicture* data);

#ifdef __cplusplus
}
#endif




#endif //__OPEN_LIVE_4_CAM_INTERFACE_PICTURE__
