QT += widgets serialport

TARGET = propterm
TEMPLATE = app
CONFIG -= app_bundle debug_and_release

INCLUDEPATH += ../
LIBS += -L../  -lpropellermanager

win32-msvc* {
	PRE_TARGETDEPS += ../propellermanager.lib
} else {
	PRE_TARGETDEPS += ../libpropellermanager.a
}

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
