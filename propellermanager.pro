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
