PROJECT(server)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)



SET(ServerSrc server OnDemandRTSPServer.cpp
			  ../../interface/interface.c)
#IF(WIN32)
SET(ServerSrc ${ServerSrc} ../include/server.h ../../interface/interface.h)
#ENDIF(WIN32)

SET(PA ../lib)
find_library(BASIC_USAGE_ENV BasicUsageEnvironment PATHS ${PA} REQUIRED)
find_library(USAGE_ENV UsageEnvironment PATHS ${PA} REQUIRED)
find_library(GROUP_SOCK groupsock PATHS ${PA} REQUIRED)
find_library(LIVE_MEDIA liveMedia PATHS ${PA} REQUIRED)
SET(LIBS ${LIBS} ${BASIC_USAGE_ENV} ${USAGE_ENV} ${GROUP_SOCK} ${LIVE_MEDIA})


# Declare the target (an executable)
ADD_LIBRARY(server SHARED ${ServerSrc})
TARGET_LINK_LIBRARIES(server ${LIBS})
set_target_properties(server PROPERTIES DEFINE_SYMBOL BUILD_DLL)

#IF(WIN32)
SET_TARGET_PROPERTIES(server PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../gui/debug)
#ELSEIF
SET_TARGET_PROPERTIES(server PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ../../gui)
#ENDIF(WIN32)
