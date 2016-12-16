#-------------------------------------------------
#
# Project created by QtCreator 2016-07-31T15:48:36
#
#-------------------------------------------------

QT       -= gui
QT += network

CONFIG += debug

LIBS += -lqhttp -lparted -lboost_system

TARGET = libinstaller
TEMPLATE = lib

DEFINES += LIBINSTALLER_LIBRARY

SOURCES += libinstaller.cpp \
    device.cpp \
    mountpoint.cpp \
    clienthandler.cpp \
    disk.cpp

HEADERS += libinstaller.h\
        libinstaller_global.h \
    device.h \
    mountpoint.h \
    clienthandler.h \
    disk.h

unix {
    target.path = /${out}/lib
    INSTALLS += target
}
