PROJECT(capture)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


FIND_PACKAGE( OpenCV REQUIRED )

SET(CaptureSrc capture camera )
#IF(WIN32)
SET(CaptureSrc ${CaptureSrc} ../include/capture.h ../include/camera.h ../../interface/picture.h ../../interface/interface.h)
#ENDIF(WIN32)

SET(PA ../../interface/)

find_library(INTERFACE interface PATHS ${PA} REQUIRED)
SET(LIBS ${INTERFACE} ${OpenCV_LIBS})

# Declare the target (an executable)
ADD_LIBRARY(capture SHARED ${CaptureSrc})

TARGET_LINK_LIBRARIES(capture ${LIBS})
set_target_properties(capture PROPERTIES DEFINE_SYMBOL BUILD_DLL)
IF(MSVC)
	set_target_properties(capture PROPERTIES DEFINE_SYMBOL VC_BUILD)
ENDIF(MSVC)

#MESSAGE(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
IF(WIN32)
IF(MSVC)
SET_TARGET_PROPERTIES(capture PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../gui/)
ELSEIF(MSVC)
SET_TARGET_PROPERTIES(capture PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../gui/debug/)
ENDIF(MSVC)
ELSEIF(WIN32)
SET_TARGET_PROPERTIES(capture PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ../../gui)
ENDIF(WIN32)
