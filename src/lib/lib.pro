QT += serialport
QT -= gui

TEMPLATE = lib
TARGET = propellermanager
CONFIG += staticlib

CONFIG -= debug_and_release
CONFIG += debug

SOURCES += \
    propellermanager.cpp \
    propellersession.cpp \
    propellerloader.cpp \
    propellerimage.cpp \
    propellerdevice.cpp \
    propellerprotocol.cpp \
    gpio.cpp \

HEADERS += \
    propellermanager.h \
    propellersession.h \
    propellerloader.h \
    propellerimage.h \
    propellerdevice.h \
    propellerprotocol.h \
    gpio.h \
    input_console.h \
