QT += widgets serialport

TARGET = propterm
TEMPLATE = app
CONFIG -= app_bundle debug_and_release

SOURCES += \
    main.cpp \
    propterm.cpp \
    console.cpp

HEADERS += \
    propterm.h \
    console.h

FORMS += \
    propterm.ui \

RESOURCES += \
    icons/propterm/propterm.qrc \
