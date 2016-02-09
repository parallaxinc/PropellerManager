include(../common.pri)

TEMPLATE = lib
TARGET = propellermanager
DESTDIR = $$TOP_PWD/lib/

CONFIG += staticlib

SOURCES += \
    logging.cpp \
    propellerdevice.cpp \
    gpio.cpp \
    propellerimage.cpp \
    propellerloader.cpp \
    protocol.cpp \
    propellermanager.cpp \
    portmonitor.cpp \
    readbuffer.cpp \
    sessionmanager.cpp \
    propellersession.cpp \
    propellerterminal.cpp \

HEADERS += \
    template/connector.h \
    template/interface.h \
    template/manager.h \
    logging.h \
    propellerdevice.h \
    gpio.h \
    propellerimage.h \
    propellerloader.h \
    protocol.h \
    devicemanager.h \
    portmonitor.h \
    propellermanager.h \
    readbuffer.h \
    sessioninterface.h \
    sessionmanager.h \
    propellersession.h \
    console.h \
    propellerterminal.h \

target.path   = $$PREFIX/lib
INSTALLS += target
