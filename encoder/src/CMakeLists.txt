PROJECT(encoder)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#if(COMMAND cmake_policy)
 #     cmake_policy(SET CMP0003 NEW)
#endif(COMMAND cmake_policy)


#FIND_PACKAGE( OpenCV REQUIRED )

SET(EncoderSrc encoder ../../interface/picture.c ../../interface/interface.c)
#IF(WIN32)
SET(EncoderSrc ${EncoderSrc} ../include/encoder.h ../../interface/picture.h ../../interface/interface.h)
#ENDIF(WIN32)

#SET(PA x264/)
#find_library(SDL_LIB SDL PATHS ${PA})
SET(264_LIB  x264)



# Declare the target (an executable)
ADD_LIBRARY(encoder SHARED ${EncoderSrc})
TARGET_LINK_LIBRARIES(encoder ${X264_LIB})
set_target_properties(encoder PROPERTIES DEFINE_SYMBOL BUILD_DLL)

#IF(WIN32)
SET_TARGET_PROPERTIES(encoder PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../gui/debug)
#ELSEIF
SET_TARGET_PROPERTIES(encoder PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ../../gui)
#ENDIF(WIN32)
