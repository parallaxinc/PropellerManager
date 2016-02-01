include(../common.pri)

TEMPLATE = lib
TARGET = propellermanager
DESTDIR = $$TOP_PWD/lib/

CONFIG += staticlib

SOURCES += \
    manager/propellermanager.cpp \
    manager/portmonitor.cpp \
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
    manager/manager.h \
    manager/sessionmanager.h \
    manager/devicemanager.h \
    manager/portmonitor.h \
    manager/connector.h \
    manager/readbuffer.h \
    manager/sessioninterface.h \
    manager/deviceinterface.h \
    manager/interface.h \
    session/propellersession.h \
    loader/propellerloader.h \
    loader/protocol.h \
    terminal/propellerterminal.h \
    terminal/console.h \
    image/propellerimage.h \
    device/propellerdevice.h \
    device/gpio.h \
    util/logging.h \

target.path   = $$PREFIX/lib
INSTALLS += target
