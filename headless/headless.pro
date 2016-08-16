QT += core network
QT -= gui

CONFIG += c++11
LIBS += -lqhttp

TARGET = headless
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

headless.files = headless
headless.path = /${out}/bin/

INSTALLS += headless
