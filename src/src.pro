QT += serialport

TEMPLATE = app
TARGET = propman
INCLUDEPATH += . ..

CONFIG += console
CONFIG -= debug_and_release

SOURCES += \
    propellersession.cpp \
    propellerimage.cpp \
    gpio.cpp \
    utility.cpp \
    main.cpp \

HEADERS += \
    propellersession.h \
    propellerimage.h \
    gpio.h \
    utility.h \
    input_console.h \
