include(../common.pri)

TEMPLATE = lib
TARGET = propellermanager
DESTDIR = $$TOP_PWD/lib/

CONFIG += staticlib

include("common/include.pri")
include("propellermanager/include.pri")
include("propellersession/include.pri")
include("propellerdevice/include.pri")
include("propellerloader/include.pri")
include("propellerterminal/include.pri")
include("propellerimage/include.pri")

target.path   = $$PREFIX/lib
INSTALLS += target
