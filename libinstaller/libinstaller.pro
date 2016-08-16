#-------------------------------------------------
#
# Project created by QtCreator 2016-07-31T15:48:36
#
#-------------------------------------------------

QT       -= gui

TARGET = libinstaller
TEMPLATE = lib

DEFINES += LIBINSTALLER_LIBRARY

SOURCES += libinstaller.cpp \
    device.cpp \
    mountpoint.cpp

HEADERS += libinstaller.h\
        libinstaller_global.h \
    device.h \
    mountpoint.h

unix {
    target.path = /${out}/lib
    INSTALLS += target
}
