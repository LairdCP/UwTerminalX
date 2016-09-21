#UwTerminalX Qt project qmake file
#By default all components are built for Github UwTerminalX releases

#Uncomment to exclude building automation form
#DEFINES += "SKIPAUTOMATIONFORM"
#Uncomment to exclude building error code lookup form
#DEFINES += "SKIPERRORCODEFORM"
#Uncomment to exclude building scripting form
#DEFINES += "SKIPSCRIPTINGFORM=1"


QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UwTerminalX
TEMPLATE = app

SOURCES += main.cpp\
    LrdScrollEdit.cpp \
    UwxMainWindow.cpp \
    UwxPopup.cpp \
    LrdLogger.cpp \
    UwxEscape.cpp

HEADERS  += \
    LrdScrollEdit.h \
    UwxMainWindow.h \
    UwxPopup.h \
    LrdLogger.h \
    UwxEscape.h

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

#Scripting form
!contains(DEFINES, SKIPSCRIPTINGFORM)
{
    SOURCES += LrdCodeEditor.cpp \
    LrdHighlighter.cpp \
    UwxScripting.cpp

    HEADERS += LrdCodeEditor.h \
    LrdHighlighter.h \
    UwxScripting.h

    FORMS += UwxScripting.ui
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

