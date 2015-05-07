QT += serialport

TEMPLATE = app
TARGET = propman
INCLUDEPATH += . ..

CONFIG += console
CONFIG -= debug_and_release

SOURCES += \
    main.cpp \
    ../Loader.cpp \
    ../GPIO.cpp \

HEADERS += \
    ../Loader.h \
    ../GPIO.h \
