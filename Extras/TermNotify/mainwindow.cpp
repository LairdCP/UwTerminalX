/******************************************************************************
** Copyright (C) 2014-2015 Ezurio Ltd
**
** Project: TermNotify
**
** Module: mainwindow.cpp
**
** Notes:
**
*******************************************************************************/

/******************************************************************************/
// Include Files
/******************************************************************************/
#include "mainwindow.h"

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
MainWindow::MainWindow(QObject *parent) : QObject(parent)
{
#if TARGET_OS_MAC
    //On mac, get the directory of the bundle (which will be <location>/TermNotify.app/Contents/MacOS) and go up to the folder with the file in
    QDir BundleDir(QCoreApplication::applicationDirPath());
    BundleDir.cdUp();
    BundleDir.cdUp();
    BundleDir.cdUp();
    QString strMacBundlePath = BundleDir.path().append("/");
    QSettings stgSettings(QString(strMacBundlePath).append("TermNotify.ini"), QSettings::IniFormat);
#else
    QSettings stgSettings("TermNotify.ini", QSettings::IniFormat);
#endif

    //Load settings
    gstrExecutable = stgSettings.value("RunFile",
#ifdef _WIN32
        //Windows
        "UwTerminalX"
#elif TARGET_OS_MAC
        //Mac
        "%DIRPATH%/UwTerminalX.app/Contents/MacOS/UwTerminalX"
#else
        //Linux
        "./UwTerminalX"
#endif
    ).toString();
    gintDisplayTime = stgSettings.value("DisplayTime", DefaultDisplayTime).toUInt();
    gintScanTime = stgSettings.value("ScanTime", DefaultScanTime).toUInt();

    if (stgSettings.value("RunFile").isNull())
    {
        //Set defaults
        stgSettings.setValue("RunFile", gstrExecutable);
        stgSettings.setValue("DisplayTime", gintDisplayTime);
        stgSettings.setValue("ScanTime", gintScanTime);
    }

#ifdef TARGET_OS_MAC
    //Replaces instances of %DIRPATH% with the folder location that TermNotify is in
    gstrExecutable.replace("%DIRPATH%", strMacBundlePath);
#endif

    //Create system tray menu
    gpContextMenu = new QMenu;
    gpContextMenu->addAction(new QAction("Laird TermNotify", this));
    gpContextMenu->addAction(new QAction(TermNotifyVer, this));
    gpContextMenu->addSeparator();
    gpContextMenu->addAction(new QAction("Exit", this));
    gpContextMenu->actions()[0]->setDisabled(true);
    gpContextMenu->actions()[1]->setDisabled(true);

    //Create system tray object
#ifdef _WIN32
    giUw16Image = QImage(":/images/TermNotify.ico");
#else
    giUw16Image = QImage(":/images/TermNotify.png");
#endif
    gpUw16Pixmap = new QPixmap(QPixmap::fromImage(giUw16Image));
    gstSysTrayIcon.setIcon(QIcon(*gpUw16Pixmap));
    gstSysTrayIcon.setContextMenu(gpContextMenu);
    gstSysTrayIcon.show();

    //Connect up the context menu
    connect(gpContextMenu, SIGNAL(triggered(QAction*)), this, SLOT(CloseApplication(QAction*)));

    //Connect balloon meesage clicked event
    connect(&gstSysTrayIcon, SIGNAL(messageClicked()), this, SLOT(OpenProgram()));

    //Connect timer
    connect(&gtSerialCheckTimer, SIGNAL(timeout()), this, SLOT(SerialCheck()));

    //Default list
    glPorts.clear();
    gbInitialScan = false;

    //Start the timer
    gtSerialCheckTimer.setInterval(gintScanTime);
    gtSerialCheckTimer.start();
}

//=============================================================================
//=============================================================================
MainWindow::~MainWindow
    (
    )
{
    //Disconnect events
    disconnect(this, SLOT(CloseApplication(QAction*)));
    disconnect(this, SLOT(OpenProgram()));
    disconnect(this, SLOT(SerialCheck()));

    //Delete context menu
    delete gpContextMenu;

    //Remove system tray icon
    gstSysTrayIcon.hide();

    //Delete pixmap
    delete gpUw16Pixmap;
}

//=============================================================================
//=============================================================================
void
MainWindow::SerialCheck
    (
    )
{
    //Checks is a new serial device has been detected
    QList<QSerialPortInfo> lSerialPorts = gspiSerialPortInfo.availablePorts();
    QSerialPortInfo spiThisSerialPort;
    if (gbInitialScan == true)
    {
        QList<QString> lTempList;
        lTempList.clear();
        foreach(spiThisSerialPort, lSerialPorts)
        {
            if (glPorts.indexOf(spiThisSerialPort.portName()) == -1)
            {
                //New port!
                gstSysTrayIcon.showMessage("Detected new serial port!", QString("New port; ").append(spiThisSerialPort.portName()).append(". Click to open UwTerminalX!"), QSystemTrayIcon::Information, gintDisplayTime);
                gstrSerialName = spiThisSerialPort.portName();
            }
            lTempList << spiThisSerialPort.portName();
        }
        glPorts = lTempList;
    }
    else
    {
        //Populate initial list
        foreach(spiThisSerialPort, lSerialPorts)
        {
            glPorts << spiThisSerialPort.portName();
        }
        gbInitialScan = true;
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::OpenProgram
    (
    )
{
    //Opens UwTerminalX when clicked
    gpTerminalProcess.startDetached(gstrExecutable, QStringList() << "ACCEPT" << QString("COM=").append(gstrSerialName) << "NOCONNECT");
}

//=============================================================================
//=============================================================================
void
MainWindow::CloseApplication
    (
    QAction *Act
    )
{
    //Closes the application
    QApplication::quit();
}
