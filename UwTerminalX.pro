#UwTerminalX Qt project qmake file

#Uncomment to exclude building automation form
#DEFINES += "SKIPAUTOMATIONFORM"
#Uncomment to exclude building error code lookup form
DEFINES += "SKIPERRORCODEFORM"
#Uncomment to exclude building testing form
DEFINES += "SKIPTESTINGFORM"


QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UwTerminalX
TEMPLATE = app

SOURCES += main.cpp\
    LrdScrollEdit.cpp \
    UwxMainWindow.cpp \
    UwxPopup.cpp \
    LrdLogger.cpp

HEADERS  += \
    LrdScrollEdit.h \
    UwxMainWindow.h \
    UwxPopup.h \
    LrdLogger.h

FORMS    += \
    UwxPopup.ui \
    UwxMainWindow.ui

RESOURCES += \
    UwTerminalXImages.qrc

#Automation form
!contains(DEFINES, SKIPAUTOMATIONFORM)
{
    SOURCES += UwxAutomation.cpp
    HEADERS += UwxAutomation.h
    FORMS += UwxAutomation.ui
}

#Testing form
!contains(DEFINES, SKIPTESTINGFORM)
{
    SOURCES += \
    QtCodeEditor.cpp \
    UwxHighlighter.cpp \
    UwxTesting.cpp

    HEADERS += \
    QtCodeEditor.h \
    UwxHighlighter.h \
    UwxTesting.h

    FORMS += \
    UwxTesting.ui
}

#Error code form
!contains(DEFINES, SKIPERRORCODEFORM)
{
    SOURCES += UwxErrorCode.cpp
    HEADERS += UwxErrorCode.h
    FORMS += UwxErrorCode.ui
}

#Windows application version information
win32:RC_FILE = version.rc

#Windows application icon
win32:RC_ICONS = images/UwTerminal32.ico

#Mac application icon
ICON = MacUwTerminalXIcon.icns

DISTFILES +=

