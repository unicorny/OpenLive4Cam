PROJECT(encoder)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#if(COMMAND cmake_policy)
 #     cmake_policy(SET CMP0003 NEW)
#endif(COMMAND cmake_policy)


add_definitions(-std=gnu99)

SET(EncoderSrc encoder x264 ../../interface/picture.c ../../interface/interface.c
               input sdp raw_input rtp_output timecode_input socket
               ../filters/filters.c ../filters/video/cache.c ../filters/video/crop.c ../filters/video/depth.c 
               ../filters/video/fix_vfr_pts.c ../filters/video/internal.c ../filters/video/resize.c ../filters/video/select_every.c
               ../filters/video/source.c ../filters/video/video.c)
#IF(WIN32)
SET(EncoderSrc ${EncoderSrc} ../include/x264.h ../include/encoder.h ../../interface/picture.h ../../interface/interface.h
                             ../include/input.h)
#ENDIF(WIN32)

SET(PA ../x264/)
find_library(X264_LIB x264 PATHS ${PA})
#SET(X264_LIB  x264)



# Declare the target (an executable)
ADD_LIBRARY(encoder SHARED ${EncoderSrc})
TARGET_LINK_LIBRARIES(encoder ${X264_LIB})
set_target_properties(encoder PROPERTIES DEFINE_SYMBOL BUILD_DLL)

#IF(WIN32)
SET_TARGET_PROPERTIES(encoder PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../gui/debug)
#ELSEIF
SET_TARGET_PROPERTIES(encoder PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ../../gui)
#ENDIF(WIN32)
