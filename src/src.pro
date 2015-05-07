QT += serialport

TEMPLATE = app
TARGET = propman
INCLUDEPATH += . ..

CONFIG += console
CONFIG -= debug_and_release

SOURCES += \
    main.cpp \
    propellerdevice.cpp \
    gpio.cpp \
    utility.cpp \

HEADERS += \
    propellerdevice.h \
    gpio.h \
    utility.h \
    input_console.h \
