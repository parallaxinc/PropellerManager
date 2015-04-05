QT += widgets serialport

TARGET = propterm
TEMPLATE = app

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
