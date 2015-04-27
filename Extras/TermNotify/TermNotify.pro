#-------------------------------------------------
#
# Project created by QtCreator 2015-01-12T15:27:57
#
#-------------------------------------------------

QT       += core serialport
QT -= gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TermNotify
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += \
    mainwindow.ui

RESOURCES += \
    termnotifystore.qrc
