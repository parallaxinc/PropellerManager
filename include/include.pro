include(../common.pri)

TEMPLATE = aux

target.path   = $$PREFIX/include
target.files  = $$files($$TOP_PWD/include/*)
INSTALLS += target
