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

install_libs.path       = $$PREFIX/lib
install_bins.path       = $$PREFIX/bin
install_includes.path   = $$PREFIX/include

install_libs.files      = $$PWD/lib/*
install_bins.files      = $$PWD/bin/*
install_includes.files  = $$PWD/include/*

INSTALLS += install_libs install_includes install_bins
