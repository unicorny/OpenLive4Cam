PROJECT(capture)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
#SET(CMAKE_MODULE_PATH "D:/downloads/programmieren/OpenCV-2.2.0-win/OpenCV-2.2.0")
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

FIND_PACKAGE( OpenCV REQUIRED )

# Declare the target (an executable)
#ADD_EXECUTABLE(OpenLive4Cam_capture  streamout.c)
ADD_LIBRARY(capture SHARED main.c)

TARGET_LINK_LIBRARIES(capture ${OpenCV_LIBS})
set_target_properties(capture PROPERTIES DEFINE_SYMBOL BUILD_DLL)

#MESSAGE(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")

INSTALL(TARGETS capture
	LIBRARY DESTINATION lib
)

