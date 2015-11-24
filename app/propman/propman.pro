QT += serialport
QT -= gui

TEMPLATE = app
TARGET = propman
DESTDIR = ../../bin/

CONFIG += console
CONFIG -= app_bundle debug_and_release

INCLUDEPATH += ../../include/
LIBS += -L../../lib/  -lpropellermanager

win32-msvc* {
	PRE_TARGETDEPS += ../../lib/propellermanager.lib
} else {
	PRE_TARGETDEPS += ../../lib/libpropellermanager.a
}

SOURCES += \
    main.cpp
