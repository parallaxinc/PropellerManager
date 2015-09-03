TEMPLATE = subdirs
SUBDIRS = \
    lib \
    cli \

cli.depends = lib
