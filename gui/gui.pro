# -------------------------------------------------
# Project created by QtCreator 2011-06-02T14:42:08
# -------------------------------------------------
TARGET = OpenLive4Cam
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    CInterface.cpp \
    capturecom.cpp \
    videoview.cpp \
    serverthread.cpp \
    encoderthread.cpp
HEADERS += mainwindow.h \
    ../interface/interface.h \
    ../interface/picture.h \
    CInterface.h \
    capturecom.h \
    videoview.h \
    serverthread.h \
    encoderthread.h
FORMS += mainwindow.ui
LIBS += -L../interface \
    -linterface
