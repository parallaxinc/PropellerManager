QT += widgets serialport

TARGET = propterm
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    console.cpp

HEADERS += \
    mainwindow.h \
    settingsdialog.h \
    console.h

FORMS += \
    forms/mainwindow.ui \
    forms/settingsdialog.ui
