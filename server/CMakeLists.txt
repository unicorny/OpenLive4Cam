PROJECT(server CXX)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)


#IF(WIN32)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../gui/debug)
#ELSEIF
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../gui)
#ENDIF(WIN32)


SUBDIRS(src)
INCLUDE_DIRECTORIES(include ../interface include/BasicUsageEnvironment include/groupsock include/liveMedia include/UsageEnvironment)



