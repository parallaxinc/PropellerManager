QT += serialport
QT -= gui

CONFIG -= debug_and_release app_bundle

VERSION = $$(VERSION)
isEmpty(VERSION) {
    VERSION = 0.0.0
}

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

TOP_PWD = $$PWD

!greaterThan(QT_MAJOR_VERSION, 4): {
    error("PropellerIDE requires Qt5.2 or greater")
}
!greaterThan(QT_MINOR_VERSION, 1): {
    error("PropellerIDE requires Qt5.2 or greater")
}

#message(common.pri: $$TOP_PWD)
