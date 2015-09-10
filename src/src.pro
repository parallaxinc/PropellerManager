TEMPLATE = subdirs
SUBDIRS = \
    lib \
    propman \
    propterm \

propman.depends = lib
propterm.depends = lib
