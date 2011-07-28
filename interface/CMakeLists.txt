PROJECT(test C)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(TestSrc test.c frame.c interface.c)

#IF(WIN32)
SET(TestSrc ${TestSrc} interface.h frame.h)
#ENDIF(WIN32)

ADD_EXECUTABLE(test ${TestSrc})

#TARGET_LINK_LIBRARIES(test dl)

SET_TARGET_PROPERTIES(test PROPERTIES LINKER_LANGUAGE C)

