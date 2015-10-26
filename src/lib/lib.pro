QT += serialport
QT -= gui

TEMPLATE = lib
TARGET = propellermanager
CONFIG += staticlib

CONFIG -= debug_and_release

SOURCES += \
    propellermanager.cpp \
    propellersession.cpp \
    propellerimage.cpp \
    propellerdevice.cpp \
    propellerprotocol.cpp \
    gpio.cpp \

HEADERS += \
    propellermanager.h \
    propellersession.h \
    propellerimage.h \
    propellerdevice.h \
    propellerprotocol.h \
    gpio.h \
    input_console.h \
    portmonitor.h \
