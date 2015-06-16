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

FORMS    +=

RESOURCES += \
    termnotifystore.qrc

win32:RC_FILE = version.rc
win32:RC_ICONS = images/TermNotify.ico

ICON = MacTermNotifyIcon.icns
