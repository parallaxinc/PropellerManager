QT += serialport
QT -= gui
CONFIG += staticlib

TEMPLATE = lib
TARGET = propellermanager
DESTDIR = ../lib/

CONFIG -= debug_and_release
CONFIG += debug

SOURCES += \
    manager/propellermanager.cpp \
    session/propellersession.cpp \
    loader/propellerloader.cpp \
    loader/protocol.cpp \
    terminal/propellerterminal.cpp \
    image/propellerimage.cpp \
    device/propellerdevice.cpp \
    device/gpio.cpp \
    util/logging.cpp \

HEADERS += \
    manager/propellermanager.h \
    session/propellersession.h \
    session/readbuffer.h \
    loader/propellerloader.h \
    loader/protocol.h \
    terminal/propellerterminal.h \
    terminal/console.h \
    image/propellerimage.h \
    device/propellerdevice.h \
    device/gpio.h \
    util/logging.h \
