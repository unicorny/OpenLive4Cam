PROJECT(capture)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

SET(CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})

FIND_PACKAGE( OpenCV REQUIRED )

SET(CaptureSrc main camera ../../interface/picture.c)
#IF(WIN32)
SET(CaptureSrc ${CaptureSrc} ../include/capture.h ../include/camera.h ../../interface/picture.h)
#ENDIF(WIN32)

# Declare the target (an executable)
ADD_LIBRARY(capture SHARED ${CaptureSrc})

TARGET_LINK_LIBRARIES(capture ${OpenCV_LIBS})
set_target_properties(capture PROPERTIES DEFINE_SYMBOL BUILD_DLL)

#MESSAGE(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
#IF(WIN32)
SET_TARGET_PROPERTIES(capture PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../gui)
#ELSEIF
SET_TARGET_PROPERTIES(capture PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ../../gui)
#ENDIF(WIN32)
