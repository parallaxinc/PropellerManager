TEMPLATE = subdirs
SUBDIRS = \
    lib \
    propman \
    example \

propman.depends = lib
example.depends = lib
