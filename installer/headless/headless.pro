QT += core network
QT -= gui

CONFIG += c++11

TARGET = headless
CONFIG += console
CONFIG -= app_bundle

LIBS += -lboost_system

TEMPLATE = app

SOURCES += main.cpp

headless.files = headless
headless.path = /${out}/bin/

INSTALLS += headless

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libinstaller/release/ -llibinstaller
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libinstaller/debug/ -llibinstaller
else:unix: LIBS += -L$$OUT_PWD/../libinstaller/ -llibinstaller

INCLUDEPATH += $$PWD/../libinstaller
DEPENDPATH += $$PWD/../libinstaller
