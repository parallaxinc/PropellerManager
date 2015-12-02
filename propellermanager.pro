VERSION = $$(VERSION)

isEmpty(VERSION) {
    VERSION = 0.0.0
}

TEMPLATE = subdirs
SUBDIRS = \
    src \
    app \

app.depends = src

docs.commands = cd doc && bash process.sh
QMAKE_EXTRA_TARGETS += docs
QMAKE_CLEAN += doc/html/* doc/doxygen_sqlite3.db


OTHER_FILES += \
    $$PWD/include/PropellerImage \
    $$PWD/include/PropellerSession \
    $$PWD/include/PropellerManager \
    $$PWD/include/PropellerLoader \
    $$PWD/include/PropellerTerminal \

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

libs.files      = lib/*
bins.files      = bin/*
includes.files  = include/*

win32 {
    libs.path       = $$PREFIX
    bins.path       = $$PREFIX
    includes.path   = $$PREFIX
} else {
    libs.path       = $$PREFIX/lib
    bins.path       = $$PREFIX/bin
    includes.path   = $$PREFIX/include
}

INSTALLS += libs includes bins
