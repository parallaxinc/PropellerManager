include(../common.pri)

TEMPLATE = aux

target.path   = $$PREFIX/include
target.files  += $$files($$TOP_PWD/include/*)
target.files  -= $$TOP_PWD/include/include.pro
INSTALLS += target
