#UwTerminalX Qt project qmake file
#By default all components are built for Github UwTerminalX releases

#Uncomment to exclude building automation form
#DEFINES += "SKIPAUTOMATIONFORM"
#Uncomment to exclude building error code lookup form
#DEFINES += "SKIPERRORCODEFORM"
#Uncomment to exclude building scripting form
#DEFINES += "SKIPSCRIPTINGFORM=1"
#Uncomment to exclude building speed test functionality
#DEFINES += "SKIPSPEEDTEST=1"
#Uncomment to exclude BL654 USB autorun escape functionality
#DEFINES += "SKIPUSBRECOVERY=1"
#Uncomment to resolve IP address from hostname prior to making web request
#DEFINES == "RESOLVEIPSEPARATELY=1"


QT       += core gui widgets serialport network

TARGET = UwTerminalX
TEMPLATE = app

SOURCES += main.cpp\
    CommandGroup.cpp \
    InputDialog.cpp \
    LrdScrollEdit.cpp \
    PredefinedCommand.cpp \
    PredefinedCommandsTableModel.cpp \
    UwxMainWindow.cpp \
    UwxPopup.cpp \
    LrdLogger.cpp \
    UwxEscape.cpp \
    UwxPredefinedCommands.cpp \
    UwxPredefinedCommandsTab.cpp

HEADERS  += \
    CommandGroup.h \
    InputDialog.h \
    LrdScrollEdit.h \
    PredefinedCommand.h \
    PredefinedCommandsTableModel.h \
    UwxMainWindow.h \
    UwxPopup.h \
    LrdLogger.h \
    UwxEscape.h \
    UwxPredefinedCommands.h \
    UwxPredefinedCommandsTab.h

FORMS    += \
    UwxPopup.ui \
    UwxMainWindow.ui \
    UwxPredefinedCommands.ui \
    UwxPredefinedCommandsTab.ui

RESOURCES += \
    UwTerminalXImages.qrc

#Automation form
!contains(DEFINES, SKIPAUTOMATIONFORM) {
    SOURCES += UwxAutomation.cpp
    HEADERS += UwxAutomation.h
    FORMS += UwxAutomation.ui
}

#Scripting form
!contains(DEFINES, SKIPSCRIPTINGFORM) {
    SOURCES += LrdCodeEditor.cpp \
    LrdHighlighter.cpp \
    UwxScripting.cpp

    HEADERS += LrdCodeEditor.h \
    LrdHighlighter.h \
    UwxScripting.h

    FORMS += UwxScripting.ui
}

#Error code form
!contains(DEFINES, SKIPERRORCODEFORM) {
    SOURCES += UwxErrorCode.cpp
    HEADERS += UwxErrorCode.h
    FORMS += UwxErrorCode.ui
}

#BL654 USB dongle autorun recovery
!contains(DEFINES, SKIPUSBRECOVERY) {
    #Linux libraries
    unix:!macx: LIBS += -lusb-1.0
    unix:!macx: LIBS += -lftdi1

    #Windows libraries
    !contains(QMAKESPEC, g++) {
        #MSVC build for windows
        contains(QT_ARCH, i386) {
            #32-bit windows
            win32: LIBS += -L$$PWD/FTDI/Win32/ -lftd2xx
        } else {
            #64-bit windows
            win32: LIBS += -L$$PWD/FTDI/Win64/ -lftd2xx
        }

        HEADERS  += FTDI/ftd2xx.h
    }
}

#Windows application version information
win32:RC_FILE = version.rc

#Windows application icon
win32:RC_ICONS = images/UwTerminal32.ico

#Mac application icon
ICON = MacUwTerminalXIcon.icns
