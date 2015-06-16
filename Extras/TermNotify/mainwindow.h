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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QMainWindow>
#include <QSerialPortInfo>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QList>
#include <QMenu>
#include <QProcess>

/******************************************************************************/
// Defines
/******************************************************************************/
#define DisplayTime 5000 //Time in mS to display a balloon message for (OS's can override this value)
#define ScanTime 800 //Time in mS to scan for new devices
#define TermNotifyVer "Version 0.53" //Version string

#ifdef __APPLE__
#error MAC OSX is not currently supported.
#endif

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow
    (
    QWidget *parent = 0
    );
    ~MainWindow
    (
    );

private slots:
    void SerialCheck
    (
    );
    void CloseApplication
    (
    QAction *Act
    );
    void OpenProgram
    (
    );

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon gstSysTrayIcon; //System tray icon
    QTimer gtSerialCheckTimer; //Serial port check timer
    QSerialPortInfo gspiSerialPortInfo; //Serial port information
    QList<QString> glPorts; //List of cached serial ports
    bool gbInitialScan; //True when the initial scan has been completed
    QMenu *gpContextMenu; //Pointer to context menu
    QString gstrSerialName; //Selected serial port name
    QProcess gpTerminalProcess; //Process spawner
    QImage giUw16Image; //Holder for 16x16 icon
    QPixmap *gpUw16Pixmap; //Pointer to pixmap for 16x16 icon
};

#endif // MAINWINDOW_H
