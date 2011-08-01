PROJECT(interface C)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(InterfaceSrc frame.c interface.c mutex.c picture.c)

#IF(WIN32)

SET(InterfaceSrc ${InterfaceSrc} frame.h mutex.h picture.h interface.h)
#ENDIF(WIN32)

#ADD_EXECUTABLE(test ${TestSrc})
ADD_LIBRARY(interface STATIC ${InterfaceSrc})

SUBDIRS(test)

SET_TARGET_PROPERTIES(interface PROPERTIES LINKER_LANGUAGE C)

