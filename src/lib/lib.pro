QT += serialport
QT -= gui

TEMPLATE = lib
TARGET = propellermanager
CONFIG += staticlib

CONFIG -= debug_and_release

SOURCES += \
    propellermanager.cpp \
    propellerloader.cpp \
    propellerimage.cpp \
    propellerdevice.cpp \
    propellerprotocol.cpp \
    gpio.cpp \

HEADERS += \
    propellermanager.h \
    propellerloader.h \
    propellerimage.h \
    propellerdevice.h \
    propellerprotocol.h \
    gpio.h \
    input_console.h \
