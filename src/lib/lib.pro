QT += serialport

TEMPLATE = lib
TARGET = propellermanager
CONFIG += staticlib

CONFIG -= debug_and_release

SOURCES += \
    propellersession.cpp \
    propellerimage.cpp \
    propellerdevice.cpp \
    propellerprotocol.cpp \
    gpio.cpp \

HEADERS += \
    propellersession.h \
    propellerimage.h \
    propellerdevice.h \
    propellerprotocol.h \
    gpio.h \
    input_console.h \
    portmonitor.h \
