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
#include "ui_mainwindow.h"

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    //Create system tray menu
    gpContextMenu = new QMenu;
    gpContextMenu->addAction(new QAction("Laird TermNotify", this));
    gpContextMenu->addAction(new QAction(TermNotifyVer, this));
    gpContextMenu->addSeparator();
    gpContextMenu->addAction(new QAction("Exit", this));
    gpContextMenu->actions()[0]->setDisabled(true);
    gpContextMenu->actions()[1]->setDisabled(true);

    //Create system tray object
    giUw16Image = QImage(":/images/TermNotify.ico");
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
    gtSerialCheckTimer.setInterval(ScanTime);
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

    //Delete UI
    delete ui;
}

//=============================================================================
//=============================================================================
void MainWindow::SerialCheck
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
                gstSysTrayIcon.showMessage("Detected new serial port!", QString("New port; ").append(spiThisSerialPort.portName()).append(". Click to open UwTerminalX!"), QSystemTrayIcon::Information, DisplayTime);
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
void MainWindow::OpenProgram
(
)
{
    //Opens UwTerminalX when clicked
    gpTerminalProcess.startDetached(
#ifdef _WIN32
    //Windows
    "UwTerminalX.exe"
#elif __APPLE__
    //Mac
#pragma warning("TODO: MAC support");
    "UwTerminalX"
#else
    //Assume linux
    "./UwTerminalX"
#endif
    , QStringList() << "ACCEPT" << QString("COM=").append(gstrSerialName) << "NOCONNECT");
}

//=============================================================================
//=============================================================================
void MainWindow::CloseApplication
(
    QAction *Act
)
{
    //Closes the application
    QCoreApplication::quit();
}
