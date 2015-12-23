QT += serialport

TOP_PWD = $$PWD

INCLUDEPATH += $$TOP_PWD/include/
LIBS        += -L$$TOP_PWD/lib/ -lpropellermanager

win32-msvc* {
	PRE_TARGETDEPS += $$TOP_PWD/lib/propellermanager.lib
} else {
	PRE_TARGETDEPS += $$TOP_PWD/lib/libpropellermanager.a
}

#message(include.pri: $$TOP_PWD)
