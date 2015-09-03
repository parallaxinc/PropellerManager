QT += serialport

TEMPLATE = app

CONFIG += console
CONFIG -= app_bundle debug_and_release

INCLUDEPATH += ../../src/lib/
LIBS += -L     ../../src/lib/  -lpropellermanager

win32-msvc* {
	PRE_TARGETDEPS += ../../src/lib/propellermanager.lib
} else {
	PRE_TARGETDEPS += ../../src/lib/libpropellermanager.a
}

