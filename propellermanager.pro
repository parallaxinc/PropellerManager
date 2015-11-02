TEMPLATE = subdirs
SUBDIRS = \
    src \

docs.commands = cd doc && bash process.sh
QMAKE_EXTRA_TARGETS += docs
QMAKE_CLEAN += doc/html/* doc/doxygen_sqlite3.db
