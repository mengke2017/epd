#-------------------------------------------------
#
# Project created by QtCreator 2019-12-24T15:17:02
#
#-------------------------------------------------

QT       += core gui network xml serialport

LIBS     += -L/usr/lib -lbz2

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = epd-ui
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    backstagemanager.cpp \
    Module/childline.cpp \
    Module/mainline.cpp \
    Module/top.cpp \
    Module/stationname.cpp \
    e-paper/miniGUI.c \
    e-paper/IT8951_USB.c \
    e-paper/IT8951_CMD.c \
    tcp/client.cpp \
    fileutils.cpp \
    Module/bulletin.cpp \
    tcp/http.cpp \
    systemutils.cpp \
    e-paper/e-paper.cpp \
    battery/batterymanager.cpp \
    battery/batterylist.cpp

HEADERS += \
        mainwindow.h \
    backstagemanager.h \
    Module/top.h \
    Module/mainline.h \
    Module/childline.h \
    Module/stationname.h \
    customize.h \
    e-paper/miniGUI.h \
    e-paper/IT8951_USB.h \
    e-paper/IT8951_CMD.h \
    tcp/client.h \
    fileutils.h \
    Module/bulletin.h \
    bzlib.h \
    tcp/StationCommand.h \
    tcp/http.h \
    systemutils.h \
    e-paper/e-paper.h \
    battery/batterymanager.h \
    battery/batterylist.h

FORMS += \
        mainwindow.ui

RESOURCES += \
    image.qrc \
    weather_ico.qrc
