PROJECT(interface C)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(InterfaceSrc frame.c interface.c mutex.c picture.c)

#IF(WIN32)
SET(InterfaceSrc ${InterfaceSrc} frame.h mutex.h picture.h interface.h)
#ENDIF(WIN32)

find_library(SDL_LIB SDL)


#ADD_EXECUTABLE(test ${TestSrc})
ADD_LIBRARY(interface STATIC ${InterfaceSrc})
TARGET_LINK_LIBRARIES(interface ${SDL_LIB})


SET_TARGET_PROPERTIES(interface PROPERTIES LINKER_LANGUAGE C)

