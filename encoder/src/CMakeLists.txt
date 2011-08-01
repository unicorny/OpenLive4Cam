PROJECT(encoder)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#IF(WIN32)
#if(COMMAND cmake_policy)
      cmake_policy(SET CMP0014 NEW)
#endif(COMMAND cmake_policy)
#ENDIF(WIN32)

add_definitions(-std=gnu99)

SET(EncoderSrc encoder x264 
               input raw_input rtp_output timecode_input socket
               ../filters/filters.c ../filters/video/cache.c ../filters/video/crop.c ../filters/video/depth.c 
               ../filters/video/fix_vfr_pts.c ../filters/video/internal.c ../filters/video/resize.c ../filters/video/select_every.c
               ../filters/video/source.c ../filters/video/video.c)
#IF(WIN32)
SET(EncoderSrc ${EncoderSrc} ../include/x264.h ../include/encoder.h ../../interface/picture.h ../../interface/interface.h ../../interface/frame.h ../include/socket.h
                             ../include/input.h)
#ENDIF(WIN32)

SET(PA ../x264/)
SET(PA2 ../../interface)
find_library(X264_LIB x264 PATHS ${PA})
find_library(INTERFACE_LIB interface PATHS ${PA2})
#ws2_32
IF(WIN32)
SET(ENCODER_LIBS  ${X264_LIB} ${INTERFACE_LIB} ws2_32)
ELSE()
SET(ENCODER_LIBS  ${X264_LIB} ${INTERFACE_LIB} m pthread)
ENDIF(WIN32)


# Declare the target (an executable)
ADD_LIBRARY(encoder SHARED ${EncoderSrc})
TARGET_LINK_LIBRARIES(encoder ${ENCODER_LIBS})
set_target_properties(encoder PROPERTIES DEFINE_SYMBOL BUILD_DLL)
IF(MSVC)
	set_target_properties(encoder PROPERTIES DEFINE_SYMBOL VC_BUILD)
ENDIF(MSVC)

SET_TARGET_PROPERTIES(encoder PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ../../gui)

