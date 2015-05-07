QT += serialport

TEMPLATE = app
TARGET = propman
INCLUDEPATH += . ..

CONFIG += console
CONFIG -= debug_and_release

SOURCES += \
    main.cpp \
    loader.cpp \
    gpio.cpp \

HEADERS += \
    loader.h \
    gpio.h \
