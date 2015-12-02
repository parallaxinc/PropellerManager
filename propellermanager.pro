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

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

libs.path       = $$PREFIX/lib
bins.path       = $$PREFIX/bin
includes.path   = $$PREFIX/include

libs.files      = $$PWD/lib/*
bins.files      = $$PWD/bin/*
includes.files  = $$PWD/include/*

INSTALLS += libs includes bins
