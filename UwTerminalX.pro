#-------------------------------------------------
#
# Project created by QtCreator 2014-08-19T11:31:30
#
#-------------------------------------------------

QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UwTerminalX
TEMPLATE = app

SOURCES += main.cpp\
    LrdScrollEdit.cpp \
    UwxMainWindow.cpp \
    UwxPopup.cpp \
    LrdLogger.cpp \
    UwxAutomation.cpp

HEADERS  += \
    LrdScrollEdit.h \
    UwxMainWindow.h \
    UwxPopup.h \
    LrdLogger.h \
    UwxAutomation.h

FORMS    += \
    UwxPopup.ui \
    UwxMainWindow.ui \
    UwxAutomation.ui

RESOURCES += \
    UwTerminalXImages.qrc

win32:RC_FILE = version.rc
win32:RC_ICONS = images/UwTerminal32.ico

ICON = MacUwTerminalXIcon.icns

DISTFILES +=
