include(../common.pri)

TEMPLATE = lib
TARGET = propellermanager
DESTDIR = $$TOP_PWD/lib/

CONFIG += staticlib

include("manager/include.pri")
include("session/include.pri")
include("device/include.pri")
include("loader/include.pri")
include("terminal/include.pri")
include("image/include.pri")

SOURCES += \
    util/logging.cpp \

HEADERS += \
    util/logging.h \

target.path   = $$PREFIX/lib
INSTALLS += target
