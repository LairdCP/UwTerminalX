/******************************************************************************
** Copyright (C) 2015-2017 Laird
**
** Project: UwTerminalX
**
** Module: UwxMainWindow.cpp
**
** Notes:
**
** License: This program is free software: you can redistribute it and/or
**          modify it under the terms of the GNU General Public License as
**          published by the Free Software Foundation, version 3.
**
**          This program is distributed in the hope that it will be useful,
**          but WITHOUT ANY WARRANTY; without even the implied warranty of
**          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**          GNU General Public License for more details.
**
**          You should have received a copy of the GNU General Public License
**          along with this program.  If not, see http://www.gnu.org/licenses/
**
*******************************************************************************/

/******************************************************************************/
// Include Files
/******************************************************************************/
#include "UwxMainWindow.h"
#include "ui_UwxMainWindow.h"

/******************************************************************************/
// Conditional Compile Defines
/******************************************************************************/
#ifdef QT_DEBUG
    //Include debug output when compiled for debugging
    #include <QDebug>
#endif
#ifdef _WIN32
    //Windows
    #define OS "Win"
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_MAC
        //Mac OSX
        #define OS "Mac"
        QString gstrMacBundlePath;
    #endif
#else
    //Assume Linux
    #ifdef __aarch64__
        //ARM64
        #define OS "Linux (AArch64)"
    #elif __arm__
        //ARM
        #define OS "Linux (ARM)"
    #elif __x86_64__
        //x86_64
        #define OS "Linux (x86_64)"
    #elif __i386
        //x86
        #define OS "Linux (x86)"
    #else
        //Unknown
        #define OS "Linux (other)"
    #endif
#endif

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    //Setup the GUI
    ui->setupUi(this);

#if TARGET_OS_MAC
    //On mac, get the directory of the bundle (which will be <location>/Term.app/Contents/MacOS) and go up to the folder with the file in
    QDir BundleDir(QCoreApplication::applicationDirPath());
    BundleDir.cdUp();
    BundleDir.cdUp();
    BundleDir.cdUp();
    gstrMacBundlePath = BundleDir.path().append("/");
    if (!QDir().exists(QStandardPaths::writableLocation(QStandardPaths::DataLocation)))
    {
        //Create UwTerminalX directory in application support
        QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    }

    //Fix mac's resize
    resize(720, 360);

    //Disable viewing files externally for mac
    ui->btn_EditExternal->setEnabled(false);

    //Increase duplicate button size for mac
    ui->btn_Duplicate->setMinimumWidth(130);
#endif

#ifndef _WIN32
#ifndef __APPLE__
    //Increase Linux window size to cope with possible large Linux fonts
    resize(this->width()+50, this->height()+20);
#endif
#endif

#ifndef _WIN32
    #ifdef __APPLE__
        //Change size of text fonts for Mac
        QFont fntTmpFnt(ui->label_PreXCompInfo->font());
        fntTmpFnt.setPixelSize(12);
        ui->label_PreXCompInfo->setFont(fntTmpFnt);
        ui->label_OnlineXCompInfo->setFont(fntTmpFnt);
        ui->label_UwTerminalXText->setFont(fntTmpFnt);
        ui->label_ErrorCodeText->setFont(fntTmpFnt);
        ui->label_AppFirmwareText1->setFont(fntTmpFnt);
        ui->label_AppFirmwareText2->setFont(fntTmpFnt);
    #else
        //Change size of text fonts for Linux
        QFont fntTmpFnt(ui->label_PreXCompInfo->font());
        fntTmpFnt.setPixelSize(11);
        ui->label_PreXCompInfo->setFont(fntTmpFnt);
        ui->label_OnlineXCompInfo->setFont(fntTmpFnt);
        ui->label_UwTerminalXText->setFont(fntTmpFnt);
        ui->label_ErrorCodeText->setFont(fntTmpFnt);
        ui->label_AppFirmwareText1->setFont(fntTmpFnt);
        ui->label_AppFirmwareText2->setFont(fntTmpFnt);
    #endif
#endif

    //Define default variable values
    gbTermBusy = false;
    gbStreamingFile = false;
    gintRXBytes = 0;
    gintTXBytes = 0;
    gintQueuedTXBytes = 0;
    gchTermBusyLines = 0;
    gchTermMode = 0;
    gchTermMode2 = 0;
    gbMainLogEnabled = false;
    gbLoopbackMode = false;
    gbSysTrayEnabled = false;
    gbIsUWCDownload = false;
    gbCTSStatus = 0;
    gbDCDStatus = 0;
    gbDSRStatus = 0;
    gbRIStatus = 0;
    gbStreamingBatch = false;
    gbaBatchReceive.clear();
    gbFileOpened = false;
    gbEditFileModified = false;
    giEditFileType = -1;
    gbErrorsLoaded = false;
    gbAutoBaud = false;
    gnmManager = 0;
    gtmrSpeedTestDelayTimer = 0;
    gbSpeedTestRunning = false;

#ifndef SKIPAUTOMATIONFORM
    guaAutomationForm = 0;
#endif
#ifndef SKIPERRORCODEFORM
    gecErrorCodeForm = 0;
#else
    ui->btn_Error->deleteLater();
#endif
#ifndef SKIPSCRIPTINGFORM
    gbScriptingRunning = false;
    gusScriptingForm = 0;
#endif

    //Clear display buffer byte array.
    gbaDisplayBuffer.clear();

    //Load settings from configuration files
    LoadSettings();

    //Create logging handle
    gpMainLog = new LrdLogger();

    //Move to 'About' tab
    ui->selector_Tab->setCurrentIndex(TabAbout);

    //Set default values for combo boxes on 'Config' tab
    ui->combo_Baud->setCurrentIndex(8);
    ui->combo_Stop->setCurrentIndex(0);
    ui->combo_Data->setCurrentIndex(1);
    ui->combo_Handshake->setCurrentIndex(1);

    //Load images
    gimEmptyCircleImage = QImage(":/images/EmptyCircle.png");
    gimRedCircleImage = QImage(":/images/RedCircle.png");
    gimGreenCircleImage = QImage(":/images/GreenCircle.png");
#ifdef _WIN32
    //Load ICOs for windows
    gimUw16Image = QImage(":/images/UwTerminal16.ico");
    gimUw32Image = QImage(":/images/UwTerminal32.ico");
#else
    //Load PNGs for Linux/Mac
    gimUw16Image = QImage(":/images/UwTerminal16.png");
    gimUw32Image = QImage(":/images/UwTerminal32.png");
#endif

#ifndef UseSSL
    //Disable SSL checkbox for non-SSL builds
    ui->check_EnableSSL->setCheckable(false);
    ui->check_EnableSSL->setChecked(false);
    ui->check_EnableSSL->setEnabled(false);
#endif

    //Create pixmaps
    gpEmptyCirclePixmap = new QPixmap(QPixmap::fromImage(gimEmptyCircleImage));
    gpRedCirclePixmap = new QPixmap(QPixmap::fromImage(gimRedCircleImage));
    gpGreenCirclePixmap = new QPixmap(QPixmap::fromImage(gimGreenCircleImage));
    gpUw16Pixmap = new QPixmap(QPixmap::fromImage(gimUw16Image));
    gpUw32Pixmap = new QPixmap(QPixmap::fromImage(gimUw32Image));

    //Show images on help
    ui->label_AboutI1->setPixmap(*gpEmptyCirclePixmap);
    ui->label_AboutI2->setPixmap(*gpRedCirclePixmap);
    ui->label_AboutI3->setPixmap(*gpGreenCirclePixmap);

    //Enable custom context menu policy
    ui->text_TermEditData->setContextMenuPolicy(Qt::CustomContextMenu);

#ifdef _WIN32
    //Connect process termination to signal
    connect(&gprocCompileProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(process_finished(int, QProcess::ExitStatus)));
#endif

    //Connect quit signals
    connect(ui->btn_Decline, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btn_Quit, SIGNAL(clicked()), this, SLOT(close()));

    //Connect key-press signals
    connect(ui->text_TermEditData, SIGNAL(EnterPressed()), this, SLOT(EnterPressed()));
    connect(ui->text_TermEditData, SIGNAL(KeyPressed(QChar)), this, SLOT(KeyPressed(QChar)));

    //Connect file drag/drop signal
    connect(ui->text_TermEditData, SIGNAL(FileDropped(QString)), this, SLOT(DroppedFile(QString)));

    //Initialise popup message
    gpmErrorForm = new PopupMessage;

    //Populate the list of devices
    RefreshSerialDevices();

    //Setup speed test mode timers
    gtmrSpeedTestStats.setInterval(SpeedTestStatUpdateTime);
    gtmrSpeedTestStats.setSingleShot(false);
    connect(&gtmrSpeedTestStats, SIGNAL(timeout()), this, SLOT(UpdateSpeedTestValues()));
    gtmrSpeedTestStats10s.setInterval(10000);
    gtmrSpeedTestStats10s.setSingleShot(false);
    connect(&gtmrSpeedTestStats10s, SIGNAL(timeout()), this, SLOT(OutputSpeedTestStats()));

    //Display version
    ui->statusBar->showMessage(QString("UwTerminalX")
#ifdef UseSSL
    .append("-SSL")
#endif
    .append(" version ").append(UwVersion).append(" (").append(OS).append("), Built ").append(__DATE__).append(" Using QT ").append(QT_VERSION_STR)
#ifdef QT_DEBUG
    .append(" [DEBUG BUILD]")
#endif
    );
    setWindowTitle(QString("UwTerminalX (v").append(UwVersion).append(")"));

    //Create menu items
    gpMenu = new QMenu(this);
    gpMenu->addAction("XCompile")->setData(MenuActionXCompile);
    gpMenu->addAction("XCompile + Load")->setData(MenuActionXCompileLoad);
    gpMenu->addAction("XCompile + Load + Run")->setData(MenuActionXCompileLoadRun);
    gpMenu->addAction("Load")->setData(MenuActionLoad);
    gpMenu->addAction("Load + Run")->setData(MenuActionLoadRun);
    gpMenu->addAction("Lookup Selected Error-Code (Hex)")->setData(MenuActionErrorHex);
    gpMenu->addAction("Lookup Selected Error-Code (Int)")->setData(MenuActionErrorInt);
    gpMenu->addAction("Enable Loopback (Rx->Tx)")->setData(MenuActionLoopback);
    gpSMenu1 = gpMenu->addMenu("Download");
    gpSMenu2 = gpSMenu1->addMenu("BASIC");
    gpSMenu2->addAction("Load Precompiled BASIC")->setData(MenuActionLoad2);
    gpSMenu2->addAction("Erase File")->setData(MenuActionEraseFile);
    gpSMenu2->addAction("Dir")->setData(MenuActionDir);
    gpSMenu2->addAction("Run")->setData(MenuActionRun);
    gpSMenu2->addAction("Debug")->setData(MenuActionDebug);
    gpSMenu3 = gpSMenu1->addMenu("Data");
    gpSMenu3->addAction("Data File +")->setData(MenuActionDataFile);
    gpSMenu3->addAction("Erase File +")->setData(MenuActionEraseFile2);
    gpSMenu3->addAction("Clear Filesystem")->setData(MenuActionClearFilesystem);
    gpSMenu3->addAction("Multi Data File +")->setData(MenuActionMultiDataFile); //Downloads more than 1 data file
    gpSMenu1->addAction("Stream File Out")->setData(MenuActionStreamFile);
    gpMenu->addAction("Font")->setData(MenuActionFont);
    gpMenu->addAction("Run")->setData(MenuActionRun2);
    gpMenu->addAction("Automation")->setData(MenuActionAutomation);
    gpMenu->addAction("Scripting")->setData(MenuActionScripting);
    gpMenu->addAction("Batch")->setData(MenuActionBatch);
    gpMenu->addAction("Clear module")->setData(MenuActionClearModule);
    gpMenu->addAction("Clear Display")->setData(MenuActionClearDisplay);
    gpMenu->addAction("Clear RX/TX count")->setData(MenuActionClearRxTx);
    gpMenu->addSeparator();
    gpMenu->addAction("Copy")->setData(MenuActionCopy);
    gpMenu->addAction("Copy All")->setData(MenuActionCopyAll);
    gpMenu->addAction("Paste")->setData(MenuActionPaste);
    gpMenu->addAction("Select All")->setData(MenuActionSelectAll);

    //Create balloon menu items
    gpBalloonMenu = new QMenu(this);
    gpBalloonMenu->addAction("Show UwTerminalX")->setData(BalloonActionShow);
    gpBalloonMenu->addAction("Exit")->setData(BalloonActionExit);

    //Create speed test button items
    gpSpeedMenu = new QMenu(this);
    gpSpeedMenu->addAction("Receive-only test")->setData(SpeedMenuActionRecv);
    gpSpeedMenu->addAction("Send-only test")->setData(SpeedMenuActionSend);
    gpSpeedMenu->addAction("Send && receive test")->setData(SpeedMenuActionSendRecv);
    gpSpeedMenu->addAction("Send && receive test (delay 5 seconds)")->setData(SpeedMenuActionSendRecv5Delay);
    gpSpeedMenu->addAction("Send && receive test (delay 10 seconds)")->setData(SpeedMenuActionSendRecv10Delay);
    gpSpeedMenu->addAction("Send && receive test (delay 15 seconds)")->setData(SpeedMenuActionSendRecv15Delay);

    //Disable unimplemented actions
    gpSMenu3->actions()[3]->setEnabled(false); //Multi Data File +
#ifdef SKIPAUTOMATIONFORM
    //Disable automation option
    gpMenu->actions()[11]->setEnabled(false);
#endif
#ifdef SKIPSCRIPTINGFORM
    //Disable scripting option
    gpMenu->actions()[12]->setEnabled(false);
#endif

    //Connect the menu actions
    connect(gpMenu, SIGNAL(triggered(QAction*)), this, SLOT(MenuSelected(QAction*)), Qt::AutoConnection);
    connect(gpMenu, SIGNAL(aboutToHide()), this, SLOT(ContextMenuClosed()), Qt::AutoConnection);
    connect(gpBalloonMenu, SIGNAL(triggered(QAction*)), this, SLOT(balloontriggered(QAction*)), Qt::AutoConnection);
    connect(gpSpeedMenu, SIGNAL(triggered(QAction*)), this, SLOT(SpeedMenuSelected(QAction*)), Qt::AutoConnection);

    //Configure the module timeout timer
    gtmrDownloadTimeoutTimer.setSingleShot(true);
    gtmrDownloadTimeoutTimer.setInterval(ModuleTimeout);
    connect(&gtmrDownloadTimeoutTimer, SIGNAL(timeout()), this, SLOT(DevRespTimeout()));

    //Configure the signal timer
    gpSignalTimer = new QTimer(this);
    connect(gpSignalTimer, SIGNAL(timeout()), this, SLOT(SerialStatusSlot()));

    //Connect serial signals
    connect(&gspSerialPort, SIGNAL(readyRead()), this, SLOT(SerialRead()));
    connect(&gspSerialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(SerialError(QSerialPort::SerialPortError)));
    connect(&gspSerialPort, SIGNAL(bytesWritten(qint64)), this, SLOT(SerialBytesWritten(qint64)));
    connect(&gspSerialPort, SIGNAL(aboutToClose()), this, SLOT(SerialPortClosing()));

    //Set update text display timer to be single shot only and connect to slot
    gtmrTextUpdateTimer.setSingleShot(true);
    gtmrTextUpdateTimer.setInterval(gpTermSettings->value("TextUpdateInterval", DefaultTextUpdateInterval).toInt());
    connect(&gtmrTextUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateReceiveText()));

    //Set update speed display timer to be single shot only and connect to slot
    gtmrSpeedUpdateTimer.setSingleShot(true);
    gtmrSpeedUpdateTimer.setInterval(gpTermSettings->value("TextUpdateInterval", DefaultTextUpdateInterval).toInt());
    connect(&gtmrSpeedUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateDisplayText()));

    //Setup timer for batch file timeout
    gtmrBatchTimeoutTimer.setSingleShot(true);
    connect(&gtmrBatchTimeoutTimer, SIGNAL(timeout()), this, SLOT(BatchTimeoutSlot()));

    //Setup timer for automatic baud rate detection
    gtmrBaudTimer.setSingleShot(true);
    gtmrBaudTimer.setInterval(AutoBaudTimeout);
    connect(&gtmrBaudTimer, SIGNAL(timeout()), this, SLOT(DetectBaudTimeout()));

    //Set logging options
    ui->edit_LogFile->setText(gpTermSettings->value("LogFile", DefaultLogFile).toString());
    ui->check_LogEnable->setChecked(gpTermSettings->value("LogEnable", DefaultLogEnable).toBool());
    ui->check_LogAppend->setChecked(gpTermSettings->value("LogMode", DefaultLogMode).toBool());

    //Set license checking option
    ui->check_CheckLicense->setChecked(gpTermSettings->value("LicenseCheck", DefaultLicenceCheckMode).toBool());

#ifdef UseSSL
    //Set SSL status
    ui->check_EnableSSL->setChecked(gpTermSettings->value("SSLEnable", DefaultSSLEnable).toBool());
    if (ui->check_EnableSSL->isChecked() == true)
    {
        //HTTPS
        WebProtocol = "https";
    }
    else
    {
        //HTTP
        WebProtocol = "http";
    }
#endif

    //Set showing filesize option
    ui->check_ShowFileSize->setChecked(gpTermSettings->value("ShowFileSize", DefaultShowFileSize).toBool());

    //Set clearing confirmation option
    ui->check_ConfirmClear->setChecked(gpTermSettings->value("ConfirmClear", DefaultConfirmClear).toBool());

    //Check if default devices were created
    if (gpPredefinedDevice->value("DoneSetup").isNull())
    {
        //Create default device configurations... BT900
        gpPredefinedDevice->setValue(QString("Port1Name"), "BT900");
        gpPredefinedDevice->setValue(QString("Port1Baud"), "115200");
        gpPredefinedDevice->setValue(QString("Port1Parity"), "0");
        gpPredefinedDevice->setValue(QString("Port1Stop"), "1");
        gpPredefinedDevice->setValue(QString("Port1Data"), "8");
        gpPredefinedDevice->setValue(QString("Port1Flow"), "1");

        //BL600/BL620
        gpPredefinedDevice->setValue(QString("Port2Name"), "BL600/BL620");
        gpPredefinedDevice->setValue(QString("Port2Baud"), "9600");
        gpPredefinedDevice->setValue(QString("Port2Parity"), "0");
        gpPredefinedDevice->setValue(QString("Port2Stop"), "1");
        gpPredefinedDevice->setValue(QString("Port2Data"), "8");
        gpPredefinedDevice->setValue(QString("Port2Flow"), "1");

        //BL620-US
        gpPredefinedDevice->setValue(QString("Port3Name"), "BL620-US");
        gpPredefinedDevice->setValue(QString("Port3Baud"), "9600");
        gpPredefinedDevice->setValue(QString("Port3Parity"), "0");
        gpPredefinedDevice->setValue(QString("Port3Stop"), "1");
        gpPredefinedDevice->setValue(QString("Port3Data"), "8");
        gpPredefinedDevice->setValue(QString("Port3Flow"), "0");

        //BL652
        gpPredefinedDevice->setValue(QString("Port4Name"), "BL652");
        gpPredefinedDevice->setValue(QString("Port4Baud"), "115200");
        gpPredefinedDevice->setValue(QString("Port4Parity"), "0");
        gpPredefinedDevice->setValue(QString("Port4Stop"), "1");
        gpPredefinedDevice->setValue(QString("Port4Data"), "8");
        gpPredefinedDevice->setValue(QString("Port4Flow"), "1");

        //RM186/RM191
        gpPredefinedDevice->setValue(QString("Port5Name"), "RM186/RM191");
        gpPredefinedDevice->setValue(QString("Port5Baud"), "115200");
        gpPredefinedDevice->setValue(QString("Port5Parity"), "0");
        gpPredefinedDevice->setValue(QString("Port5Stop"), "1");
        gpPredefinedDevice->setValue(QString("Port5Data"), "8");
        gpPredefinedDevice->setValue(QString("Port5Flow"), "1");

        //Mark as completed
        gpPredefinedDevice->setValue(QString("DoneSetup"), "1");
    }

    //Add predefined devices
    unsigned char i = 1;
    while (i < 255)
    {
        if (gpPredefinedDevice->value(QString("Port").append(QString::number(i)).append("Name")).isNull())
        {
            break;
        }
        ui->combo_PredefinedDevice->addItem(gpPredefinedDevice->value(QString("Port").append(QString::number(i)).append("Name")).toString());
        ++i;
    }

    //Load settings from first device
    if (ui->combo_PredefinedDevice->count() > 0)
    {
        on_combo_PredefinedDevice_currentIndexChanged(ui->combo_PredefinedDevice->currentIndex());
    }

    //Add tooltips
    ui->check_SkipDL->setToolTip("Enable this to skip displaying the commands sent/received to the module when downloading a file to it.");
    ui->check_ShowCLRF->setToolTip("Enable this to escape various characters (CR will show as \\r, LF will show as \\n and Tab will show as \\t).");
    ui->check_EnableSSL->setToolTip("Enable this to use HTTPS (SSL) when communicating with UwTerminalX server (when updating or compiling applications), otherwise uses plaintext HTTP.");
    ui->check_ShowFileSize->setToolTip("Enable this to see the filesize of compiled applications.");
    ui->check_ConfirmClear->setToolTip("Enable this in order to confirm clearing the filesystem or resetting the module configuration to factory settings.");
    ui->check_LineSeparator->setToolTip("When enabled, pressing shit+enter in the terminal will create a line separator allowing for multiple lines to be sent when enter is pressed. If disabled then shift+enter will behave the same as pressing enter and send the line.");

    //Give focus to accept button
    if (ui->btn_Accept->isEnabled() == true)
    {
        ui->btn_Accept->setFocus();
    }

    //Enable system tray if it's available and enabled
    if (gpTermSettings->value("SysTrayIcon", DefaultSysTrayIcon).toBool() == true && QSystemTrayIcon::isSystemTrayAvailable())
    {
        //System tray enabled and available on system, set it up with contect menu/icon and show it
        gpSysTray = new QSystemTrayIcon;
        gpSysTray->setContextMenu(gpBalloonMenu);
        gpSysTray->setIcon(QIcon(*gpUw16Pixmap));
        gpSysTray->show();
        gbSysTrayEnabled = true;
    }

    //Update pre/post XCompile executable options
    ui->check_PreXCompRun->setChecked(gpTermSettings->value("PrePostXCompRun", DefaultPrePostXCompRun).toBool());
    ui->check_PreXCompFail->setChecked(gpTermSettings->value("PrePostXCompFail", DefaultPrePostXCompFail).toBool());
    if (gpTermSettings->value("PrePostXCompMode", DefaultPrePostXCompMode).toInt() == 1)
    {
        //Post-XCompiler run
        ui->radio_XCompPost->setChecked(true);
    }
    else
    {
        //Pre-XCompiler run
        ui->radio_XCompPre->setChecked(true);
    }
    ui->edit_PreXCompFilename->setText(gpTermSettings->value("PrePostXCompPath", DefaultPrePostXCompPath).toString());

    //Load skip download display setting
    ui->check_SkipDL->setChecked(gpTermSettings->value("SkipDownloadDisplay", DefaultSkipDownloadDisplay).toBool());

    //Load line separator setting
    ui->check_LineSeparator->setChecked(gpTermSettings->value("ShiftEnterLineSeparator", DefaultShiftEnterLineSeparator).toBool());

    //Notify scroll edit area of line separator value
    ui->text_TermEditData->mbLineSeparator = ui->check_LineSeparator->isChecked();

    //Update GUI for pre/post XComp executable
    on_check_PreXCompRun_stateChanged(ui->check_PreXCompRun->isChecked()*2);

    //Set Online XCompilation mode
#ifdef _WIN32
    //Windows
    ui->label_OnlineXCompInfo->setText("By enabling Online XCompilation support, if a local XCompiler is not found the source code will be uploaded and compiled remotely on a Laird cloud server. Uploaded data is not stored by Laird.");
#else
    //Mac or Linux
    ui->label_OnlineXCompInfo->setText("By enabling Online XCompilation support, when compiling an application, the source data will be uploadeda and compiled remotely on a Laird cloud server. Uploaded data is not stored by Laird.");
#endif
    ui->check_OnlineXComp->setChecked(gpTermSettings->value("OnlineXComp", DefaultOnlineXComp).toBool());

    //Load last directory path
    gstrLastFilename[FilenameIndexApplication] = gpTermSettings->value("LastFileDirectory", "").toString();
    gstrLastFilename[FilenameIndexOthers] = gpTermSettings->value("LastOtherFileDirectory", "").toString();

    //Refresh list of log files
    on_btn_LogRefresh_clicked();

    //Change terminal font to a monospaced font
#pragma warning("TODO: Revert manual font selection when QTBUG-54623 is fixed")
#ifdef _WIN32
    QFont fntTmpFnt2 = QFontDatabase::systemFont(QFontDatabase::FixedFont);
#elif __APPLE__
    QFont fntTmpFnt2 = QFontDatabase::systemFont(QFontDatabase::FixedFont);
#else
    //Fix for qt bug
    QFont fntTmpFnt2 = QFont("monospace");
#endif
    QFontMetrics tmTmpFM(fntTmpFnt2);
    ui->text_TermEditData->setFont(fntTmpFnt2);
    ui->text_TermEditData->setTabStopWidth(tmTmpFM.width(" ")*6);
    ui->text_LogData->setFont(fntTmpFnt2);
    ui->text_LogData->setTabStopWidth(tmTmpFM.width(" ")*6);
    ui->text_SpeedEditData->setFont(fntTmpFnt2);
    ui->text_SpeedEditData->setTabStopWidth(tmTmpFM.width(" ")*6);

    //Set resolved hostname to be empty
    gstrResolvedServer = "";

#ifdef UseSSL
    //Load SSL certificate
    QFile certFile(":/certificates/UwTerminalX_new.crt");
    if (certFile.open(QIODevice::ReadOnly))
    {
        //Load certificate data
        sslcLairdSSLNew = new QSslCertificate(certFile.readAll());
        QSslSocket::addDefaultCaCertificate(*sslcLairdSSLNew);
        certFile.close();
    }
#endif

    //Check command line
    QStringList slArgs = QCoreApplication::arguments();
    unsigned char chi = 1;
    bool bArgCom = false;
    bool bArgAccept = false;
    bool bArgNoConnect = false;
    bool bStartScript = false;
    while (chi < slArgs.length())
    {
        if (slArgs[chi].toUpper() == "ACCEPT")
        {
            //Skip the front panel - disable buttons
            ui->btn_Accept->setEnabled(false);
            ui->btn_Decline->setEnabled(false);

            //Switch to config tab
            ui->selector_Tab->setCurrentIndex(TabConfig);

            //Default empty images
            ui->image_CTS->setPixmap(*gpEmptyCirclePixmap);
            ui->image_DCD->setPixmap(*gpEmptyCirclePixmap);
            ui->image_DSR->setPixmap(*gpEmptyCirclePixmap);
            ui->image_RI->setPixmap(*gpEmptyCirclePixmap);

            ui->image_CTSb->setPixmap(*gpEmptyCirclePixmap);
            ui->image_DCDb->setPixmap(*gpEmptyCirclePixmap);
            ui->image_DSRb->setPixmap(*gpEmptyCirclePixmap);
            ui->image_RIb->setPixmap(*gpEmptyCirclePixmap);

            bArgAccept = true;
        }
        else if (slArgs[chi].left(4).toUpper() == "COM=")
        {
            //Set com port
            QString strPort = slArgs[chi].right(slArgs[chi].length()-4);
#ifdef _WIN32
            if (strPort.left(3) != "COM")
            {
                //Prepend COM for UwTerminal shortcut compatibility
                strPort.prepend("COM");
            }
#endif
            ui->combo_COM->setCurrentText(strPort);
            bArgCom = true;

            //Update serial port info
            on_combo_COM_currentIndexChanged(0);
        }
        else if (slArgs[chi].left(5).toUpper() == "BAUD=")
        {
            //Set baud rate
            ui->combo_Baud->setCurrentText(slArgs[chi].right(slArgs[chi].length()-5));
        }
        else if (slArgs[chi].left(5).toUpper() == "STOP=")
        {
            //Set stop bits
            if (slArgs[chi].right(1) == "1")
            {
                //One
                ui->combo_Stop->setCurrentIndex(0);
            }
            else if (slArgs[chi].right(1) == "2")
            {
                //Two
                ui->combo_Stop->setCurrentIndex(1);
            }
        }
        else if (slArgs[chi].left(5).toUpper() == "DATA=")
        {
            //Set data bits
            if (slArgs[chi].right(1) == "7")
            {
                //Seven
                ui->combo_Data->setCurrentIndex(0);
            }
            else if (slArgs[chi].right(1).toUpper() == "8")
            {
                //Eight
                ui->combo_Data->setCurrentIndex(1);
            }
        }
        else if (slArgs[chi].left(4).toUpper() == "PAR=")
        {
            //Set parity
            if (slArgs[chi].right(1).toInt() >= 0 && slArgs[chi].right(1).toInt() < 3)
            {
                ui->combo_Parity->setCurrentIndex(slArgs[chi].right(1).toInt());
            }
        }
        else if (slArgs[chi].left(5).toUpper() == "FLOW=")
        {
            //Set flow control
            if (slArgs[chi].right(1).toInt() >= 0 && slArgs[chi].right(1).toInt() < 3)
            {
                //Valid
                ui->combo_Handshake->setCurrentIndex(slArgs[chi].right(1).toInt());
            }
        }
        else if (slArgs[chi].left(7).toUpper() == "ENDCHR=")
        {
            //Sets the end of line character
            if (slArgs[chi].right(1) == "0")
            {
                //CR
                ui->radio_LCR->setChecked(true);
            }
            else if (slArgs[chi].right(1) == "1")
            {
                //LF
                ui->radio_LLF->setChecked(true);
            }
            else if (slArgs[chi].right(1) == "2")
            {
                //CRLF
                ui->radio_LCRLF->setChecked(true);
            }
            else if (slArgs[chi].right(1) == "3")
            {
                //LFCR
                ui->radio_LLFCR->setChecked(true);
            }
        }
        else if (slArgs[chi].left(10).toUpper() == "LOCALECHO=")
        {
            //Enable or disable local echo
            if (slArgs[chi].right(1) == "0")
            {
                //Off
                ui->check_Echo->setChecked(false);
            }
            else if (slArgs[chi].right(1) == "1")
            {
                //On (default)
                ui->check_Echo->setChecked(true);
            }
        }
        else if (slArgs[chi].left(9).toUpper() == "LINEMODE=")
        {
            //Enable or disable line mode
            if (slArgs[chi].right(1) == "0")
            {
                //Off
                ui->check_Line->setChecked(false);
                on_check_Line_stateChanged();
            }
            else if (slArgs[chi].right(1) == "1")
            {
                //On (default)
                ui->check_Line->setChecked(true);
                on_check_Line_stateChanged();
            }
        }
        else if (slArgs[chi].toUpper() == "LOG")
        {
            //Enables logging
            ui->check_LogEnable->setChecked(true);
        }
        else if (slArgs[chi].toUpper() == "LOG+")
        {
            //Enables appending to the previous log file instead of erasing
            ui->check_LogAppend->setChecked(true);
        }
        else if (slArgs[chi].left(4).toUpper() == "LOG=")
        {
            //Specifies log filename
            ui->edit_LogFile->setText(slArgs[chi].mid(4, -1));
        }
        else if (slArgs[chi].toUpper() == "SHOWCRLF")
        {
            //Displays \t, \r, \n etc. as \t, \r, \n instead of [tab], [new line], [carriage return]
            ui->check_ShowCLRF->setChecked(true);
        }
        else if (slArgs[chi].toUpper() == "NOCONNECT")
        {
            //Connect to device at startup
            bArgNoConnect = true;
        }
#ifndef SKIPAUTOMATIONFORM
        else if (slArgs[chi].toUpper() == "AUTOMATION" && bArgAccept == true)
        {
            //Show automation window
            if (guaAutomationForm == 0)
            {
                //Initialise automation popup
                guaAutomationForm = new UwxAutomation(this);

                //Populate window handles for automation object
                guaAutomationForm->SetPopupHandle(gpmErrorForm);

                //Update automation form with connection status
                guaAutomationForm->ConnectionChange(gspSerialPort.isOpen());

                //Connect signals
                connect(guaAutomationForm, SIGNAL(SendData(QString,bool,bool)), this, SLOT(MessagePass(QString,bool,bool)));

                //Give focus to the first line
                guaAutomationForm->SetFirstLineFocus();

                //Show form
                guaAutomationForm->show();
            }
        }
        else if (slArgs[chi].left(15).toUpper() == "AUTOMATIONFILE=")
        {
            //Load automation file
            if (guaAutomationForm == 0)
            {
                //Initialise automation popup
                guaAutomationForm = new UwxAutomation(this);

                //Populate window handles for automation object
                guaAutomationForm->SetPopupHandle(gpmErrorForm);

                //Update automation form with connection status
                guaAutomationForm->ConnectionChange(gspSerialPort.isOpen());

                //Connect signals
                connect(guaAutomationForm, SIGNAL(SendData(QString,bool,bool)), this, SLOT(MessagePass(QString,bool,bool)));
            }

            //Load file
            guaAutomationForm->LoadFile(slArgs[chi].right(slArgs[chi].size()-15));
        }
#endif
#ifndef SKIPSCRIPTINGFORM
        else if (slArgs[chi].toUpper() == "SCRIPTING" && bArgAccept == true)
        {
            //Show scripting window
            if (gusScriptingForm == 0)
            {
                //Initialise scripting form
                gusScriptingForm = new UwxScripting(this);

                //Populate window handles for automation object
                gusScriptingForm->SetPopupHandle(gpmErrorForm);

                //Send the UwTerminalX version string
                gusScriptingForm->SetUwTerminalXVersion(UwVersion);

                //Connect the message passing signal and slot
                connect(gusScriptingForm, SIGNAL(SendData(QString,bool,bool)), this, SLOT(MessagePass(QString,bool,bool)));
                connect(gusScriptingForm, SIGNAL(ScriptStartRequest()), this, SLOT(ScriptStartRequest()));
                connect(gusScriptingForm, SIGNAL(ScriptFinished()), this, SLOT(ScriptFinished()));

                if (gspSerialPort.isOpen())
                {
                    //Tell the form that the serial port is open
                    gusScriptingForm->SerialPortStatus(true);
                }

                //Show form
                gusScriptingForm->show();
                gusScriptingForm->SetEditorFocus();
            }
        }
        else if (slArgs[chi].left(11).toUpper() == "SCRIPTFILE=")
        {
            //Load a script file
            if (gusScriptingForm != 0)
            {
                QString strFilename = slArgs[chi].right(slArgs[chi].size()-11);
                gusScriptingForm->LoadScriptFile(&strFilename);
            }
        }
        else if (slArgs[chi].left(13).toUpper() == "SCRIPTACTION=")
        {
            //Action for scripting form
            if (slArgs[chi].right(slArgs[chi].size()-13) == "1" && gusScriptingForm != 0)
            {
                //Enable script execution
                bStartScript = true;
            }
        }
#endif
        ++chi;
    }

    if (bArgAccept == true && bArgCom == true && bArgNoConnect == false)
    {
        //Enough information to connect!
        OpenDevice();
        if (bStartScript == true)
        {
            //Start script execution request
            ScriptStartRequest();
        }
    }

#if __APPLE__
    //Show a warning to Mac users with the FTDI driver installed
    if ((QFile::exists("/System/Library/Extensions/FTDIUSBSerialDriver.kext") || QFile::exists("/Library/Extensions/FTDIUSBSerialDriver.kext")) && gpTermSettings->value("MacFTDIDriverWarningShown").isNull())
    {
        //FTDI driver detected and warning has not been shown, show warning
        gpTermSettings->setValue("MacFTDIDriverWarningShown", 1);
        QString strMessage = tr("Warning: The Mac FTDI VCP driver has been detected on your system. There is a known issue with this driver that can cause your system to crash if the serial port is closed and the buffer is not empty.\r\n\r\nIf you experience this issue, it is recommended that you remove the FTDI driver and use the apple VCP driver instead. Instructions to do this are available from the FTDI website (follow the uninstall section): http://www.ftdichip.com/Support/Documents/AppNotes/AN_134_FTDI_Drivers_Installation_Guide_for_MAC_OSX.pdf\r\n\r\nThis message will not be shown again.");
        gpmErrorForm->show();
        gpmErrorForm->SetMessage(&strMessage);
    }
#endif
}

//=============================================================================
//=============================================================================
MainWindow::~MainWindow(){
    //Disconnect all signals
#ifdef _WIN32
    disconnect(this, SLOT(process_finished(int, QProcess::ExitStatus)));
#endif
    disconnect(this, SLOT(close()));
    disconnect(this, SLOT(EnterPressed()));
    disconnect(this, SLOT(KeyPressed(QChar)));
    disconnect(this, SLOT(DroppedFile(QString)));
    disconnect(this, SLOT(MenuSelected(QAction*)));
    disconnect(this, SLOT(balloontriggered(QAction*)));
    disconnect(this, SLOT(ContextMenuClosed()));
    disconnect(this, SLOT(DevRespTimeout()));
    disconnect(this, SLOT(SerialStatusSlot()));
    disconnect(this, SLOT(SerialRead()));
    disconnect(this, SLOT(SerialError(QSerialPort::SerialPortError)));
    disconnect(this, SLOT(SerialBytesWritten(qint64)));
    disconnect(this, SLOT(UpdateReceiveText()));
    disconnect(this, SLOT(UpdateDisplayText()));
    disconnect(this, SLOT(SerialPortClosing()));
    disconnect(this, SLOT(BatchTimeoutSlot()));
    disconnect(this, SLOT(replyFinished(QNetworkReply*)));
    disconnect(this, SLOT(DetectBaudTimeout()));
    disconnect(this, SLOT(MessagePass(QString,bool,bool)));
    disconnect(this, SLOT(UpdateSpeedTestValues()));
    disconnect(this, SLOT(OutputSpeedTestStats()));

    if (gtmrSpeedTestDelayTimer != 0)
    {
        if (gtmrSpeedTestDelayTimer->isActive())
        {
            //Stop timer
            gtmrSpeedTestDelayTimer->stop();
        }

        //Disconnect slots and delete timer
        disconnect(this, SLOT(SpeedTestStartTimer()));
        disconnect(this, SLOT(SpeedTestStopTimer()));
        delete gtmrSpeedTestDelayTimer;
    }

    if (gspSerialPort.isOpen() == true)
    {
        //Close serial connection before quitting
        gspSerialPort.close();
        gpSignalTimer->stop();
    }

    if (gbMainLogEnabled == true)
    {
        //Close main log file before quitting
        gpMainLog->CloseLogFile();
    }

    //Close popups if open
    if (gpmErrorForm->isVisible())
    {
        //Close warning message
        gpmErrorForm->close();
    }
#ifndef SKIPAUTOMATIONFORM
    if (guaAutomationForm != 0)
    {
        if (guaAutomationForm->isVisible())
        {
            //Close automation form
            guaAutomationForm->close();
        }
        delete guaAutomationForm;
    }
#endif
#ifndef SKIPSCRIPTINGFORM
    if (gusScriptingForm != 0)
    {
        if (gusScriptingForm->isVisible())
        {
            //Close scripting form
            gusScriptingForm->close();
        }
        disconnect(this, SLOT(ScriptStartRequest()));
        disconnect(this, SLOT(ScriptFinished()));
        delete gusScriptingForm;
    }
#endif
#ifndef SKIPERRORCODEFORM
    if (gecErrorCodeForm != 0)
    {
        if (gecErrorCodeForm->isVisible())
        {
            //Close error code form
            gecErrorCodeForm->close();
        }
        delete gecErrorCodeForm;
    }
#endif

    //Delete system tray object
    if (gbSysTrayEnabled == true)
    {
        gpSysTray->hide();
        delete gpSysTray;
    }

    //Clear up streaming data if opened
    if (gbTermBusy == true && gbStreamingFile == true)
    {
        gpStreamFileHandle->close();
        delete gpStreamFileHandle;
    }
    else if (gbTermBusy == true && gbStreamingBatch == true)
    {
        //Clear up batch
        gpStreamFileHandle->close();
        delete gpStreamFileHandle;
    }

#ifdef UseSSL
    if (sslcLairdSSLNew != NULL)
    {
        //Clear up (newer) SSL certificate
        delete sslcLairdSSLNew;
    }
#endif

    if (gnmManager != 0)
    {
        //Clear up network manager
        delete gnmManager;
    }

    if (lstFileData.length() > 0)
    {
        //Clear the file data list
        ClearFileDataList();
    }

    //Delete variables
    delete gpMainLog;
    delete gpPredefinedDevice;
    delete gpTermSettings;
    delete gpErrorMessages;
    delete gpSignalTimer;
    delete gpSpeedMenu;
    delete gpBalloonMenu;
    delete gpSMenu3;
    delete gpSMenu2;
    delete gpSMenu1;
    delete gpMenu;
    delete gpEmptyCirclePixmap;
    delete gpRedCirclePixmap;
    delete gpGreenCirclePixmap;
    delete gpUw16Pixmap;
    delete gpUw32Pixmap;
    delete gpmErrorForm;
    delete ui;
}

//=============================================================================
//=============================================================================
void
MainWindow::closeEvent(
    QCloseEvent *
    )
{
    //Runs when the form is closed. Close child popups to exit the application
    if (gpmErrorForm->isVisible())
    {
        //Close warning message form
        gpmErrorForm->close();
    }
#ifndef SKIPAUTOMATIONFORM
    if (guaAutomationForm != 0 && guaAutomationForm->isVisible())
    {
        //Close automation form
        guaAutomationForm->close();
    }
#endif
#ifndef SKIPSCRIPTINGFORM
    if (gusScriptingForm != 0 && gusScriptingForm->isVisible())
    {
        //Close scripting form
        gusScriptingForm->close();
    }
#endif
#ifndef SKIPERRORCODEFORM
    if (gecErrorCodeForm != 0 && gecErrorCodeForm->isVisible())
    {
        //Close error code form
        gecErrorCodeForm->close();
    }
#endif

    //Close application
    QApplication::quit();
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Accept_clicked(
    )
{
    //Runs when accept button is clicked - disable buttons
    ui->btn_Accept->setEnabled(false);
    ui->btn_Decline->setEnabled(false);

    //Switch to config tab
    ui->selector_Tab->setCurrentIndex(TabConfig);

    //Default empty images
    ui->image_CTS->setPixmap(*gpEmptyCirclePixmap);
    ui->image_DCD->setPixmap(*gpEmptyCirclePixmap);
    ui->image_DSR->setPixmap(*gpEmptyCirclePixmap);
    ui->image_RI->setPixmap(*gpEmptyCirclePixmap);

    ui->image_CTSb->setPixmap(*gpEmptyCirclePixmap);
    ui->image_DCDb->setPixmap(*gpEmptyCirclePixmap);
    ui->image_DSRb->setPixmap(*gpEmptyCirclePixmap);
    ui->image_RIb->setPixmap(*gpEmptyCirclePixmap);
}

//=============================================================================
//=============================================================================
void
MainWindow::on_selector_Tab_currentChanged(
    int intIndex
    )
{
    if (ui->btn_Accept->isEnabled() == true && intIndex != TabAbout)
    {
        //Not accepted the terms yet
        ui->selector_Tab->setCurrentIndex(TabAbout);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Connect_clicked(
    )
{
    //Connect to COM port button clicked.
    OpenDevice();
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_TermClose_clicked(
    )
{
    if (ui->btn_TermClose->text() == "&Open Port")
    {
        //Open connection
        OpenDevice();
    }
    else if (ui->btn_TermClose->text() == "C&lose Port")
    {
        //Close, but first clear up from download/streaming
        gbTermBusy = false;
        gchTermMode = 0;
        gchTermMode2 = 0;
        ui->btn_Cancel->setEnabled(false);
        if (gbStreamingFile == true)
        {
            //Clear up file stream
            gtmrStreamTimer.invalidate();
            gbStreamingFile = false;
            gpStreamFileHandle->close();
            delete gpStreamFileHandle;
        }
        else if (gbStreamingBatch == true)
        {
            //Clear up batch
            gtmrStreamTimer.invalidate();
            gtmrBatchTimeoutTimer.stop();
            gbStreamingBatch = false;
            gpStreamFileHandle->close();
            delete gpStreamFileHandle;
            gbaBatchReceive.clear();
        }
        else if (gbSpeedTestRunning == true)
        {
            //Clear up speed testing
            if (gtmrSpeedTestDelayTimer != 0)
            {
                //Clean up timer
                disconnect(gtmrSpeedTestDelayTimer, SIGNAL(timeout()), this, SLOT(SpeedTestStartTimer()));
                disconnect(gtmrSpeedTestDelayTimer, SIGNAL(timeout()), this, SLOT(SpeedTestStopTimer()));
                delete gtmrSpeedTestDelayTimer;
                gtmrSpeedTestDelayTimer = 0;
            }

            ui->btn_SpeedStop->setEnabled(false);
            ui->btn_SpeedStart->setEnabled(false);
            ui->combo_SpeedDataType->setEnabled(true);
            if (ui->combo_SpeedDataType->currentIndex() == 1)
            {
                //Enable string options
                ui->edit_SpeedTestData->setEnabled(true);
                ui->check_SpeedStringUnescape->setEnabled(true);
            }

            //Update values
            OutputSpeedTestAvgStats((gtmrSpeedTimer.nsecsElapsed() < 1000000000 ? 1000000000 : gtmrSpeedTimer.nsecsElapsed()/1000000000));

            //Set speed test as no longer running
            gchSpeedTestMode = SpeedModeInactive;
            gbSpeedTestRunning = false;

            if (gtmrSpeedTimer.isValid())
            {
                //Invalidate speed test timer
                gtmrSpeedTimer.invalidate();
            }
            if (gtmrSpeedTestStats.isActive())
            {
                //Stop stats update timer
                gtmrSpeedTestStats.stop();
            }
            if (gtmrSpeedTestStats10s.isActive())
            {
                //Stop 10 second stats update timer
                gtmrSpeedTestStats10s.stop();
            }

            //Clear buffers
            gbaSpeedMatchData.clear();
            gbaSpeedReceivedData.clear();

            //Show finished message in status bar
            ui->statusBar->showMessage("Speed testing failed due to serial port being closed.");
        }

        //Close the serial port
        while (gspSerialPort.isOpen() == true)
        {
            gspSerialPort.clear();
            gspSerialPort.close();
        }
        gpSignalTimer->stop();

        //Re-enable inputs
        ui->edit_FWRH->setEnabled(true);

        //Disable active checkboxes
        ui->check_Break->setEnabled(false);
        ui->check_DTR->setEnabled(false);
        ui->check_SpeedDTR->setEnabled(false);
        ui->check_Echo->setEnabled(false);
        ui->check_Line->setEnabled(false);
        ui->check_RTS->setEnabled(false);
        ui->check_SpeedRTS->setEnabled(false);

        //Disable text entry
//        ui->text_TermEditData->setReadOnly(true);

        //Change status message
        ui->statusBar->showMessage("");

        //Change button text
        ui->btn_TermClose->setText("&Open Port");
        ui->btn_SpeedClose->setText("&Open Port");

#ifndef SKIPAUTOMATIONFORM
        //Notify automation form
        if (guaAutomationForm != 0)
        {
            guaAutomationForm->ConnectionChange(false);
        }
#endif

        //Disallow file drops
        setAcceptDrops(false);

        //Close log file if open
        if (gpMainLog->IsLogOpen() == true)
        {
            gpMainLog->CloseLogFile();
        }

        //Enable log options
        ui->edit_LogFile->setEnabled(true);
        ui->check_LogEnable->setEnabled(true);
        ui->check_LogAppend->setEnabled(true);
        ui->btn_LogFileSelect->setEnabled(true);

        //Disable speed testing
        ui->btn_SpeedStart->setEnabled(false);
    }

    //Update images
    UpdateImages();
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Refresh_clicked(
    )
{
    //Refresh the list of serial ports
    RefreshSerialDevices();
}

//=============================================================================
//=============================================================================
void
MainWindow::RefreshSerialDevices(
    )
{
    //Clears and refreshes the list of serial devices
    QString strPrev = "";
    QRegularExpression reTempRE("^(\\D*?)(\\d+)$");
    QList<int> lstEntries;
    lstEntries.clear();

    if (ui->combo_COM->count() > 0)
    {
        //Remember previous option
        strPrev = ui->combo_COM->currentText();
    }
    ui->combo_COM->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QRegularExpressionMatch remTempREM = reTempRE.match(info.portName());
        if (remTempREM.hasMatch() == true)
        {
            //Can sort this item
            int i = lstEntries.count()-1;
            while (i >= 0)
            {
                if (remTempREM.captured(2).toInt() > lstEntries[i])
                {
                    //Found correct order position, add here
                    ui->combo_COM->insertItem(i+1, info.portName());
                    lstEntries.insert(i+1, remTempREM.captured(2).toInt());
                    i = -1;
                }
                --i;
            }

            if (i == -1)
            {
                //Position not found, add to beginning
                ui->combo_COM->insertItem(0, info.portName());
                lstEntries.insert(0, remTempREM.captured(2).toInt());
            }
        }
        else
        {
            //Cannot sort this item
            ui->combo_COM->insertItem(ui->combo_COM->count(), info.portName());
        }
    }

    //Search for previous item if one was selected
    if (strPrev == "")
    {
        //Select first item
        ui->combo_COM->setCurrentIndex(0);
    }
    else
    {
        //Search for previous
        int i = 0;
        while (i < ui->combo_COM->count())
        {
            if (ui->combo_COM->itemText(i) == strPrev)
            {
                //Found previous item
                ui->combo_COM->setCurrentIndex(i);
                break;
            }
            ++i;
        }
    }

    //Update serial port info
    on_combo_COM_currentIndexChanged(0);
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_TermClear_clicked(
    )
{
    //Clears the screen of the terminal tab
    ui->text_TermEditData->ClearDatIn();
}

//=============================================================================
//=============================================================================
void
MainWindow::SerialRead(
    )
{
    //Read the data into a buffer and copy it to edit for the display data
    if (gbSpeedTestRunning == true)
    {
        //Serial test is running, pass to speed test function
        SpeedTestReceive();
    }
    else
    {
        //Speed test is not running
        QByteArray baOrigData = gspSerialPort.readAll();
#ifndef SKIPSCRIPTINGFORM
        if (gusScriptingForm != 0 && gbScriptingRunning == true)
        {
            gusScriptingForm->SerialPortData(&baOrigData);
        }
#endif

        if (ui->check_SkipDL->isChecked() == false || (gbTermBusy == false || (gbTermBusy == true && baOrigData.length() > 6) || (gbTermBusy == true && (gchTermMode == MODE_CHECK_ERROR_CODE_VERSIONS || gchTermMode == MODE_CHECK_UWTERMINALX_VERSIONS || gchTermMode == MODE_UPDATE_ERROR_CODE || gchTermMode == MODE_CHECK_FIRMWARE_VERSIONS || gchTermMode == 50))))
        {
            //Update the display with the data
            QByteArray baDispData = baOrigData;

            //Add to log
            gpMainLog->WriteRawLogData(baOrigData);

            if (ui->check_ShowCLRF->isChecked() == true)
            {
                //Escape \t, \r and \n
                baDispData.replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n");
            }

            //Replace unprintable characters
            baDispData.replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f");

            //Update display buffer
            gbaDisplayBuffer.append(baDispData);
            if (!gtmrTextUpdateTimer.isActive())
            {
                gtmrTextUpdateTimer.start();
            }

            if (gbLoopbackMode == true)
            {
                //Loopback enabled, send this data back
                gspSerialPort.write(baOrigData);
                gintQueuedTXBytes += baOrigData.length();
                gpMainLog->WriteRawLogData(baOrigData);
                gbaDisplayBuffer.append(baDispData);
            }
        }

        //Update number of recieved bytes
        gintRXBytes = gintRXBytes + baOrigData.length();
        ui->label_TermRx->setText(QString::number(gintRXBytes));

        if (gbAutoBaud == true)
        {
            //Append data to batch receive buffer to save memory instead of having another buffer
            gbaBatchReceive += baOrigData;
            if (gbaBatchReceive.indexOf("\n00\r") != -1 || (gbaBatchReceive.indexOf("\n10\t0\t") != -1 && gbaBatchReceive.indexOf("\r", gbaBatchReceive.indexOf("\n10\t0\t")) != -1))
            {
                //Baud rate found
                gbAutoBaud = false;
                gtmrBaudTimer.stop();
                gbTermBusy = false;
                gchTermMode = 0;
                ui->btn_Cancel->setEnabled(false);

                //Show success message to user
                QRegularExpression reTempRE("10\t0\t([a-zA-Z0-9]{3,20})\r");
                QRegularExpressionMatch remTempREM = reTempRE.match(gbaBatchReceive);
                QString strMessage = tr("Successfully detected ").append((remTempREM.hasMatch() == true ? QString(remTempREM.captured(1)).append(" ") : "")).append("module on port ").append(ui->combo_COM->currentText()).append(" at baud rate ").append(ui->combo_Baud->currentText()).append(".\r\n\r\nThe port has been left open for you to communicate with the module.\r\n\r\n(Please note that it is possible to change the default module baud rate with newer firmware versions using AT+CFG 520 <baud>. Please check the smartBASIC extension manual for your module to see if this is supported and how to configure it)");
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);

                //Clear buffer
                gbaBatchReceive.clear();
            }
        }

        //Send next chunk of batch data if enabled
        StreamBatchContinue(&baOrigData);

        if (gbTermBusy == true && gchTermMode2 == 0)
        {
            //Currently waiting for a response
            gstrTermBusyData = gstrTermBusyData.append(baOrigData);
            gchTermBusyLines = gchTermBusyLines + baOrigData.count("\n");
            if ((ui->check_CheckLicense->isChecked() && (gchTermMode == MODE_COMPILE_LOAD || gchTermMode == MODE_COMPILE_LOAD_RUN || gchTermMode == MODE_SERVER_COMPILE_LOAD || gchTermMode == MODE_SERVER_COMPILE_LOAD_RUN) && gchTermBusyLines == 6) || ((!ui->check_CheckLicense->isChecked() || !(gchTermMode == MODE_COMPILE_LOAD || gchTermMode == MODE_COMPILE_LOAD_RUN || gchTermMode == MODE_SERVER_COMPILE_LOAD || gchTermMode == MODE_SERVER_COMPILE_LOAD_RUN)) && gchTermBusyLines == 4))
            {
                //Enough data, check it.
                if (ui->check_CheckLicense->isChecked() && gchTermBusyLines == 6)
                {
                    //Check for license response
                    QRegularExpression reTempLicRE("\n10\t4\t00 ([a-zA-Z0-9]{12})\r\n00\r");
                    QRegularExpressionMatch remTempLicREM = reTempLicRE.match(gstrTermBusyData);
                    if (remTempLicREM.hasMatch() == true && remTempLicREM.captured(1).toUpper() == "0016A4C0FFEE")
                    {
                        //License is not valid, display a warning to the user
                        QString strMessage = tr("Please note: The module you are downloading to does not have a valid license and therefore some firmware functionality may not work or return unexpected error codes.\r\n\r\nTo fix this issue, please add your module license using: 'at+lic <license code>', or contact Laird Support with the module Bluetooth Address if you do not have a backup of the license code (issue the command 'at i 14' to get your module's Bluetooth address).");
                        gpmErrorForm->show();
                        gpmErrorForm->SetMessage(&strMessage);
                    }
                }

                QRegularExpression reTempRE("\n10	0	(.{2,32}?)\r\n00\r\n10	13	([a-zA-Z0-9]{4}) ([a-zA-Z0-9]{4}) \r\n00\r");
                QRegularExpressionMatch remTempREM = reTempRE.match(gstrTermBusyData);
                if (remTempREM.hasMatch() == true)
                {
                    //Extract device name
                    QString strDevName = AtiToXCompName(remTempREM.captured(1));
                    if (gchTermMode == MODE_SERVER_COMPILE || gchTermMode == MODE_SERVER_COMPILE_LOAD || gchTermMode == MODE_SERVER_COMPILE_LOAD_RUN)
                    {
                        if (ui->check_OnlineXComp->isChecked() == true)
                        {
                            //Check if online XCompiler supports this device
                            if (LookupDNSName() == true)
                            {
                                gnmrReply = gnmManager->get(QNetworkRequest(QUrl(QString(WebProtocol).append("://").append(gstrResolvedServer).append("/supported.php?JSON=1&Dev=").append(strDevName).append("&HashA=").append(remTempREM.captured(2)).append("&HashB=").append(remTempREM.captured(3)))));
                            }
                        }
                        else
                        {
                            //Online XCompiler not enabled
                            QString strMessage = tr("Unable to XCompile application: Online XCompilation support must be enabled to XCompile applications on Mac/Linux.\nPlease enable it from the 'Config' tab and try again.");
                            gpmErrorForm->show();
                            gpmErrorForm->SetMessage(&strMessage);
                        }
                    }
                    else
                    {
                        //Matched and split, now start the compilation!
                        gtmrDownloadTimeoutTimer.stop();

                        //Split the file path up
                        QList<QString> lstFI = SplitFilePath(gstrTermFilename);
#ifdef _WIN32
                        //Windows
                        if (QFile::exists(QString(gpTermSettings->value("CompilerDir", DefaultCompilerDir).toString()).append((gpTermSettings->value("CompilerSubDirs", DefaultCompilerSubDirs).toBool() == true ? QString(strDevName).append("/") : "")).append("XComp_").append(strDevName).append("_").append(remTempREM.captured(2)).append("_").append(remTempREM.captured(3)).append(".exe")) == true)
                        {
                            //XCompiler found! - First run the Pre XCompile program if enabled and it exists
                            if (ui->check_PreXCompRun->isChecked() == true && ui->radio_XCompPre->isChecked() == true)
                            {
                                //Run Pre-XComp program
                                RunPrePostExecutable(gstrTermFilename);
                            }
                            gprocCompileProcess.start(QString(gpTermSettings->value("CompilerDir", DefaultCompilerDir).toString()).append((gpTermSettings->value("CompilerSubDirs", DefaultCompilerSubDirs).toBool() == true ? QString(strDevName).append("/") : "")).append("XComp_").append(strDevName).append("_").append(remTempREM.captured(2)).append("_").append(remTempREM.captured(3)).append(".exe"), QStringList(gstrTermFilename));
                        }
                        else if (QFile::exists(QString(lstFI[0]).append("XComp_").append(strDevName).append("_").append(remTempREM.captured(2)).append("_").append(remTempREM.captured(3)).append(".exe")) == true)
                        {
                            //XCompiler found in directory with sB file
                            if (ui->check_PreXCompRun->isChecked() == true && ui->radio_XCompPre->isChecked() == true)
                            {
                                //Run Pre-XComp program
                                RunPrePostExecutable(gstrTermFilename);
                            }
                            gprocCompileProcess.start(QString(lstFI[0]).append("XComp_").append(strDevName).append("_").append(remTempREM.captured(2)).append("_").append(remTempREM.captured(3)).append(".exe"), QStringList(gstrTermFilename));
                        }
                        else
#endif
                        if (ui->check_OnlineXComp->isChecked() == true)
                        {
                            //XCompiler not found, try Online XCompiler
                            if (LookupDNSName() == true)
                            {
                                gnmrReply = gnmManager->get(QNetworkRequest(QUrl(QString(WebProtocol).append("://").append(gstrResolvedServer).append("/supported.php?JSON=1&Dev=").append(strDevName).append("&HashA=").append(remTempREM.captured(2)).append("&HashB=").append(remTempREM.captured(3)))));
                                ui->statusBar->showMessage("Device support request sent...", 2000);

                                if (gchTermMode == MODE_COMPILE)
                                {
                                    gchTermMode = MODE_SERVER_COMPILE;
                                }
                                else if (gchTermMode == MODE_COMPILE_LOAD)
                                {
                                    gchTermMode = MODE_SERVER_COMPILE_LOAD;
                                }
                                else if (gchTermMode == MODE_COMPILE_LOAD_RUN)
                                {
                                    gchTermMode = MODE_SERVER_COMPILE_LOAD_RUN;
                                }
                            }
                            else
                            {
                                //DNS resolution failed
                                gbTermBusy = false;
                                ui->btn_Cancel->setEnabled(false);
                            }
                        }
                        else
                        {
                            //XCompiler not found, Online XCompiler disabled
                            QString strMessage = tr("Error during XCompile:\nXCompiler \"XComp_").append(strDevName).append("_").append(remTempREM.captured(2)).append("_").append(remTempREM.captured(3))
#ifdef _WIN32
                            .append(".exe")
#endif
                            .append("\" was not found.\r\n\r\nPlease ensure you put XCompile binaries in the correct directory (").append(gpTermSettings->value("CompilerDir", DefaultCompilerDir).toString()).append((gpTermSettings->value("CompilerSubDirs", DefaultCompilerSubDirs).toBool() == true ? strDevName : "")).append(").\n\nYou can also enable Online XCompilation from the 'Config' tab to XCompile applications using Laird's online server.");
                            gpmErrorForm->show();
                            gpmErrorForm->SetMessage(&strMessage);
                            gbTermBusy = false;
                            ui->btn_Cancel->setEnabled(false);
                        }
                    }
                }
            }
        }
        else if (gbTermBusy == true && gchTermMode2 > 0 && gchTermMode2 < 20)
        {
            gstrTermBusyData = gstrTermBusyData.append(baOrigData);
            if (gstrTermBusyData.length() > 3)
            {
                if (gchTermMode2 == 1)
                {
                    QByteArray baTmpBA = QString("AT+FOW \"").append(gstrDownloadFilename).append("\"").toUtf8();
                    gspSerialPort.write(baTmpBA);
                    gbFileOpened = true;
                    gintQueuedTXBytes += baTmpBA.size();
                    DoLineEnd();
                    gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
                    if (ui->check_SkipDL->isChecked() == false)
                    {
                        //Output download details
                        if (ui->check_ShowCLRF->isChecked() == true)
                        {
                            //Escape \t, \r and \n
                            baTmpBA.replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n");
                        }

                        //Replace unprintable characters
                        baTmpBA.replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f");

                        //Update display buffer
                        gbaDisplayBuffer.append(baTmpBA);
                        if (!gtmrTextUpdateTimer.isActive())
                        {
                            gtmrTextUpdateTimer.start();
                        }
                    }
                    gstrTermBusyData = "";
                }
                else if (gchTermMode2 == 2)
                {
                    //Append data to buffer
                    gpMainLog->WriteLogData(gstrTermBusyData);
                    if (QString::fromUtf8(baOrigData).indexOf("\n01", 0, Qt::CaseSensitive) == -1 && QString::fromUtf8(baOrigData).indexOf("~FAULT", 0, Qt::CaseSensitive) == -1 && gstrTermBusyData.indexOf("00", 0, Qt::CaseSensitive) != -1)
                    {
                        //Success
                        gstrTermBusyData = "";
                        if (gstrHexData.length() > 0)
                        {
                            gtmrDownloadTimeoutTimer.stop();
                            QByteArray baTmpBA = QString("AT+FWRH \"").append(gstrHexData.left(ui->edit_FWRH->text().toInt())).append("\"").toUtf8();
                            gspSerialPort.write(baTmpBA);
                            gintQueuedTXBytes += baTmpBA.size();
                            DoLineEnd();
                            gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
                            if (ui->check_SkipDL->isChecked() == false)
                            {
                                //Output download details
                                if (ui->check_ShowCLRF->isChecked() == true)
                                {
                                    //Escape \t, \r and \n
                                    baTmpBA.replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n");
                                }

                                //Replace unprintable characters
                                baTmpBA.replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f");

                                //Update display buffer
                                gbaDisplayBuffer.append(baTmpBA);
                                if (!gtmrTextUpdateTimer.isActive())
                                {
                                    gtmrTextUpdateTimer.start();
                                }
                            }

                            if (gstrHexData.length() < ui->edit_FWRH->text().toInt())
                            {
                                //Finished sending data
                                gstrHexData.clear();
                            }
                            else
                            {
                                //More data to send
                                gstrHexData = gstrHexData.right(gstrHexData.length()-ui->edit_FWRH->text().toInt());
                            }
                            --gchTermMode2;
                            gtmrDownloadTimeoutTimer.start();

                            //Update amount of data left to send
                            ui->label_TermTxLeft->setText(QString::number(gstrHexData.length()));
                        }
                        else
                        {
                            gstrHexData = "";
                            gbaDisplayBuffer.append("\n-- Finished downloading file --\n");
                            gspSerialPort.write("AT+FCL");
                            gintQueuedTXBytes += 6;
                            DoLineEnd();
                            gpMainLog->WriteLogData("AT+FCL\n");
                            if (ui->check_SkipDL->isChecked() == false)
                            {
                                //Output download details
                                gbaDisplayBuffer.append("AT+FCL\n");
                            }
                            if (!gtmrTextUpdateTimer.isActive())
                            {
                                gtmrTextUpdateTimer.start();
                            }
                            QList<QString> lstFI = SplitFilePath(gstrTermFilename);
                            if (gpTermSettings->value("DelUWCAfterDownload", DefaultDelUWCAfterDownload).toBool() == true && gbIsUWCDownload == true && QFile::exists(QString(lstFI[0]).append(lstFI[1]).append(".uwc")))
                            {
                                //Remove UWC
                                QFile::remove(QString(lstFI[0]).append(lstFI[1]).append(".uwc"));
                            }
                        }
                    }
                    else
                    {
                        //Presume error
                        gbTermBusy = false;
                        gchTermBusyLines = 0;
                        gchTermMode = 0;
                        gchTermMode2 = 0;
                        QString strMessage = tr("Error whilst downloading data to device. If filesystem is full, please restart device with 'atz' and clear the filesystem using 'at&f 1'.\nPlease note this will erase ALL FILES on the device, configuration keys and all bonding keys.\n\nReceived: ").append(QString::fromUtf8(baOrigData));
                        gpmErrorForm->show();
                        gpmErrorForm->SetMessage(&strMessage);
                        QList<QString> lstFI = SplitFilePath(gstrTermFilename);
                        if (gpTermSettings->value("DelUWCAfterDownload", DefaultDelUWCAfterDownload).toBool() == true && gbIsUWCDownload == true && QFile::exists(QString(lstFI[0]).append(lstFI[1]).append(".uwc")))
                        {
                            //Remove UWC
                            QFile::remove(QString(lstFI[0]).append(lstFI[1]).append(".uwc"));
                        }
                        ui->btn_Cancel->setEnabled(false);
                    }
                }
                else if (gchTermMode2 == MODE_COMPILE_LOAD_RUN)
                {
                    if (gchTermMode == MODE_COMPILE_LOAD_RUN || gchTermMode == MODE_LOAD_RUN)
                    {
                        //Run!
                        RunApplication();
                    }
                }
                ++gchTermMode2;

                if (gchTermMode2 > gchTermMode)
                {
                    //Finished, no longer busy
                    gchTermMode = 0;
                    gchTermMode2 = 0;
                    gbTermBusy = false;
                    ui->btn_Cancel->setEnabled(false);
                }
                else if ((gchTermMode == MODE_LOAD || gchTermMode == 6) && gchTermMode2 == MODE_LOAD)
                {
                    gchTermMode = 0;
                    gchTermMode2 = 0;
                    gbTermBusy = false;
                    ui->btn_Cancel->setEnabled(false);
                }
            }
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::StreamBatchContinue(
    QByteArray *baOrigData
    )
{
    if (gbStreamingBatch == true)
    {
        //Batch stream in progress
        gbaBatchReceive += *baOrigData;
        if (gbaBatchReceive.indexOf("\n00\r") != -1)
        {
            //Success code, next statement
            if (gpStreamFileHandle->atEnd())
            {
                //Finished sending
                FinishBatch(false);
            }
            else
            {
                //Send more data
                QByteArray baFileData = gpStreamFileHandle->readLine().replace("\n", "").replace("\r", "");
                gspSerialPort.write(baFileData);
                gintQueuedTXBytes += baFileData.length();
                DoLineEnd();
                gpMainLog->WriteLogData(QString(baFileData).append("\n"));
//        gintStreamBytesRead += FileData.length();
                gtmrBatchTimeoutTimer.start(BatchTimeout);
                ++gintStreamBytesRead;

                //Update the display buffer
                gbaDisplayBuffer.append(baFileData);
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
            }
            gbaBatchReceive.clear();
        }
        else if (gbaBatchReceive.indexOf("\n01\t") != -1 && gbaBatchReceive.indexOf("\r", gbaBatchReceive.indexOf("\n01\t")+4) != -1)
        {
            //Failure code
            QRegularExpression reTempRE("\t([a-zA-Z0-9]{1,9})(\t|\r)");
            QRegularExpressionMatch remTempREM = reTempRE.match(gbaBatchReceive);
            if (remTempREM.hasMatch() == true)
            {
                //Got the error code
                gbaDisplayBuffer.append("\nError during batch command, error code: ").append(remTempREM.captured(1)).append("\n");

                //Lookup error code
                bool bTmpBool;
                unsigned int ErrCode = QString("0x").append(remTempREM.captured(1)).toUInt(&bTmpBool, 16);
                if (bTmpBool == true)
                {
                    //Converted
                    LookupErrorCode(ErrCode);
                }
            }
            else
            {
                //Unknown error code
                gbaDisplayBuffer.append("\nError during batch command, unknown error code.\n");
            }
            if (!gtmrTextUpdateTimer.isActive())
            {
                gtmrTextUpdateTimer.start();
            }

            //Show status message
            ui->statusBar->showMessage(QString("Failed sending batch file at line ").append(QString::number(gintStreamBytesRead)));

            //Clear up and cancel timer
            gtmrBatchTimeoutTimer.stop();
            gbTermBusy = false;
            gbStreamingBatch = false;
            gchTermMode = 0;
            gpStreamFileHandle->close();
            delete gpStreamFileHandle;
            gbaBatchReceive.clear();
            ui->btn_Cancel->setEnabled(false);
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_text_TermEditData_customContextMenuRequested(
    const QPoint &pos
    )
{
    //Creates the custom context menu
    gpMenu->popup(ui->text_TermEditData->viewport()->mapToGlobal(pos));
    ui->text_TermEditData->mbContextMenuOpen = true;
}

void
MainWindow::MenuSelected(
    QAction* qaAction
    )
{
    //Runs when a menu item is selected
    int intItem = qaAction->data().toInt();
    if (gspSerialPort.isOpen() == true && gbLoopbackMode == false && gbTermBusy == false)
    {
        //Serial is open, allow xcompile functions
        if (intItem == MenuActionXCompile)
        {
            //Compiles SB applet
#ifdef _WIN32
            CompileApp(MODE_COMPILE);
#else
            CompileApp(MODE_SERVER_COMPILE);
#endif
        }
        else if (intItem == MenuActionXCompileLoad)
        {
            //Compiles and loads a SB applet
#ifdef _WIN32
            CompileApp(MODE_COMPILE_LOAD);
#else
            CompileApp(MODE_SERVER_COMPILE_LOAD);
#endif
        }
        else if (intItem == MenuActionXCompileLoadRun)
        {
            //Compiles, loads and runs a SB applet
#ifdef _WIN32
            CompileApp(MODE_COMPILE_LOAD_RUN);
#else
            CompileApp(MODE_SERVER_COMPILE_LOAD_RUN);
#endif
        }
        else if (intItem == MenuActionLoad || intItem == MenuActionLoad2 || intItem == MenuActionDataFile)
        {
            //Just load an application
            CompileApp(MODE_LOAD);
        }
        else if (intItem == MenuActionLoadRun)
        {
            //Load and run an application
            CompileApp(MODE_LOAD_RUN);
        }
    }

    if (intItem == MenuActionErrorHex)
    {
        //Shows a meaning for the error code selected (number in hex)
        bool bTmpBool;
        unsigned int uiErrCode = QString("0x").append(ui->text_TermEditData->textCursor().selection().toPlainText()).toUInt(&bTmpBool, 16);
        if (bTmpBool == true)
        {
            //Converted
            LookupErrorCode(uiErrCode);
        }
    }
    else if (intItem == MenuActionErrorInt)
    {
        //Shows a meaning for the error code selected (number as int)
        LookupErrorCode(ui->text_TermEditData->textCursor().selection().toPlainText().toInt());
    }
    else if (intItem == MenuActionLoopback)
    {
        //Enable/disable loopback mode
        gbLoopbackMode = !gbLoopbackMode;
        if (gbLoopbackMode == true)
        {
            //Enabled
            gbaDisplayBuffer.append("\n[Loopback Enabled]\n");
            gpMenu->actions()[7]->setText("Disable Loopback (Rx->Tx)");
        }
        else
        {
            //Disabled
            gbaDisplayBuffer.append("\n[Loopback Disabled]\n");
            gpMenu->actions()[7]->setText("Enable Loopback (Rx->Tx)");
        }
        if (!gtmrTextUpdateTimer.isActive())
        {
            gtmrTextUpdateTimer.start();
        }
    }
    else if (intItem == MenuActionEraseFile || intItem == MenuActionEraseFile2)
    {
        //Erase file
        if (gspSerialPort.isOpen() == true && gbLoopbackMode == false && gbTermBusy == false)
        {
            //Not currently busy
#ifndef SKIPAUTOMATIONFORM
            if (guaAutomationForm != 0)
            {
                guaAutomationForm->TempAlwaysOnTop(0);
            }
#endif
            QString strFilename = QFileDialog::getOpenFileName(this, "Open File", gstrLastFilename[FilenameIndexApplication], "SmartBasic Applications (*.uwc);;All Files (*.*)");
#ifndef SKIPAUTOMATIONFORM
            if (guaAutomationForm != 0)
            {
                guaAutomationForm->TempAlwaysOnTop(1);
            }
#endif

            if (strFilename.length() > 1)
            {
                //Set last directory config
                gstrLastFilename[FilenameIndexApplication] = strFilename;
                gpTermSettings->setValue("LastFileDirectory", SplitFilePath(strFilename)[0]);

                //Delete file
                if (strFilename.lastIndexOf(".") >= 0)
                {
                    //Get up until the first dot
                    QRegularExpression reTempRE("^(.*)/(.*)$");
                    QRegularExpressionMatch remTempREM = reTempRE.match(strFilename);

                    if (remTempREM.hasMatch() == true)
                    {
                        strFilename = remTempREM.captured(2);
                        if (strFilename.count(".") > 0)
                        {
                            //Strip off after the dot
                            strFilename = strFilename.left(strFilename.indexOf("."));
                        }
                    }
                }
                QByteArray baTmpBA = QString("at+del \"").append(strFilename).append("\"").append((intItem == 14 ? " +" : "")).toUtf8();
                gspSerialPort.write(baTmpBA);
                gintQueuedTXBytes += baTmpBA.size();
                DoLineEnd();
                gbaDisplayBuffer.append(baTmpBA);
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
                gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
            }
        }
    }
    else if (intItem == MenuActionDir)
    {
        //Dir
        if (gspSerialPort.isOpen() == true && gbLoopbackMode == false && gbTermBusy == false)
        {
            //List current directory contents
            gspSerialPort.write("at+dir");
            gintQueuedTXBytes += 6;
            DoLineEnd();
            gbaDisplayBuffer.append("\nat+dir\n");
            if (!gtmrTextUpdateTimer.isActive())
            {
                gtmrTextUpdateTimer.start();
            }
            gpMainLog->WriteLogData("at+dir\n");
        }
    }
    else if (intItem == MenuActionRun || intItem == MenuActionDebug)
    {
        //Run/debug an application
        if (gspSerialPort.isOpen() == true && gbLoopbackMode == false && gbTermBusy == false)
        {
            //Not currently busy
#ifndef SKIPAUTOMATIONFORM
            if (guaAutomationForm != 0)
            {
                guaAutomationForm->TempAlwaysOnTop(0);
            }
#endif
            QString strFilename = QFileDialog::getOpenFileName(this, "Open File", gstrLastFilename[FilenameIndexApplication], "SmartBasic Applications (*.uwc);;All Files (*.*)");
#ifndef SKIPAUTOMATIONFORM
            if (guaAutomationForm != 0)
            {
                guaAutomationForm->TempAlwaysOnTop(1);
            }
#endif

            if (strFilename.length() > 1)
            {
                //Set last directory config
                gstrLastFilename[FilenameIndexApplication]  = strFilename;
                gpTermSettings->setValue("LastFileDirectory", SplitFilePath(strFilename)[0]);

                //Get the application name
                if (strFilename.lastIndexOf(".") >= 0)
                {
                    //Get up until the first dot
                    QRegularExpression reTempRE("^(.*)/(.*)$");
                    QRegularExpressionMatch remTempREM = reTempRE.match(strFilename);
                    if (remTempREM.hasMatch() == true)
                    {
                        //Got a match
                        strFilename = remTempREM.captured(2);
                        if (strFilename.count(".") > 0)
                        {
                            //Strip off after the dot
                            strFilename = strFilename.left(strFilename.indexOf("."));
                        }
                    }
                }
                QByteArray baTmpBA = QString((intItem == 13 ? "at+dbg \"" : "at+run \"")).append(strFilename).append("\"").toUtf8();
                gspSerialPort.write(baTmpBA);
                gintQueuedTXBytes += baTmpBA.size();
                DoLineEnd();
                gbaDisplayBuffer.append(baTmpBA);
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
                gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
            }
        }
    }
    else if (intItem == MenuActionClearFilesystem)
    {
        //Clear filesystem
        if (gspSerialPort.isOpen() == true && gbLoopbackMode == false && gbTermBusy == false)
        {
            if (!ui->check_ConfirmClear->isChecked() || QMessageBox::question(this, "Clear module filesystem and configuration keys?", "You are about to clear the filesystem of the module, including the values of configuration keys. This process is irreversible!\r\n\r\nClick yes to proceed ot no to cancel.", QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
            {
                //Send command
                QByteArray baTmpBA = QString("at&f 1").toUtf8();
                gspSerialPort.write(baTmpBA);
                gintQueuedTXBytes += baTmpBA.size();
                DoLineEnd();
                gbaDisplayBuffer.append(baTmpBA);
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
                gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
            }
        }
    }
    else if (intItem == MenuActionStreamFile)
    {
        //Stream out a file
        if (gspSerialPort.isOpen() == true && gbLoopbackMode == false && gbTermBusy == false)
        {
            //Not currently busy
#ifndef SKIPAUTOMATIONFORM
            if (guaAutomationForm != 0)
            {
                guaAutomationForm->TempAlwaysOnTop(0);
            }
#endif
            QString strFilename = QFileDialog::getOpenFileName(this, tr("Open File To Stream"), gstrLastFilename[FilenameIndexOthers], tr("Text Files (*.txt);;All Files (*.*)"));
#ifndef SKIPAUTOMATIONFORM
            if (guaAutomationForm != 0)
            {
                guaAutomationForm->TempAlwaysOnTop(1);
            }
#endif

            if (strFilename.length() > 1)
            {
                //Set last directory config
                gstrLastFilename[FilenameIndexOthers] = strFilename;
                gpTermSettings->setValue("LastOtherFileDirectory", SplitFilePath(strFilename)[0]);

                //File was selected - start streaming it out
                gpStreamFileHandle = new QFile(strFilename);

                if (!gpStreamFileHandle->open(QIODevice::ReadOnly))
                {
                    //Unable to open file
                    QString strMessage = tr("Error during file streaming: Access to selected file is denied: ").append(strFilename);
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                    return;
                }

                //We're now busy
                gbTermBusy = true;
                gbStreamingFile = true;
                gchTermMode = 50;
                ui->btn_Cancel->setEnabled(true);

                //Save the size of the file
                gintStreamBytesSize = gpStreamFileHandle->size();
                gintStreamBytesRead = 0;
                gintStreamBytesProgress = StreamProgress;

                //Start a timer
                gtmrStreamTimer.start();

                //Reads out each block
                QByteArray baFileData = gpStreamFileHandle->read(FileReadBlock);
                gspSerialPort.write(baFileData);
                gintQueuedTXBytes += baFileData.size();
                gintStreamBytesRead = baFileData.length();
                gpMainLog->WriteLogData(QString(baFileData).append("\n"));
                if (gpStreamFileHandle->atEnd())
                {
                    //Finished sending
                    FinishStream(false);
                }
            }
        }
    }
    else if (intItem == MenuActionFont)
    {
        //Change font
        bool bTmpBool;
        QFont fntTmpFnt = QFontDialog::getFont(&bTmpBool, ui->text_TermEditData->font(), this);
        if (bTmpBool == true)
        {
            //Set font and re-adjust tab spacing
            QFontMetrics tmTmpFM(fntTmpFnt);
            ui->text_TermEditData->setFont(fntTmpFnt);
            ui->text_TermEditData->setTabStopWidth(tmTmpFM.width(" ")*6);
            ui->text_SpeedEditData->setFont(fntTmpFnt);
            ui->text_SpeedEditData->setTabStopWidth(tmTmpFM.width(" ")*6);
        }
    }
    else if (intItem == MenuActionRun2)
    {
        //Runs an application
        if (gspSerialPort.isOpen() == true && gbLoopbackMode == false && gbTermBusy == false)
        {
            //Not currently busy
#ifndef SKIPAUTOMATIONFORM
            if (guaAutomationForm != 0)
            {
                guaAutomationForm->TempAlwaysOnTop(0);
            }
#endif
            QString strFilename = QFileDialog::getOpenFileName(this, "Open File", gstrLastFilename[FilenameIndexApplication], "SmartBasic Applications (*.uwc);;All Files (*.*)");
#ifndef SKIPAUTOMATIONFORM
            if (guaAutomationForm != 0)
            {
                guaAutomationForm->TempAlwaysOnTop(1);
            }
#endif

            if (strFilename.length() > 1)
            {
                //Set last directory config
                gstrLastFilename[FilenameIndexApplication] = strFilename;
                gpTermSettings->setValue("LastFileDirectory", SplitFilePath(strFilename)[0]);

                //Run file
                if (strFilename.lastIndexOf(".") >= 0)
                {
                    //Get up until the first dot
                    QRegularExpression reTmpRE("^(.*)/(.*)$");
                    QRegularExpressionMatch remTempREM = reTmpRE.match(strFilename);

                    if (remTempREM.hasMatch() == true)
                    {
                        strFilename = remTempREM.captured(2);
                        if (strFilename.count(".") > 0)
                        {
                            //Strip off after the dot
                            strFilename = strFilename.left(strFilename.indexOf("."));
                        }
                    }
                }
                QByteArray baTmpBA = QString("at+run \"").append(strFilename).append("\"").toUtf8();
                gspSerialPort.write(baTmpBA);
                gintQueuedTXBytes += baTmpBA.size();
                gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
                DoLineEnd();

                //Update display buffer
                gbaDisplayBuffer.append(baTmpBA);
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
            }
        }
    }
#ifndef SKIPAUTOMATIONFORM
    else if (intItem == MenuActionAutomation)
    {
        //Show automation window
        if (guaAutomationForm == 0)
        {
            //Initialise automation popup
            guaAutomationForm = new UwxAutomation(this);

            //Populate window handles for automation object
            guaAutomationForm->SetPopupHandle(gpmErrorForm);

            //Update automation form with connection status
            guaAutomationForm->ConnectionChange(gspSerialPort.isOpen());

            //Connect signals
            connect(guaAutomationForm, SIGNAL(SendData(QString,bool,bool)), this, SLOT(MessagePass(QString,bool,bool)));

            //Give focus to the first line
            guaAutomationForm->SetFirstLineFocus();
        }
        guaAutomationForm->show();
    }
#endif
#ifndef SKIPSCRIPTINGFORM
    else if (intItem == MenuActionScripting)
    {
        //Show scripting window
        if (gusScriptingForm == 0)
        {
            //Initialise scripting form
            gusScriptingForm = new UwxScripting(this);

            //Populate window handles for automation object
            gusScriptingForm->SetPopupHandle(gpmErrorForm);

            //Send the UwTerminalX version string
            gusScriptingForm->SetUwTerminalXVersion(UwVersion);

            //Connect the message passing signal and slot
            connect(gusScriptingForm, SIGNAL(SendData(QString,bool,bool)), this, SLOT(MessagePass(QString,bool,bool)));
            connect(gusScriptingForm, SIGNAL(ScriptStartRequest()), this, SLOT(ScriptStartRequest()));
            connect(gusScriptingForm, SIGNAL(ScriptFinished()), this, SLOT(ScriptFinished()));

            if (gspSerialPort.isOpen())
            {
                //Tell the form that the serial port is open
                gusScriptingForm->SerialPortStatus(true);
            }
        }
        gusScriptingForm->show();
        gusScriptingForm->SetEditorFocus();
    }
#endif
    else if (intItem == MenuActionBatch)
    {
        //Start a Batch file script
        if (gspSerialPort.isOpen() == true && gbLoopbackMode == false && gbTermBusy == false)
        {
            //Not currently busy
#ifndef SKIPAUTOMATIONFORM
            if (guaAutomationForm != 0)
            {
                guaAutomationForm->TempAlwaysOnTop(0);
            }
#endif
            QString strFilename = QFileDialog::getOpenFileName(this, tr("Open Batch File"), gstrLastFilename[FilenameIndexOthers], tr("Text Files (*.txt);;All Files (*.*)"));
#ifndef SKIPAUTOMATIONFORM
            if (guaAutomationForm != 0)
            {
                guaAutomationForm->TempAlwaysOnTop(1);
            }
#endif

            if (strFilename.length() > 1)
            {
                //Set last directory config
                gstrLastFilename[FilenameIndexOthers] = strFilename;
                gpTermSettings->setValue("LastOtherFileDirectory", SplitFilePath(strFilename)[0]);

                //File selected
                gpStreamFileHandle = new QFile(strFilename);

                if (!gpStreamFileHandle->open(QIODevice::ReadOnly))
                {
                    //Unable to open file
                    QString strMessage = tr("Error during batch streaming: Access to selected file is denied: ").append(strFilename);
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                    return;
                }

                //We're now busy
                gbTermBusy = true;
                gbStreamingBatch = true;
                gchTermMode = 50;
                gbaBatchReceive.clear();
                ui->btn_Cancel->setEnabled(true);

                //Start a timer
                gtmrStreamTimer.start();

                //Reads out first block
                QByteArray baFileData = gpStreamFileHandle->readLine().replace("\n", "").replace("\r", "");
                gspSerialPort.write(baFileData);
                gintQueuedTXBytes += baFileData.size();
                DoLineEnd();
                gpMainLog->WriteLogData(QString(baFileData).append("\n"));
                gintStreamBytesRead = 1;

                //Update the display buffer
                gbaDisplayBuffer.append(baFileData);
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }

                //Start a timeout timer
                gtmrBatchTimeoutTimer.start(BatchTimeout);
            }
        }
    }
    else if (intItem == MenuActionClearModule)
    {
        //Clear module
        if (gspSerialPort.isOpen() == true && gbLoopbackMode == false && gbTermBusy == false)
        {
            if (!ui->check_ConfirmClear->isChecked() || QMessageBox::question(this, "Clear module and return to default settings?", "You are about to clear the whole module including configuration keys and applications. This process is irreversible!\r\n\r\nClick yes to proceed ot no to cancel.", QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
            {
                QByteArray baTmpBA = QString("at&f*").toUtf8();
                gspSerialPort.write(baTmpBA);
                gintQueuedTXBytes += baTmpBA.size();
                DoLineEnd();
                gbaDisplayBuffer.append(baTmpBA);
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
                gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
            }
        }
    }
    else if (intItem == MenuActionClearDisplay)
    {
        //Clear display
        ui->text_TermEditData->ClearDatIn();
    }
    else if (intItem == MenuActionClearRxTx)
    {
        //Clear counts
        gintRXBytes = 0;
        gintTXBytes = 0;
        ui->label_TermRx->setText(QString::number(gintRXBytes));
        ui->label_TermTx->setText(QString::number(gintTXBytes));
    }
    else if (intItem == MenuActionCopy)
    {
        //Copy selected data
        QApplication::clipboard()->setText(ui->text_TermEditData->textCursor().selection().toPlainText());
    }
    else if (intItem == MenuActionCopyAll)
    {
        //Copy all data
        QApplication::clipboard()->setText(ui->text_TermEditData->toPlainText());
    }
    else if (intItem == MenuActionPaste)
    {
        //Paste data from clipboard
        ui->text_TermEditData->AddDatOutText(QApplication::clipboard()->text());
    }
    else if (intItem == MenuActionSelectAll)
    {
        //Select all text
        ui->text_TermEditData->selectAll();
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::balloontriggered(
    QAction* qaAction
    )
{
    //Runs when a balloon menu item is selected
    int intItem = qaAction->data().toInt();
    if (intItem == BalloonActionShow)
    {
        //Make UwTerminalX the active window
#if TARGET_OS_MAC
        //Bugfix for mac (icon vanishes when clicked)
        gpSysTray->setIcon(QIcon(*gpUw16Pixmap));
#endif
        this->raise();
        this->activateWindow();
    }
    else if (intItem == BalloonActionExit)
    {
        //Exit
        QApplication::quit();
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::EnterPressed(
    )
{
    //Enter pressed in line mode
    if (gspSerialPort.isOpen())
    {
        if (gbTermBusy == false)
        {
            if (gbLoopbackMode == false)
            {
                QByteArray baTmpBA = ui->text_TermEditData->GetDatOut()->replace("\n\r", "\n").replace("\r\n", "\n").replace("\n", (ui->radio_LCR->isChecked() ? "\r" : ui->radio_LLF->isChecked() ? "\n" : ui->radio_LCRLF->isChecked() ? "\r\n" : ui->radio_LLFCR->isChecked() ? "\n\r" : "")).replace("\r", (ui->radio_LCR->isChecked() ? "\r" : ui->radio_LLF->isChecked() ? "\n" : ui->radio_LCRLF->isChecked() ? "\r\n" : ui->radio_LLFCR->isChecked() ? "\n\r" : "")).toUtf8();
                gspSerialPort.write(baTmpBA);
                gintQueuedTXBytes += baTmpBA.size();
                DoLineEnd();

                //Add to log
                gpMainLog->WriteLogData(baTmpBA.append("\n"));
            }
            else if (gbLoopbackMode == true)
            {
                //Loopback is enabled
                gbaDisplayBuffer.append("\n[Cannot send: Loopback mode is enabled.]\n");
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
            }

            if (ui->check_Echo->isChecked() == true)
            {
                //Local echo
                QByteArray baTmpBA = ui->text_TermEditData->GetDatOut()->toUtf8();
                baTmpBA.append("\n");
                ui->text_TermEditData->AddDatInText(&baTmpBA);
            }
            ui->text_TermEditData->ClearDatOut();
        }
    }
}

//=============================================================================
//=============================================================================
unsigned char
MainWindow::CompileApp(
    unsigned char chMode
    )
{
    //Runs when an application is to be compiled
    gchTermMode = chMode;
#ifndef SKIPAUTOMATIONFORM
    if (guaAutomationForm != 0)
    {
        guaAutomationForm->TempAlwaysOnTop(0);
    }
#endif
    QString strFilename = QFileDialog::getOpenFileName(this, (chMode == 6 || chMode == 7 ? tr("Open File") : (chMode == MODE_LOAD || chMode == MODE_LOAD_RUN ? tr("Open SmartBasic Application") : tr("Open SmartBasic Source"))), gstrLastFilename[FilenameIndexApplication], (chMode == 6 || chMode == 7 ? tr("All Files (*.*)") : (chMode == MODE_LOAD || chMode == MODE_LOAD_RUN ? tr("SmartBasic Applications (*.uwc);;All Files (*.*)") : tr("Text/SmartBasic Files (*.txt *.sb);;All Files (*.*)"))));
#ifndef SKIPAUTOMATIONFORM
    if (guaAutomationForm != 0)
    {
        guaAutomationForm->TempAlwaysOnTop(1);
    }
#endif

    if (strFilename != "")
    {
        //Set last directory config
        gstrTermFilename = strFilename;
        gstrLastFilename[FilenameIndexApplication] = strFilename;
        gpTermSettings->setValue("LastFileDirectory", SplitFilePath(gstrTermFilename)[0]);

        if (chMode == MODE_LOAD || chMode == MODE_LOAD_RUN)
        {
            //Loading a compiled application
            gbTermBusy = true;
            LoadFile(false);

            //Download to the device
            gchTermMode2 = MODE_COMPILE;
            QByteArray baTmpBA = QString("AT+DEL \"").append(gstrDownloadFilename).append("\" +").toUtf8();
            gspSerialPort.write(baTmpBA);
            gintQueuedTXBytes += baTmpBA.size();
            DoLineEnd();
            gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
            if (ui->check_SkipDL->isChecked() == false)
            {
                //Output download details
                if (ui->check_ShowCLRF->isChecked() == true)
                {
                    //Escape \t, \r and \n
                    baTmpBA.replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n");
                }

                //Replace unprintable characters
                baTmpBA.replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f");

                //Update display buffer
                gbaDisplayBuffer.append(baTmpBA);
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
            }

            //Start the timeout timer
            gtmrDownloadTimeoutTimer.start();
        }
        else if (chMode == 6 || chMode == 7)
        {
            //Download any file to device
            gbTermBusy = true;
            LoadFile(false);

            //Download to the device
            gchTermMode2 = MODE_COMPILE;
            QByteArray baTmpBA = QString("AT+DEL \"").append(gstrDownloadFilename).append("\" +").toUtf8();
            gspSerialPort.write(baTmpBA);
            gintQueuedTXBytes += baTmpBA.size();
            DoLineEnd();
            gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
            if (ui->check_SkipDL->isChecked() == false)
            {
                //Output download details
                if (ui->check_ShowCLRF->isChecked() == true)
                {
                    //Escape \t, \r and \n
                    baTmpBA.replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n");
                }

                //Replace unprintable characters
                baTmpBA.replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f");

                //Update display buffer
                gbaDisplayBuffer.append(baTmpBA);
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
            }

            //Start the timeout timer
            gtmrDownloadTimeoutTimer.start();
        }
        else
        {
            //A file was selected, get the version number
            gbTermBusy = true;
            gchTermMode2 = 0;
            gchTermBusyLines = 0;
            gstrTermBusyData = tr("");
            if (ui->check_CheckLicense->isChecked() && (chMode == MODE_COMPILE_LOAD || chMode == MODE_COMPILE_LOAD_RUN || chMode == MODE_SERVER_COMPILE_LOAD || chMode == MODE_SERVER_COMPILE_LOAD_RUN))
            {
                //Check license
                gspSerialPort.write("at i 4");
                gintQueuedTXBytes += 6;
                DoLineEnd();
                gpMainLog->WriteLogData("at i 4\n");
            }
            gspSerialPort.write("at i 0");
            gintQueuedTXBytes += 6;
            DoLineEnd();
            gpMainLog->WriteLogData("at i 0\n");
            gspSerialPort.write("at i 13");
            gintQueuedTXBytes += 7;
            DoLineEnd();
            gpMainLog->WriteLogData("at i 13\n");

            //Start the timeout timer
            gtmrDownloadTimeoutTimer.start();
        }
    }
    ui->btn_Cancel->setEnabled(true);
    return 0;
}

//=============================================================================
//=============================================================================
void
MainWindow::UpdateImages(
    )
{
    //Updates images to reflect status
    if (gspSerialPort.isOpen() == true)
    {
        //Port open
        SerialStatus(true);
    }
    else
    {
        //Port closed
        ui->image_CTS->setPixmap(*gpEmptyCirclePixmap);
        ui->image_DCD->setPixmap(*gpEmptyCirclePixmap);
        ui->image_DSR->setPixmap(*gpEmptyCirclePixmap);
        ui->image_RI->setPixmap(*gpEmptyCirclePixmap);

        ui->image_CTSb->setPixmap(*gpEmptyCirclePixmap);
        ui->image_DCDb->setPixmap(*gpEmptyCirclePixmap);
        ui->image_DSRb->setPixmap(*gpEmptyCirclePixmap);
        ui->image_RIb->setPixmap(*gpEmptyCirclePixmap);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::KeyPressed(
    QChar chrKeyValue
    )
{
    //Key pressed, send it out
    if (gspSerialPort.isOpen())
    {
        if (gbTermBusy == false && gbLoopbackMode == false)
        {
            if (ui->check_Echo->isChecked() == true)
            {
                //Echo mode on
                if (chrKeyValue == Qt::Key_Enter || chrKeyValue == Qt::Key_Return)
                {
                    gbaDisplayBuffer.append("\n");
                }
                else
                {
                    /*if (ui->check_ShowCLRF->isChecked() == true)
                    {
                        //Escape \t, \r and \n in addition to normal escaping
                        gbaDisplayBuffer.append(QString(QChar(chrKeyValue)).toUtf8().replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n").replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f"));
                    }
                    else
                    {
                        //Normal escaping
                        gbaDisplayBuffer.append(QString(QChar(chrKeyValue)).toUtf8().replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f"));
                    }*/
                }
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
            }

            //Convert character to a byte array (in case it's UTF-8 and more than 1 byte)
            QByteArray baTmpBA = QString(chrKeyValue).toUtf8();

            //Character mode, send right on
            if (chrKeyValue == Qt::Key_Enter || chrKeyValue == Qt::Key_Return || chrKeyValue == '\r' || chrKeyValue == '\n')
            {
                //Return key or newline
                gpMainLog->WriteLogData("\n");
                DoLineEnd();
            }
            else
            {
                //Not return
                gspSerialPort.write(baTmpBA);
                gintQueuedTXBytes += baTmpBA.size();
            }

            //Output back to screen buffer if echo mode is enabled
            if (ui->check_Echo->isChecked())
            {
                if (ui->check_ShowCLRF->isChecked() == true)
                {
                    //Escape \t, \r and \n in addition to normal escaping
                    gbaDisplayBuffer.append(baTmpBA.replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n").replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f"));
                }
                else
                {
                    //Normal escaping
                    gbaDisplayBuffer.append(baTmpBA.replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f"));
                }

                //Run display update timer
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }

                //Output to log file
                gpMainLog->WriteLogData(QString(chrKeyValue).toUtf8());
            }
        }
        else if (gbLoopbackMode == true)
        {
            //Loopback is enabled
            gbaDisplayBuffer.append("[Cannot send: Loopback mode is enabled.]");
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::DoLineEnd(
    )
{
    //Outputs a line ending
    if (ui->radio_LCR->isChecked())
    {
        //CR
        gspSerialPort.write("\r");
        gintQueuedTXBytes += 1;
    }
    else if (ui->radio_LLF->isChecked())
    {
        //LF
        gspSerialPort.write("\n");
        gintQueuedTXBytes += 1;
    }
    else if (ui->radio_LCRLF->isChecked())
    {
        //CR LF
        gspSerialPort.write("\r\n");
        gintQueuedTXBytes += 2;
    }
    else if (ui->radio_LLFCR->isChecked())
    {
        //LF CR
        gspSerialPort.write("\n\r");
        gintQueuedTXBytes += 2;
    }
    return;
}

//=============================================================================
//=============================================================================
void
MainWindow::DevRespTimeout(
    )
{
    //Runs to indiciate a device timeout when waiting to compile an application
    if (gbTermBusy == true)
    {
        //Update buffer
        gbaDisplayBuffer.append("\nTimeout occured whilst attempting to XCompile application or download to module.\n");
        if (!gtmrTextUpdateTimer.isActive())
        {
            gtmrTextUpdateTimer.start();
        }

        //Reset variables
        gbTermBusy = false;
        gchTermBusyLines = 0;
        gstrTermBusyData = tr("");
        ui->btn_Cancel->setEnabled(false);
    }
}

#ifdef _WIN32
//=============================================================================
//=============================================================================
void
MainWindow::process_finished(
    int intExitCode,
    QProcess::ExitStatus
    )
{
    if (ui->check_PreXCompRun->isChecked() == true && ui->radio_XCompPost->isChecked() == true && (intExitCode == 0 || ui->check_PreXCompFail->isChecked() == true))
    {
        //Run Post-XComp program
        RunPrePostExecutable(gstrTermFilename);
    }

    //Exit code 1: Fail, Exit code 0: Success
    if (intExitCode == 1)
    {
        //Display an error message
        QString strMessage = tr("Error during XCompile:\n").append(gprocCompileProcess.readAllStandardOutput());
        gpmErrorForm->show();
        gpmErrorForm->SetMessage(&strMessage);
        gbTermBusy = false;
        ui->btn_Cancel->setEnabled(false);
    }
    else if (intExitCode == 0)
    {
        //Success
        if (gchTermMode == MODE_COMPILE)
        {
            //XCompile complete
            gtmrDownloadTimeoutTimer.stop();
            gstrHexData = "";
            if (ui->check_ShowFileSize->isChecked())
            {
                //Display size
                QList<QString> lstFI = SplitFilePath(gstrTermFilename);
                gbaDisplayBuffer.append("\n-- XCompile complete (").append(CleanFilesize(QString(lstFI[0]).append(lstFI[1]).append(".uwc"))).append(") --\n");
            }
            else
            {
                //Normal message
                gbaDisplayBuffer.append("\n-- XCompile complete --\n");
            }
            if (!gtmrTextUpdateTimer.isActive())
            {
                gtmrTextUpdateTimer.start();
            }
            gchTermMode = 0;
            gchTermMode2 = 0;
            gbTermBusy = false;
            ui->btn_Cancel->setEnabled(false);
        }
        else if (gchTermMode == MODE_COMPILE_LOAD || gchTermMode == MODE_COMPILE_LOAD_RUN)
        {
            //Load the file
            LoadFile(true);
            gchTermMode2 = MODE_COMPILE;

            if (ui->check_ShowFileSize->isChecked())
            {
                //Display size of application
                QList<QString> lstFI = SplitFilePath(gstrTermFilename);
                gbaDisplayBuffer.append("\n-- XCompile complete (").append(CleanFilesize(QString(lstFI[0]).append(lstFI[1]).append(".uwc"))).append(") Downloading --\n");
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
            }

            //Download to the device
            QByteArray baTmpBA = QString("AT+DEL \"").append(gstrDownloadFilename).append("\" +").toUtf8();
            gspSerialPort.write(baTmpBA);
            gintQueuedTXBytes += baTmpBA.size();
            DoLineEnd();
            gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
            gtmrDownloadTimeoutTimer.start();
            if (ui->check_SkipDL->isChecked() == false)
            {
                //Output download details
                if (ui->check_ShowCLRF->isChecked() == true)
                {
                    //Escape \t, \r and \n
                    baTmpBA.replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n");
                }

                //Replace unprintable characters
                baTmpBA.replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f");

                //Update display buffer
                gbaDisplayBuffer.append(baTmpBA);
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
            }
        }
    }
    else
    {
        //Unknown exit reason
        QString strMessage = tr("Err code: ").append(QString::number(intExitCode));
        gpmErrorForm->show();
        gpmErrorForm->SetMessage(&strMessage);
        gbTermBusy = false;
        ui->btn_Cancel->setEnabled(false);
    }
}
#endif

//=============================================================================
//=============================================================================
void
MainWindow::SerialStatus(
    bool bType
    )
{
    if (gspSerialPort.isOpen() == true)
    {
        unsigned int intSignals = gspSerialPort.pinoutSignals();
        if ((((intSignals & QSerialPort::ClearToSendSignal) == QSerialPort::ClearToSendSignal ? 1 : 0) != gbCTSStatus || bType == true))
        {
            //CTS changed
            gbCTSStatus = ((intSignals & QSerialPort::ClearToSendSignal) == QSerialPort::ClearToSendSignal ? 1 : 0);
            ui->image_CTS->setPixmap((gbCTSStatus == true ? *gpGreenCirclePixmap : *gpRedCirclePixmap));
            ui->image_CTSb->setPixmap((gbCTSStatus == true ? *gpGreenCirclePixmap : *gpRedCirclePixmap));
        }
        if ((((intSignals & QSerialPort::DataCarrierDetectSignal) == QSerialPort::DataCarrierDetectSignal ? 1 : 0) != gbDCDStatus || bType == true))
        {
            //DCD changed
            gbDCDStatus = ((intSignals & QSerialPort::DataCarrierDetectSignal) == QSerialPort::DataCarrierDetectSignal ? 1 : 0);
            ui->image_DCD->setPixmap((gbDCDStatus == true ? *gpGreenCirclePixmap : *gpRedCirclePixmap));
            ui->image_DCDb->setPixmap((gbDCDStatus == true ? *gpGreenCirclePixmap : *gpRedCirclePixmap));
        }
        if ((((intSignals & QSerialPort::DataSetReadySignal) == QSerialPort::DataSetReadySignal ? 1 : 0) != gbDSRStatus || bType == true))
        {
            //DSR changed
            gbDSRStatus = ((intSignals & QSerialPort::DataSetReadySignal) == QSerialPort::DataSetReadySignal ? 1 : 0);
            ui->image_DSR->setPixmap((gbDSRStatus == true ? *gpGreenCirclePixmap : *gpRedCirclePixmap));
            ui->image_DSRb->setPixmap((gbDSRStatus == true ? *gpGreenCirclePixmap : *gpRedCirclePixmap));
        }
        if ((((intSignals & QSerialPort::RingIndicatorSignal) == QSerialPort::RingIndicatorSignal ? 1 : 0) != gbRIStatus || bType == true))
        {
            //RI changed
            gbRIStatus = ((intSignals & QSerialPort::RingIndicatorSignal) == QSerialPort::RingIndicatorSignal ? 1 : 0);
            ui->image_RI->setPixmap((gbRIStatus == true ? *gpGreenCirclePixmap : *gpRedCirclePixmap));
            ui->image_RIb->setPixmap((gbRIStatus == true ? *gpGreenCirclePixmap : *gpRedCirclePixmap));
        }
    }
    else
    {
        //Port isn't open, display empty circles
        ui->image_CTS->setPixmap(*gpEmptyCirclePixmap);
        ui->image_DCD->setPixmap(*gpEmptyCirclePixmap);
        ui->image_DSR->setPixmap(*gpEmptyCirclePixmap);
        ui->image_RI->setPixmap(*gpEmptyCirclePixmap);

        ui->image_CTSb->setPixmap(*gpEmptyCirclePixmap);
        ui->image_DCDb->setPixmap(*gpEmptyCirclePixmap);
        ui->image_DSRb->setPixmap(*gpEmptyCirclePixmap);
        ui->image_RIb->setPixmap(*gpEmptyCirclePixmap);

        //Disable timer
        gpSignalTimer->stop();
    }
    return;
}

//=============================================================================
//=============================================================================
void
MainWindow::SerialStatusSlot(
    )
{
    //Slot function to update serial pinout status
    SerialStatus(0);
}

//=============================================================================
//=============================================================================
void
MainWindow::OpenDevice(
    )
{
    //Function to open serial port
    if (gspSerialPort.isOpen() == true)
    {
        //Serial port is already open - cancel any pending operations
        if (gbTermBusy == true && gbFileOpened == true)
        {
            //Run cancel operation
            on_btn_Cancel_clicked();
        }

        //Close serial port
        while (gspSerialPort.isOpen() == true)
        {
            gspSerialPort.clear();
            gspSerialPort.close();
        }
        gpSignalTimer->stop();

        //Change status message
        ui->statusBar->showMessage("");

#ifndef SKIPAUTOMATIONFORM
        //Notify automation form
        if (guaAutomationForm != 0)
        {
            guaAutomationForm->ConnectionChange(false);
        }
#endif

        //Update images
        UpdateImages();

        //Close log file if open
        if (gpMainLog->IsLogOpen() == true)
        {
            gpMainLog->CloseLogFile();
        }
    }

    if (ui->combo_COM->currentText().length() > 0)
    {
        //Port selected: setup serial port
        gspSerialPort.setPortName(ui->combo_COM->currentText());
        gspSerialPort.setBaudRate(ui->combo_Baud->currentText().toInt());
        gspSerialPort.setDataBits((QSerialPort::DataBits)ui->combo_Data->currentText().toInt());
        gspSerialPort.setStopBits((QSerialPort::StopBits)ui->combo_Stop->currentText().toInt());

        //Parity
        gspSerialPort.setParity((ui->combo_Parity->currentIndex() == 1 ? QSerialPort::OddParity : (ui->combo_Parity->currentIndex() == 2 ? QSerialPort::EvenParity : QSerialPort::NoParity)));

        //Flow control
        gspSerialPort.setFlowControl((ui->combo_Handshake->currentIndex() == 1 ? QSerialPort::HardwareControl : (ui->combo_Handshake->currentIndex() == 2 ? QSerialPort::SoftwareControl : QSerialPort::NoFlowControl)));

        if (gspSerialPort.open(QIODevice::ReadWrite))
        {
            //Successful
            ui->statusBar->showMessage(QString("[").append(ui->combo_COM->currentText()).append(":").append(ui->combo_Baud->currentText()).append(",").append((ui->combo_Parity->currentIndex() == 0 ? "N" : ui->combo_Parity->currentIndex() == 1 ? "O" : ui->combo_Parity->currentIndex() == 2 ? "E" : "")).append(",").append(ui->combo_Data->currentText()).append(",").append(ui->combo_Stop->currentText()).append(",").append((ui->combo_Handshake->currentIndex() == 0 ? "N" : ui->combo_Handshake->currentIndex() == 1 ? "H" : ui->combo_Handshake->currentIndex() == 2 ? "S" : "")).append("]{").append((ui->radio_LCR->isChecked() ? "cr" : (ui->radio_LLF->isChecked() ? "lf" : (ui->radio_LCRLF->isChecked() ? "cr lf" : (ui->radio_LLFCR->isChecked() ? "lf cr" : ""))))).append("}"));
            ui->label_TermConn->setText(ui->statusBar->currentMessage());
            ui->label_SpeedConn->setText(ui->statusBar->currentMessage());

            //Switch to Terminal tab if not on terminal or speed testing tab
            if (ui->selector_Tab->currentIndex() != TabTerminal && ui->selector_Tab->currentIndex() != TabSpeedTest)
            {
                ui->selector_Tab->setCurrentIndex(TabTerminal);
            }

            //Disable read-only mode
            ui->text_TermEditData->setReadOnly(false);

            //DTR
            gspSerialPort.setDataTerminalReady(ui->check_DTR->isChecked());

            //Flow control
            if (ui->combo_Handshake->currentIndex() == 1)
            {
                //Hardware handshaking
                ui->check_RTS->setEnabled(false);
                ui->check_SpeedRTS->setEnabled(false);
            }
            else
            {
                //Not hardware handshaking - RTS
                ui->check_RTS->setEnabled(true);
                ui->check_SpeedRTS->setEnabled(true);
                gspSerialPort.setRequestToSend(ui->check_RTS->isChecked());
            }

            //Break
            gspSerialPort.setBreakEnabled(ui->check_Break->isChecked());

            //Enable checkboxes
            ui->check_Break->setEnabled(true);
            ui->check_DTR->setEnabled(true);
            ui->check_SpeedDTR->setEnabled(true);
            ui->check_Echo->setEnabled(true);
            ui->check_Line->setEnabled(true);

            //Update button text
            ui->btn_TermClose->setText("C&lose Port");
            ui->btn_SpeedClose->setText("C&lose Port");

            //Signal checking
            SerialStatus(1);

            //Enable timer
            gpSignalTimer->start(gpTermSettings->value("SerialSignalCheckInterval", DefaultSerialSignalCheckInterval).toUInt());

#ifndef SKIPAUTOMATIONFORM
            //Notify automation form
            if (guaAutomationForm != 0)
            {
                guaAutomationForm->ConnectionChange(true);
            }
#endif

            //Notify scroll edit
            ui->text_TermEditData->SetSerialOpen(true);

            //Set focus to input text edit
            ui->text_TermEditData->setFocus();

            //Disable log options
            ui->edit_LogFile->setEnabled(false);
            ui->check_LogEnable->setEnabled(false);
            ui->check_LogAppend->setEnabled(false);
            ui->btn_LogFileSelect->setEnabled(false);

            //Enable speed testing
            ui->btn_SpeedStart->setEnabled(true);

            //Open log file
            if (ui->check_LogEnable->isChecked() == true)
            {
                //Logging is enabled
#if TARGET_OS_MAC
                if (gpMainLog->OpenLogFile(QString((ui->edit_LogFile->text().left(1) == "/" || ui->edit_LogFile->text().left(1) == "\\") ? "" : QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/").append(ui->edit_LogFile->text())) == LOG_OK)
#else
                if (gpMainLog->OpenLogFile(ui->edit_LogFile->text()) == LOG_OK)
#endif
                {
                    //Log opened
                    if (ui->check_LogAppend->isChecked() == false)
                    {
                        //Clear the log file
                        gpMainLog->ClearLog();
                    }
                    gpMainLog->WriteLogData(tr("-").repeated(31));
                    gpMainLog->WriteLogData(tr("\n Log opened ").append(QDate::currentDate().toString("dd/MM/yyyy")).append(" @ ").append(QTime::currentTime().toString("hh:mm")).append(" \n"));
                    gpMainLog->WriteLogData(tr(" UwTerminalX ").append(UwVersion).append(" \n"));
                    gpMainLog->WriteLogData(QString(" Port: ").append(ui->combo_COM->currentText()).append("\n"));
                    gpMainLog->WriteLogData(tr("-").repeated(31).append("\n\n"));
                    gbMainLogEnabled = true;
                }
                else
                {
                    //Log not writeable
                    QString strMessage = tr("Error whilst opening log.\nPlease ensure you have access to the log file ").append(ui->edit_LogFile->text()).append(" and have enough free space on your hard drive.");
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                }
            }

            //Allow file drops for uploads
            setAcceptDrops(true);

#ifndef SKIPSCRIPTINGFORM
            if (gusScriptingForm != 0)
            {
                gusScriptingForm->SerialPortStatus(true);
            }
#endif
        }
        else
        {
            //Error whilst opening
            ui->statusBar->showMessage("Error: ");
            ui->statusBar->showMessage(ui->statusBar->currentMessage().append(gspSerialPort.errorString()));

            QString strMessage = tr("Error whilst attempting to open the serial device: ").append(gspSerialPort.errorString()).append("\n\nIf the serial port is open in another application, please close the other application")
#if !defined(_WIN32) && !defined( __APPLE__)
            .append(", please also ensure you have been granted permission to the serial device in /dev/")
#endif
            .append((ui->combo_Baud->currentText().toULong() > 115200 ? ", please also ensure that your serial device supports baud rates greater than 115200 (normal COM ports do not have support for these baud rates)" : ""))
            .append(" and try again.");
            gpmErrorForm->show();
            gpmErrorForm->SetMessage(&strMessage);
            ui->text_TermEditData->SetSerialOpen(false);
        }
    }
    else
    {
        //No serial port selected
        QString strMessage = tr("No serial port was selected, please select a serial port and try again.\r\nIf you see no serial ports listed, ensure your device is connected to your computer and you have the appropriate drivers installed.");
        gpmErrorForm->show();
        gpmErrorForm->SetMessage(&strMessage);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::LoadFile(
    bool bToUWC
    )
{
    //Load
    QList<QString> lstFI = SplitFilePath(gstrTermFilename);
    QFile fileFileName((bToUWC == true ? QString(lstFI[0]).append(lstFI[1]).append(".uwc") : gstrTermFilename));
    if (!fileFileName.open(QIODevice::ReadOnly))
    {
        //Unable to open file
        QString strMessage = tr("Error during XCompile: Access to selected file is denied: ").append((bToUWC ? QString(lstFI[0]).append(lstFI[1]).append(".uwc") : gstrTermFilename));
        gpmErrorForm->show();
        gpmErrorForm->SetMessage(&strMessage);
        gbTermBusy = false;
        ui->btn_Cancel->setEnabled(false);
        return;
    }

    //Is this a UWC download?
    gbIsUWCDownload = bToUWC;

    //Create a data stream and hex string holder
    QDataStream in(&fileFileName);
    gstrHexData = "";
    while (!in.atEnd())
    {
        //One byte at a time, convert the data to hex
        quint8 ThisByte;
        in >> ThisByte;
        QString strThisHex;
        strThisHex.setNum(ThisByte, 16);
        if (strThisHex.length() == 1)
        {
            //Expand to 2 characters
            gstrHexData.append("0");
        }

        //Add the hex character to the string
        gstrHexData.append(strThisHex.toUpper());
    }

    //Close the file handle
    fileFileName.close();

    //Download filename is filename without a file extension
    gstrDownloadFilename = (lstFI[1].indexOf(".") == -1 ? lstFI[1] : lstFI[1].left(lstFI[1].indexOf(".")));
}

//=============================================================================
//=============================================================================
void
MainWindow::RunApplication(
    )
{
    //Runs an application
    QByteArray baTmpBA = QString("AT+RUN \"").append(gstrDownloadFilename).append("\"").toUtf8();
    gspSerialPort.write(baTmpBA);
    gintQueuedTXBytes += baTmpBA.size();
    DoLineEnd();
    if (ui->check_SkipDL->isChecked() == false)
    {
        //Output download details
        if (ui->check_ShowCLRF->isChecked() == true)
        {
            //Escape \t, \r and \n
            baTmpBA.replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n");
        }

        //Replace unprintable characters
        baTmpBA.replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f");

        //Update display buffer
        gbaDisplayBuffer.append(baTmpBA);
        if (!gtmrTextUpdateTimer.isActive())
        {
            gtmrTextUpdateTimer.start();
        }
    }

    if (gchTermMode == MODE_COMPILE_LOAD_RUN || gchTermMode == MODE_LOAD_RUN)
    {
        //Finished
        gbTermBusy = false;
        ui->btn_Cancel->setEnabled(false);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_Break_stateChanged(
    )
{
    //Break status changed
    gspSerialPort.setBreakEnabled(ui->check_Break->isChecked());
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_RTS_stateChanged(
    )
{
    //RTS status changed
    gspSerialPort.setRequestToSend(ui->check_RTS->isChecked());
    if (ui->check_SpeedRTS->isChecked() != ui->check_RTS->isChecked())
    {
        //Update speed form checkbox
        ui->check_SpeedRTS->setChecked(ui->check_RTS->isChecked());
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_DTR_stateChanged(
    )
{
    //DTR status changed
    gspSerialPort.setDataTerminalReady(ui->check_DTR->isChecked());
    if (ui->check_SpeedDTR->isChecked() != ui->check_DTR->isChecked())
    {
        //Update speed form checkbox
        ui->check_SpeedDTR->setChecked(ui->check_DTR->isChecked());
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_Line_stateChanged(
    )
{
    //Line mode status changed
    ui->text_TermEditData->SetLineMode(ui->check_Line->isChecked());
}

//=============================================================================
//=============================================================================
void
MainWindow::SerialError(
    QSerialPort::SerialPortError speErrorCode
    )
{
#ifndef SKIPSCRIPTINGFORM
    if (gbScriptingRunning == true)
    {
        gusScriptingForm->SerialPortError(speErrorCode);
    }
#endif

    if (speErrorCode == QSerialPort::NoError)
    {
        //No error. Why this is ever emitted is a mystery to me.
        return;
    }
#if QT_VERSION < 0x050700
    //As of Qt 5.7 these are now deprecated. It is being left in as a conditional compile for anyone using older versions of Qt to prevent these errors closing the serial port.
    else if (speErrorCode == QSerialPort::ParityError)
    {
        //Parity error
    }
    else if (speErrorCode == QSerialPort::FramingError)
    {
        //Framing error
    }
#endif
    else if (speErrorCode == QSerialPort::ResourceError || speErrorCode == QSerialPort::PermissionError)
    {
        //Resource error or permission error (device unplugged?)
        QString strMessage = tr("Fatal error with serial connection.\nPlease reconnect to the device to continue.");
        gpmErrorForm->show();
        gpmErrorForm->SetMessage(&strMessage);
        ui->text_TermEditData->SetSerialOpen(false);

        if (gspSerialPort.isOpen() == true)
        {
            //Close active connection
            gspSerialPort.close();
        }

        if (gbStreamingFile == true)
        {
            //Clear up file stream
            gtmrStreamTimer.invalidate();
            gbStreamingFile = false;
            gpStreamFileHandle->close();
            delete gpStreamFileHandle;
        }
        else if (gbStreamingBatch == true)
        {
            //Clear up batch
            gtmrStreamTimer.invalidate();
            gtmrBatchTimeoutTimer.stop();
            gbStreamingBatch = false;
            gpStreamFileHandle->close();
            delete gpStreamFileHandle;
            gbaBatchReceive.clear();
        }
        else if (gbSpeedTestRunning == true)
        {
            //Clear up speed testing
            if (gtmrSpeedTestDelayTimer != 0)
            {
                //Clean up timer
                disconnect(gtmrSpeedTestDelayTimer, SIGNAL(timeout()), this, SLOT(SpeedTestStartTimer()));
                disconnect(gtmrSpeedTestDelayTimer, SIGNAL(timeout()), this, SLOT(SpeedTestStopTimer()));
                delete gtmrSpeedTestDelayTimer;
                gtmrSpeedTestDelayTimer = 0;
            }

            ui->btn_SpeedStop->setEnabled(false);
            ui->btn_SpeedStart->setEnabled(false);
            ui->combo_SpeedDataType->setEnabled(true);
            if (ui->combo_SpeedDataType->currentIndex() == 1)
            {
                //Enable string options
                ui->edit_SpeedTestData->setEnabled(true);
                ui->check_SpeedStringUnescape->setEnabled(true);
            }

            //Update values
            OutputSpeedTestAvgStats((gtmrSpeedTimer.nsecsElapsed() < 1000000000 ? 1000000000 : gtmrSpeedTimer.nsecsElapsed()/1000000000));

            //Set speed test as no longer running
            gchSpeedTestMode = SpeedModeInactive;
            gbSpeedTestRunning = false;

            if (gtmrSpeedTimer.isValid())
            {
                //Invalidate speed test timer
                gtmrSpeedTimer.invalidate();
            }
            if (gtmrSpeedTestStats.isActive())
            {
                //Stop stats update timer
                gtmrSpeedTestStats.stop();
            }
            if (gtmrSpeedTestStats10s.isActive())
            {
                //Stop 10 second stats update timer
                gtmrSpeedTestStats10s.stop();
            }

            //Clear buffers
            gbaSpeedMatchData.clear();
            gbaSpeedReceivedData.clear();

            //Show finished message in status bar
            ui->statusBar->showMessage("Speed testing failed due to serial port error.");
        }

        //No longer busy
        gbTermBusy = false;
        gchTermMode = 0;
        gchTermMode2 = 0;

        //Disable cancel button
        ui->btn_Cancel->setEnabled(false);

        //Disable active checkboxes
        ui->check_Break->setEnabled(false);
        ui->check_DTR->setEnabled(false);
        ui->check_SpeedDTR->setEnabled(false);
        ui->check_Echo->setEnabled(false);
        ui->check_Line->setEnabled(false);
        ui->check_RTS->setEnabled(false);
        ui->check_SpeedRTS->setEnabled(false);

        //Disable text entry
        ui->text_TermEditData->setReadOnly(true);

        //Change status message
        ui->statusBar->showMessage("");

        //Change button text
        ui->btn_TermClose->setText("&Open Port");
        ui->btn_SpeedClose->setText("&Open Port");

        //Update images
        UpdateImages();

        //Close log file if open
        if (gpMainLog->IsLogOpen() == true)
        {
            gpMainLog->CloseLogFile();
        }

        //Enable log options
        ui->edit_LogFile->setEnabled(true);
        ui->check_LogEnable->setEnabled(true);
        ui->check_LogAppend->setEnabled(true);
        ui->btn_LogFileSelect->setEnabled(true);

#ifndef SKIPAUTOMATIONFORM
        //Notify automation form
        if (guaAutomationForm != 0)
        {
            guaAutomationForm->ConnectionChange(false);
        }
#endif

        //Show disconnection balloon
        if (gbSysTrayEnabled == true && !this->isActiveWindow() && !gpmErrorForm->isActiveWindow()
#ifndef SKIPAUTOMATIONFORM
           && (guaAutomationForm == 0 || (guaAutomationForm != 0 && !guaAutomationForm->isActiveWindow()))
#endif
           )
        {
            gpSysTray->showMessage(ui->combo_COM->currentText().append(" Removed"), QString("Connection to device ").append(ui->combo_COM->currentText()).append(" has been lost due to disconnection."), QSystemTrayIcon::Critical);
        }

        //Disallow file drops
        setAcceptDrops(false);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Duplicate_clicked(
    )
{
    //Duplicates instance of UwTerminalX
    QProcess DuplicateProcess;
    DuplicateProcess.startDetached(QCoreApplication::applicationFilePath(), QStringList() << "ACCEPT" << tr("COM=").append(ui->combo_COM->currentText()) << tr("BAUD=").append(ui->combo_Baud->currentText()) << tr("STOP=").append(ui->combo_Stop->currentText()) << tr("DATA=").append(ui->combo_Data->currentText()) << tr("PAR=").append(ui->combo_Parity->currentText()) << tr("FLOW=").append(QString::number(ui->combo_Handshake->currentIndex())) << tr("ENDCHR=").append((ui->radio_LCR->isChecked() == true ? "0" : ui->radio_LLF->isChecked() == true ? "1" : ui->radio_LCRLF->isChecked() == true ? "2" : "3")) << tr("LOCALECHO=").append((ui->check_Echo->isChecked() == true ? "1" : "0")) << tr("LINEMODE=").append((ui->check_Line->isChecked() == true ? "1" : "0")) << "NOCONNECT");
}

//=============================================================================
//=============================================================================
void
MainWindow::MessagePass(
    QString strDataString,
    bool bEscapeString,
    bool bFromScripting
    )
{
    //Receive a command from the automation window
    if (gspSerialPort.isOpen() == true && (gbTermBusy == false || bFromScripting == true) && gbLoopbackMode == false)
    {
        if (bEscapeString == true)
        {
            //Escape string sequences
            UwxEscape::EscapeCharacters(&strDataString);
        }
        QByteArray baTmpBA = strDataString.toUtf8();
        gspSerialPort.write(baTmpBA);
        gintQueuedTXBytes += baTmpBA.size();
        if (ui->check_Echo->isChecked() == true)
        {
            if (ui->check_ShowCLRF->isChecked() == true)
            {
                //Escape \t, \r and \n
                baTmpBA.replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n");
            }

            //Replace unprintable characters
            baTmpBA.replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f");

            //Output to display buffer
            gbaDisplayBuffer.append(baTmpBA);
            if (!gtmrTextUpdateTimer.isActive())
            {
                gtmrTextUpdateTimer.start();
            }
        }
        gpMainLog->WriteLogData(strDataString);
        if (bEscapeString == false && bFromScripting == false)
        {
            //Not escaping sequences and not from scripting form so send line end
            DoLineEnd();
            gpMainLog->WriteLogData("\n");
        }
    }
    else if (gspSerialPort.isOpen() == true && gbLoopbackMode == true)
    {
        //Loopback is enabled
        gbaDisplayBuffer.append("\n[Cannot send: Loopback mode is enabled.]\n");
        if (!gtmrTextUpdateTimer.isActive())
        {
            gtmrTextUpdateTimer.start();
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::LookupErrorCode(
    unsigned int intErrorCode
    )
{
    //Looks up an error code and outputs it in the edit (does NOT store it to the log)
    if (gbErrorsLoaded == true)
    {
        //Error file has been loaded
        gbaDisplayBuffer.append(QString("\nError code 0x").append(QString::number(intErrorCode, 16)).append(": ").append(gpErrorMessages->value(QString::number(intErrorCode), "Undefined Error Code").toString()).append("\n"));
    }
    else
    {
        //Error file has not been loaded
        gbaDisplayBuffer.append(QString("\nUnable to lookup error code: error file (codes.csv) not loaded. Check the Update tab to download the latest version.\n"));
    }

    if (!gtmrTextUpdateTimer.isActive())
    {
        gtmrTextUpdateTimer.start();
    }
    ui->text_TermEditData->moveCursor(QTextCursor::End);
}

//=============================================================================
//=============================================================================
void
MainWindow::SerialBytesWritten(
    qint64 intByteCount
    )
{
    //Updates the display with the number of bytes written
    if (gbSpeedTestRunning == true)
    {
        //Speed test is running, pass to speed test function
        SpeedTestBytesWritten(intByteCount);
    }
    else
    {
        //Not running speed test
        gintTXBytes += intByteCount;
        ui->label_TermTx->setText(QString::number(gintTXBytes));
//    ui->label_TermQueue->setText(QString::number(gintQueuedTXBytes));

#ifndef SKIPSCRIPTINGFORM
        if (gusScriptingForm != 0 && gbScriptingRunning == true)
        {
            gusScriptingForm->SerialPortWritten(intByteCount);
        }
#endif

        if (gbStreamingFile == true)
        {
            //File stream in progress, read out a block
            QByteArray baFileData = gpStreamFileHandle->read(FileReadBlock);
            gspSerialPort.write(baFileData);
            gintQueuedTXBytes += baFileData.size();
            gintStreamBytesRead += baFileData.length();
            if (gpStreamFileHandle->atEnd())
            {
                //Finished sending
                FinishStream(false);
            }
            else if (gintStreamBytesRead > gintStreamBytesProgress)
            {
                //Progress output
                gbaDisplayBuffer.append(QString("Streamed ").append(QString::number(gintStreamBytesRead)).append(" bytes (").append(QString::number(gintStreamBytesRead*100/gintStreamBytesSize)).append("%).\n"));
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
                gintStreamBytesProgress = gintStreamBytesProgress + StreamProgress;
            }

            //Update status bar
            if (gintStreamBytesRead == gintStreamBytesSize)
            {
                //Finished streaming file
                ui->statusBar->showMessage("File streaming complete!");
            }
            else
            {
                //Still streaming
                ui->statusBar->showMessage(QString("Streamed ").append(QString::number(gintStreamBytesRead).append(" bytes of ").append(QString::number(gintStreamBytesSize))).append(" (").append(QString::number(gintStreamBytesRead*100/gintStreamBytesSize)).append("%)"));
            }
        }
        else if (gbStreamingBatch == true)
        {
            //Batch file command
            ui->statusBar->showMessage(QString("Sending Batch line number ").append(QString::number(gintStreamBytesRead)));
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Cancel_clicked(
    )
{
    //Cancel current stream or file download
    if (gbTermBusy == true)
    {
        if (gchTermMode >= MODE_COMPILE && gchTermMode < MODE_SERVER_COMPILE)
        {
            //Cancel download
            gtmrDownloadTimeoutTimer.stop();
            gstrHexData = "";
            gbaDisplayBuffer.append("\n-- File download cancelled --\n");
            if (gbFileOpened == true)
            {
                gspSerialPort.write("AT+FCL");
                gintQueuedTXBytes += 6;
                DoLineEnd();
                gpMainLog->WriteLogData("AT+FCL\n");
                if (ui->check_SkipDL->isChecked() == false)
                {
                    //Output download details
                    gbaDisplayBuffer.append("AT+FCL\n");
                }
            }
            if (!gtmrTextUpdateTimer.isActive())
            {
                gtmrTextUpdateTimer.start();
            }
            QList<QString> lstFI = SplitFilePath(gstrTermFilename);
            if (gpTermSettings->value("DelUWCAfterDownload", DefaultDelUWCAfterDownload).toBool() == true && gbIsUWCDownload == true && QFile::exists(QString(lstFI[0]).append(lstFI[1]).append(".uwc")))
            {
                //Remove UWC
                QFile::remove(QString(lstFI[0]).append(lstFI[1]).append(".uwc"));
            }
            gchTermMode = 0;
            gchTermMode2 = 0;
            gbTermBusy = false;
        }
        else if (gbStreamingFile == true)
        {
            //Cancel stream
            FinishStream(true);
        }
        else if (gbStreamingBatch == true)
        {
            //Cancel batch streaming
            FinishBatch(true);
        }
        else if (gbAutoBaud == true)
        {
            //Canacel auto baud-rate detection
            gtmrBaudTimer.stop();
            gbTermBusy = false;
            gbAutoBaud = false;
            gchTermMode = 0;
            ui->btn_Cancel->setEnabled(false);
        }
        else if (gchTermMode >= MODE_SERVER_COMPILE || gchTermMode <= MODE_SERVER_COMPILE_LOAD_RUN)
        {
            //Cancel network request and download
            gnmrReply->abort();

            //Change to just a compile
            if (gchTermMode != MODE_SERVER_COMPILE)
            {
                gchTermMode = MODE_SERVER_COMPILE;
                gchTermMode2 = MODE_SERVER_COMPILE;
            }
            return;

        }
        else if (gchTermMode == MODE_CHECK_ERROR_CODE_VERSIONS || gchTermMode == MODE_CHECK_UWTERMINALX_VERSIONS || gchTermMode == MODE_CHECK_FIRMWARE_VERSIONS || gchTermMode == MODE_CHECK_FIRMWARE_SUPPORT)
        {
            //Cancel network request
            gnmrReply->abort();
            return;
        }
    }

    //Disable button
    ui->btn_Cancel->setEnabled(false);
}

//=============================================================================
//=============================================================================
void
MainWindow::FinishStream(
    bool bType
    )
{
    //Sending a file stream has finished
    if (bType == true)
    {
        //Stream cancelled
        gbaDisplayBuffer.append(QString("\nCancelled stream after ").append(QString::number(gintStreamBytesRead)).append(" bytes (").append(QString::number(1+(gtmrStreamTimer.nsecsElapsed()/1000000000))).append(" seconds) [~").append(QString::number((gintStreamBytesRead/(1+gtmrStreamTimer.nsecsElapsed()/1000000000)))).append(" bytes/second].\n"));
        ui->statusBar->showMessage("File streaming cancelled.");
    }
    else
    {
        //Stream finished
        gbaDisplayBuffer.append(QString("\nFinished streaming file, ").append(QString::number(gintStreamBytesRead)).append(" bytes sent in ").append(QString::number(1+(gtmrStreamTimer.nsecsElapsed()/1000000000))).append(" seconds [~").append(QString::number((gintStreamBytesRead/(1+gtmrStreamTimer.nsecsElapsed()/1000000000)))).append(" bytes/second].\n"));
        ui->statusBar->showMessage("File streaming complete!");
    }

    //Initiate timer for buffer update
    if (!gtmrTextUpdateTimer.isActive())
    {
        gtmrTextUpdateTimer.start();
    }

    //Clear up
    gtmrStreamTimer.invalidate();
    gbTermBusy = false;
    gbStreamingFile = false;
    gchTermMode = 0;
    gpStreamFileHandle->close();
    delete gpStreamFileHandle;
    ui->btn_Cancel->setEnabled(false);
}

//=============================================================================
//=============================================================================
void
MainWindow::FinishBatch(
    bool bType
    )
{
    //Sending a file stream has finished
    if (bType == true)
    {
        //Stream cancelled
        gbaDisplayBuffer.append(QString("\nCancelled batch (").append(QString::number(1+(gtmrStreamTimer.nsecsElapsed()/1000000000))).append(" seconds)\n"));
        ui->statusBar->showMessage("Batch file sending cancelled.");
    }
    else
    {
        //Stream finished
        gbaDisplayBuffer.append(QString("\nFinished sending batch file, ").append(QString::number(gintStreamBytesRead)).append(" lines sent in ").append(QString::number(1+(gtmrStreamTimer.nsecsElapsed()/1000000000))).append(" seconds\n"));
        ui->statusBar->showMessage("Batch file sending complete!");
    }

    //Initiate timer for buffer update
    if (!gtmrTextUpdateTimer.isActive())
    {
        gtmrTextUpdateTimer.start();
    }

    //Clear up and cancel timer
    gtmrStreamTimer.invalidate();
    gtmrBatchTimeoutTimer.stop();
    gbTermBusy = false;
    gbStreamingBatch = false;
    gchTermMode = 0;
    gpStreamFileHandle->close();
    delete gpStreamFileHandle;
    gbaBatchReceive.clear();
    ui->btn_Cancel->setEnabled(false);
}

//=============================================================================
//=============================================================================
void
MainWindow::UpdateReceiveText(
    )
{
    //Updates the receive text buffer
    ui->text_TermEditData->AddDatInText(&gbaDisplayBuffer);
    gbaDisplayBuffer.clear();
}

//=============================================================================
//=============================================================================
void
MainWindow::BatchTimeoutSlot(
    )
{
    //A response to a batch command has timed out
    gbaDisplayBuffer.append("\nModule command timed out.\n");
    if (!gtmrTextUpdateTimer.isActive())
    {
        gtmrTextUpdateTimer.start();
    }
    gbTermBusy = false;
    gbStreamingBatch = false;
    gchTermMode = 0;
    gpStreamFileHandle->close();
    ui->btn_Cancel->setEnabled(false);
    delete gpStreamFileHandle;
}

//=============================================================================
//=============================================================================
void
MainWindow::on_combo_COM_currentIndexChanged(
    int
    )
{
    //Serial port selection has been changed, update text
    if (ui->combo_COM->currentText().length() > 0)
    {
        QSerialPortInfo SerialInfo(ui->combo_COM->currentText());
        if (SerialInfo.isValid())
        {
            //Port exists
            QString strDisplayText(SerialInfo.description());
            if (SerialInfo.manufacturer().length() > 1)
            {
                //Add manufacturer
                strDisplayText.append(" (").append(SerialInfo.manufacturer()).append(")");
            }
            if (SerialInfo.serialNumber().length() > 1)
            {
                //Add serial
                strDisplayText.append(" [").append(SerialInfo.serialNumber()).append("]");
            }
            ui->label_SerialInfo->setText(strDisplayText);
        }
        else
        {
            //No such port
            ui->label_SerialInfo->setText("Invalid serial port selected");
        }
    }
    else
    {
        //Clear text as no port is selected
        ui->label_SerialInfo->clear();
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::dragEnterEvent(
    QDragEnterEvent *dragEvent
    )
{
    //A file is being dragged onto the window
    if (dragEvent->mimeData()->urls().count() == 1 && gbTermBusy == false && gspSerialPort.isOpen() == true)
    {
        //Nothing is running, serial handle is open and a single file is being dragged - accept action
        dragEvent->acceptProposedAction();
    }
    else
    {
        //Terminal is busy, serial port is closed or more than 1 file was dropped
        dragEvent->ignore();
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::dropEvent(
    QDropEvent *dropEvent
    )
{
    //A file has been dragged onto the window - send this file if possible
    QList<QUrl> lstURLs = dropEvent->mimeData()->urls();
    if (lstURLs.isEmpty())
    {
        //No files
        return;
    }
    else if (lstURLs.length() > 1)
    {
        //More than 1 file - ignore
        return;
    }

    QString strFileName = lstURLs.first().toLocalFile();
    if (strFileName.isEmpty())
    {
        //Invalid filename
        return;
    }

    //Pass to other function call
    DroppedFile(strFileName);
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_PreXCompRun_stateChanged(
    int intChecked
    )
{
    //User has changed running pre/post XCompiler executable, update GUI
    if (intChecked == 2)
    {
        //Enabled
        ui->check_PreXCompFail->setDisabled(false);
        ui->radio_XCompPost->setDisabled(false);
        ui->radio_XCompPre->setDisabled(false);
        ui->edit_PreXCompFilename->setDisabled(false);
        ui->btn_PreXCompSelect->setDisabled(false);
        ui->label_PreXCompInfo->setDisabled(false);
        ui->label_PreXCompText->setDisabled(false);
        gpTermSettings->setValue("PrePostXCompRun", "1");
    }
    else
    {
        //Disabled
        ui->check_PreXCompFail->setDisabled(true);
        ui->radio_XCompPost->setDisabled(true);
        ui->radio_XCompPre->setDisabled(true);
        ui->edit_PreXCompFilename->setDisabled(true);
        ui->btn_PreXCompSelect->setDisabled(true);
        ui->label_PreXCompInfo->setDisabled(true);
        ui->label_PreXCompText->setDisabled(true);
        gpTermSettings->setValue("PrePostXCompRun", "0");
    }
}

//=============================================================================
//=============================================================================
bool
MainWindow::RunPrePostExecutable(
    QString strFilename
    )
{
    //Runs the pre/post XCompile executable
    QProcess pXProcess;
    if (ui->edit_PreXCompFilename->text().left(1) == "\"")
    {
        //First character is a quote, search for end quote
        int i = 0;
        while (i != -1)
        {
            i = ui->edit_PreXCompFilename->text().indexOf("\"", i+1);
            if (i == -1)
            {
                //No end quote found
                QString strMessage = tr("Error with Pre/Post-XCompile executable: No end quote found for executable file.");
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
                return false;
            }
            else if (ui->edit_PreXCompFilename->text().mid(i-1, 2) != "\\\"")
            {
                //Found end of quote
                if (!QFile::exists(ui->edit_PreXCompFilename->text().mid(1, i-1)))
                {
                    //Executable file doesn't appear to exist
                    QString strMessage = tr("Error with Pre/Post-XCompile executable: File ").append(ui->edit_PreXCompFilename->text().left(i+1)).append(" does not exist.");
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                    return false;
                }

                //Split up all the arguments
                QList<QString> lArguments;
                int l = ui->edit_PreXCompFilename->text().indexOf(" ", i+1);
                while (l != -1)
                {
                    lArguments << ui->edit_PreXCompFilename->text().mid(l+1, ui->edit_PreXCompFilename->text().indexOf(" ", l+1)-l-1).replace("%1", strFilename);
                    l = ui->edit_PreXCompFilename->text().indexOf(" ", l+1);
                }

                if (ui->edit_PreXCompFilename->text().mid(1, i-1).right(4).toLower() == ".bat")
                {
                    //Batch file - run through cmd
                    lArguments.insert(0, ui->edit_PreXCompFilename->text().mid(1, i-1));
                    lArguments.insert(0, "/C");
                    pXProcess.start("cmd", lArguments);
                }
                else
                {
                    //Normal executable
                    pXProcess.start(ui->edit_PreXCompFilename->text().mid(1, i-1), lArguments);
                }
                pXProcess.waitForFinished(PrePostXCompTimeout);
                return true;
            }
        }

        return false;
    }
    else
    {
        //No quote character, search for a space
        if (ui->edit_PreXCompFilename->text().indexOf(" ") == -1)
        {
            //No spaces found, assume whole string is executable
            if (!QFile::exists(ui->edit_PreXCompFilename->text()))
            {
                //Executable file doesn't appear to exist
                QString strMessage = tr("Error with Pre/Post-XCompile executable: File \"").append(ui->edit_PreXCompFilename->text().left(ui->edit_PreXCompFilename->text().indexOf(" "))).append("\" does not exist.");
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
                return false;
            }

            if (ui->edit_PreXCompFilename->text().right(4).toLower() == ".bat")
            {
                //Batch file - run through cmd
                pXProcess.start("cmd", QList<QString>() << "/C" << ui->edit_PreXCompFilename->text().replace("/", "\\"));
            }
            else
            {
                //Normal executable
                pXProcess.start(ui->edit_PreXCompFilename->text());
            }
            pXProcess.waitForFinished(PrePostXCompTimeout);
        }
        else
        {
            //Space found, assume everything before the space is a filename and after are arguments
            QList<QString> lArguments;
            int i = ui->edit_PreXCompFilename->text().indexOf(" ");
            while (i != -1)
            {
                lArguments << ui->edit_PreXCompFilename->text().mid(i+1, ui->edit_PreXCompFilename->text().indexOf(" ", i+1)-i-1).replace("%1", strFilename);
                i = ui->edit_PreXCompFilename->text().indexOf(" ", i+1);
            }

            if (!QFile::exists(ui->edit_PreXCompFilename->text().left(ui->edit_PreXCompFilename->text().indexOf(" "))))
            {
                //Executable file doesn't appear to exist
                QString strMessage = tr("Error with Pre/Post-XCompile executable: File \"").append(ui->edit_PreXCompFilename->text().left(ui->edit_PreXCompFilename->text().indexOf(" "))).append("\" does not exist.");
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
                return false;
            }

            if (ui->edit_PreXCompFilename->text().left(ui->edit_PreXCompFilename->text().indexOf(" ")).right(4).toLower() == ".bat")
            {
                //Batch file - run through cmd
                lArguments.insert(0, ui->edit_PreXCompFilename->text().left(ui->edit_PreXCompFilename->text().indexOf(" ")));
                lArguments.insert(0, "/C");
                pXProcess.start("cmd", lArguments);
            }
            else
            {
                //Normal executable
                pXProcess.start(ui->edit_PreXCompFilename->text().left(ui->edit_PreXCompFilename->text().indexOf(" ")), lArguments);
            }
            pXProcess.waitForFinished(PrePostXCompTimeout);
        }
    }

    //Return success code
    return true;
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_PreXCompSelect_clicked(
    )
{
    //Opens an executable selector
#ifndef SKIPAUTOMATIONFORM
    if (guaAutomationForm != 0)
    {
        guaAutomationForm->TempAlwaysOnTop(0);
    }
#endif
    QString strFilename = QFileDialog::getOpenFileName(this, "Open Executable/batch", gstrLastFilename[FilenameIndexOthers], "Executables/Batch/Bash files (*.exe *.bat *.sh);;All Files (*.*)");
#ifndef SKIPAUTOMATIONFORM
    if (guaAutomationForm != 0)
    {
        guaAutomationForm->TempAlwaysOnTop(1);
    }
#endif

    if (strFilename.length() > 1)
    {
        //Set last directory config
        gstrLastFilename[FilenameIndexOthers] = strFilename;
        gpTermSettings->setValue("LastOtherFileDirectory", SplitFilePath(strFilename)[0]);

        if ((unsigned int)(QFile(strFilename).permissions() & (QFileDevice::ExeOther | QFileDevice::ExeGroup | QFileDevice::ExeUser | QFileDevice::ExeOwner)) == 0)
        {
            //File is not executable, give an error
            QString strMessage = tr("Error: Selected file \"").append(strFilename).append("\" is not an executable file.");
            gpmErrorForm->show();
            gpmErrorForm->SetMessage(&strMessage);
            return;
        }

        //File selected is executable, update text box
        ui->edit_PreXCompFilename->setText(QString("\"").append(strFilename).append("\""));

        //Update the INI settings
        gpTermSettings->setValue("PrePostXCompPath", ui->edit_PreXCompFilename->text());
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_radio_XCompPre_toggled(
    bool
    )
{
    //Pre/post XCompiler run time changed to run before XCompiler - update settings
    gpTermSettings->setValue("PrePostXCompMode", "0");
}

//=============================================================================
//=============================================================================
void
MainWindow::on_radio_XCompPost_toggled(
    bool
    )
{
    //Pre/post XCompiler run time changed to run after XCompiler - update settings
    gpTermSettings->setValue("PrePostXCompMode", "1");
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_PreXCompFail_stateChanged(
    int
    )
{
    //Pre/post XCompiler run if XCompiler failed changed - update settings
    gpTermSettings->setValue("PrePostXCompFail", (ui->check_PreXCompFail->isChecked() == true ? 1 : 0));
}

//=============================================================================
//=============================================================================
void
MainWindow::on_edit_PreXCompFilename_editingFinished(
    )
{
    //Pre/post XCompiler executable changed - update settings
    gpTermSettings->setValue("PrePostXCompPath", ui->edit_PreXCompFilename->text());
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_GitHub_clicked(
    )
{
    //Open webpage at the UwTerminalX github page)
    if (QDesktopServices::openUrl(QUrl("https://github.com/LairdCP/UwTerminalX")) == false)
    {
        //Failed to open URL
        QString strMessage = tr("An error occured whilst attempting to open a web browser, please ensure you have a web browser installed and configured. URL: https://github.com/LairdCP/UwTerminalX");
        gpmErrorForm->show();
        gpmErrorForm->SetMessage(&strMessage);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::replyFinished(
    QNetworkReply* nrReply
    )
{
    //Response received from online server
    if (nrReply->error() != QNetworkReply::NoError && nrReply->error() != QNetworkReply::ServiceUnavailableError)
    {
        //An error occured
        ui->btn_Cancel->setEnabled(false);
        ui->btn_ErrorCodeUpdate->setEnabled(true);
        ui->btn_ErrorCodeDownload->setEnabled(true);
        ui->btn_UwTerminalXUpdate->setEnabled(true);
        ui->btn_ModuleFirmware->setEnabled(true);
        ui->btn_OnlineXComp_Supported->setEnabled(true);
        gtmrDownloadTimeoutTimer.stop();
        gstrHexData = "";
        if (!gtmrTextUpdateTimer.isActive())
        {
            gtmrTextUpdateTimer.start();
        }
        gchTermMode = 0;
        gchTermMode2 = 0;
        gbTermBusy = false;

        //Display error message if operation wasn't cancelled
        if (nrReply->error() != QNetworkReply::OperationCanceledError)
        {
            //Output error message
            QString strMessage = QString("An error occured during an online request related to XCompilation or updates: ").append(nrReply->errorString());
            gpmErrorForm->show();
            gpmErrorForm->SetMessage(&strMessage);
        }
    }
    else
    {
        if (gchTermMode2 == 0)
        {
            //Check if device is supported
            QJsonParseError jpeJsonError;
            QJsonDocument jdJsonData = QJsonDocument::fromJson(nrReply->readAll(), &jpeJsonError);
            if (jpeJsonError.error == QJsonParseError::NoError)
            {
                //Decoded JSON
                QJsonObject joJsonObject = jdJsonData.object();
                if (nrReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 503)
                {
                    //Server responded with error
                    gtmrDownloadTimeoutTimer.stop();
                    gstrHexData = "";
                    if (!gtmrTextUpdateTimer.isActive())
                    {
                        gtmrTextUpdateTimer.start();
                    }
                    gchTermMode = 0;
                    gchTermMode2 = 0;
                    gbTermBusy = false;
                    ui->btn_Cancel->setEnabled(false);

                    QString strMessage = QString("Server responded with error code ").append(joJsonObject["Result"].toString()).append("; ").append(joJsonObject["Error"].toString());
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                }
                else if (nrReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200)
                {
                    //Server responded with OK
                    if (joJsonObject["Result"].toString() == "1")
                    {
                        //Device supported
                        gstrDeviceID = joJsonObject["ID"].toString();
                        gchTermMode2 = MODE_SERVER_COMPILE;

                        //Compile application
                        if (LookupDNSName() == true)
                        {
                            //Setup the request and add the original application filename
                            QFileInfo fiFileInfo(gstrTermFilename);
                            QNetworkRequest nrThisReq(QUrl(QString(WebProtocol).append("://").append(gstrResolvedServer).append("/xcompile.php?JSON=1")));
                            QByteArray baPostData;
                            baPostData.append("-----------------------------17192614014659\r\nContent-Disposition: form-data; name=\"file_XComp\"\r\n\r\n").append(joJsonObject["ID"].toString()).append("\r\n");
                            baPostData.append(QString("-----------------------------17192614014659\r\nContent-Disposition: form-data; name=\"file_sB\"; filename=\"").append(fiFileInfo.fileName().replace("\"", "")).append("\"\r\nContent-Type: application/octet-stream\r\n\r\n"));

                            //Clear the file data list if it's not empty
                            if (lstFileData.length() > 0)
                            {
                                ClearFileDataList();
                            }

                            //Create and insert starting file into file array
                            FileSStruct *tempFileS = new FileSStruct();
                            tempFileS->strFilename = fiFileInfo.fileName();
                            tempFileS->iStartingLine = 1;
                            tempFileS->iEndingLine = 0;
                            tempFileS->iLineSpaces = 0;
                            lstFileData.append(tempFileS);

                            //Add file data
                            QFile file(gstrTermFilename);
                            QString tmpData;
                            if (!file.open(QIODevice::ReadOnly | QFile::Text))
                            {
                                //Failed to open file selected for download
                                nrReply->deleteLater();
                                gtmrDownloadTimeoutTimer.stop();
                                gstrHexData = "";
                                if (!gtmrTextUpdateTimer.isActive())
                                {
                                    gtmrTextUpdateTimer.start();
                                }
                                gchTermMode = 0;
                                gchTermMode2 = 0;
                                gbTermBusy = false;
                                ui->btn_Cancel->setEnabled(false);

                                QString strMessage = QString("Failed to open file for reading: ").append(gstrTermBusyData);
                                gpmErrorForm->show();
                                gpmErrorForm->SetMessage(&strMessage);

                                return;
                            }

                            //Read all the file's data
                            while (!file.atEnd())
                            {
                                tmpData.append(file.readAll());
                            }
                            file.close();

                            //Include other files
                            QRegularExpression reTempRE("(^|:)(\\s{0,})#(\\s{0,})include(\\s{1,})\"(.*?)\"");
                            reTempRE.setPatternOptions(QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
                            bool bChangedState = true;
                            while (bChangedState == true)
                            {
                                bChangedState = false;
                                QRegularExpressionMatchIterator rx1match = reTempRE.globalMatch(tmpData);
                                while (rx1match.hasNext())
                                {
                                    //Found an include, add the file data
                                    QRegularExpressionMatch ThisMatch = rx1match.next();

                                    file.setFileName(QString(fiFileInfo.path()).append("/").append(ThisMatch.captured(5).replace("\\", "/")));
                                    if (!file.open(QIODevice::ReadOnly | QFile::Text))
                                    {
                                        //Failed to open include file
                                        nrReply->deleteLater();
                                        gtmrDownloadTimeoutTimer.stop();
                                        gstrHexData = "";
                                        if (!gtmrTextUpdateTimer.isActive())
                                        {
                                            gtmrTextUpdateTimer.start();
                                        }
                                        gchTermMode = 0;
                                        gchTermMode2 = 0;
                                        gbTermBusy = false;
                                        ui->btn_Cancel->setEnabled(false);

                                        QString strMessage = QString("Failed to open file for reading: ").append(fiFileInfo.path()).append("/").append(ThisMatch.captured(5).replace("\\", "/"));
                                        gpmErrorForm->show();
                                        gpmErrorForm->SetMessage(&strMessage);
                                        return;
                                    }

                                    //Use a string ref to count the number of lines
                                    QStringRef strrefLines(&tmpData, 0, tmpData.indexOf(ThisMatch.captured(0)));
                                    tempFileS = new FileSStruct();
                                    tempFileS->strFilename = ThisMatch.captured(5);
                                    tempFileS->iStartingLine = strrefLines.count("\n")+2;
                                    tempFileS->iLineSpaces = ThisMatch.captured(0).count("\n");

                                    //Set state to changed to check for next include
                                    bChangedState = true;

                                    //Read all the included file's data
                                    QString tmpData2;
                                    while (!file.atEnd())
                                    {
                                        tmpData2.append(file.readAll());
                                    }
                                    file.close();

                                    //Add the data into the buffer replacing the include statement
                                    unsigned int i = tmpData.indexOf(ThisMatch.captured(0));
                                    tmpData.remove(i, ThisMatch.captured(0).length());
                                    tmpData.insert(i, "\n");
                                    tmpData.insert(i+1, tmpData2);

                                    //Set the ending line of the include file and add it to the list
                                    tempFileS->iEndingLine = tempFileS->iStartingLine + tmpData2.count("\n");
                                    lstFileData.append(tempFileS);
                                }
                            }

                            //Remove all extra #include statments
                            tmpData.replace("#include", "");

                            //Set the total number of lines for the original file
                            lstFileData.at(0)->iEndingLine = tmpData.count("\n")+2;

qDebug() << tmpData;

                            //Append the data to the POST request
                            baPostData.append(tmpData);
                            baPostData.append("\r\n-----------------------------17192614014659--\r\n");
                            nrThisReq.setRawHeader("Content-Type", "multipart/form-data; boundary=---------------------------17192614014659");
                            nrThisReq.setRawHeader("Content-Length", QString(baPostData.length()).toUtf8());
                            gnmrReply = gnmManager->post(nrThisReq, baPostData);
                            ui->statusBar->showMessage("Sending smartBASIC application for online compilation...", 500);
                        }
                        else
                        {
                            //DNS resolution failed
                            gstrHexData = "";
                            if (!gtmrTextUpdateTimer.isActive())
                            {
                                gtmrTextUpdateTimer.start();
                            }
                            gchTermMode = 0;
                            gchTermMode2 = 0;
                            gbTermBusy = false;
                            ui->btn_Cancel->setEnabled(false);
                        }

                        //Stop the module timeout timer
                        gtmrDownloadTimeoutTimer.stop();
                    }
                    else
                    {
                        //Device should be supported but something went wrong...
                        gtmrDownloadTimeoutTimer.stop();
                        gstrHexData = "";
                        if (!gtmrTextUpdateTimer.isActive())
                        {
                            gtmrTextUpdateTimer.start();
                        }
                        gchTermMode = 0;
                        gchTermMode2 = 0;
                        gbTermBusy = false;
                        ui->btn_Cancel->setEnabled(false);

                        QString strMessage = QString("Unfortunately your device is not supported for online XCompiling.");
                        gpmErrorForm->show();
                        gpmErrorForm->SetMessage(&strMessage);
                    }
                }
                else
                {
                    //Unknown response
                    gtmrDownloadTimeoutTimer.stop();
                    gstrHexData = "";
                    if (!gtmrTextUpdateTimer.isActive())
                    {
                        gtmrTextUpdateTimer.start();
                    }
                    gchTermMode = 0;
                    gchTermMode2 = 0;
                    gbTermBusy = false;
                    ui->btn_Cancel->setEnabled(false);

                    QString strMessage = QString("Server responded with unknown response, code: ").append(nrReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                }
            }
            else
            {
                //Error whilst decoding JSON
                gtmrDownloadTimeoutTimer.stop();
                gstrHexData = "";
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
                gchTermMode = 0;
                gchTermMode2 = 0;
                gbTermBusy = false;
                ui->btn_Cancel->setEnabled(false);

                QString strMessage = QString("Error: Unable to decode server JSON response, debug: ").append(jpeJsonError.errorString());
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
            }
            ui->statusBar->showMessage("");
        }
        else if (gchTermMode2 == MODE_SERVER_COMPILE)
        {
            //XCompile result
            if (nrReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 503)
            {
                //Error compiling
                QJsonParseError jpeJsonError;
                QJsonDocument jdJsonData = QJsonDocument::fromJson(nrReply->readAll(), &jpeJsonError);
                gtmrDownloadTimeoutTimer.stop();
                gstrHexData = "";
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
                gchTermMode = 0;
                gchTermMode2 = 0;
                gbTermBusy = false;
                ui->btn_Cancel->setEnabled(false);

                if (jpeJsonError.error == QJsonParseError::NoError)
                {
                    //Decoded JSON
                    QJsonObject joJsonObject = jdJsonData.object();

                    //Server responded with error
                    if (joJsonObject["Result"].toString() == "-9")
                    {
                        //Error whilst compiling, show results
                        QString strMessage = QString("Failed to compile ").append(joJsonObject["Result"].toString()).append("; ").append(joJsonObject["Error"].toString().append("\r\n").append(joJsonObject["Description"].toString()));
                        if (strMessage.indexOf("File   : ") != -1 && strMessage.indexOf("Line   : ") != -1)
                        {
                            //File and line error information available, translate into the proper file
                            qint32 iLineNumber = strMessage.mid(strMessage.indexOf("Line   : ") + 9, strMessage.indexOf("\n", strMessage.indexOf("Line   : "))-strMessage.indexOf("Line   : ")-9).toInt();
                            if (iLineNumber >= 1 && iLineNumber < 200000)
                            {
                                //Line number seems valid, search for the file
/*qint16 ia = lstFileData.length()-1;
while (ia >= 0)
{
//Free up each element and delete it
FileSStruct *tempFileS = lstFileData.at(ia);
qDebug() << tempFileS->strFilename << ": " << tempFileS->iStartingLine << ", " << tempFileS->iEndingLine;
--ia;
}*/
qDebug() << "Error is on #" << iLineNumber;
                                qint16 iCFile = lstFileData.length()-1;
                                qint16 tst = 0;
                                while (iCFile >= 0)
                                {
                                    //Search for the file with the error
                                    FileSStruct *tempFileS = lstFileData.at(iCFile);
qDebug() << "check: " << tempFileS->strFilename << ": " << tempFileS->iStartingLine << ", " << tempFileS->iEndingLine << ", " << tempFileS->iLineSpaces;
                                    //if (tempFileS->iStartingLine >= iLineNumber && tempFileS->iEndingLine <= iLineNumber)
                                    if (tempFileS->iStartingLine <= iLineNumber && tempFileS->iEndingLine >= iLineNumber)
                                    {
                                        //Found the file
                                        qint16 iFPos = strMessage.indexOf("File   : ");
                                        strMessage.remove(strMessage.indexOf("File   : ")-1, strMessage.indexOf("\n", strMessage.indexOf("Line   : "))-strMessage.indexOf("File   : ")+2);
                                        strMessage.insert(iFPos-1, QString("File   : ").append(tempFileS->strFilename).append("\n").append("Line   : ").append(QString::number(iLineNumber - tempFileS->iStartingLine + 1
-(tst > 0 && iCFile == 0 ? tst+1 : 0)
)).append("\n"));
                                        break;
                                    }
                                    else if (tempFileS->iStartingLine <= iLineNumber)
                                    {
                                        //
                                        tst += tempFileS->iEndingLine-tempFileS->iStartingLine - tempFileS->iLineSpaces;
                                    }
                                    --iCFile;
                                }
                            }
                        }
                        gpmErrorForm->show();
                        gpmErrorForm->SetMessage(&strMessage);
                    }
                    else
                    {
                        //Server responded with error
                        QString strMessage = QString("Server responded with error code ").append(joJsonObject["Result"].toString()).append("; ").append(joJsonObject["Error"].toString());
                        gpmErrorForm->show();
                        gpmErrorForm->SetMessage(&strMessage);
                    }
                }
                else
                {
                    //Error whilst decoding JSON
                    QString strMessage = QString("Unable to decode JSON data from server, debug data: ").append(jdJsonData.toBinaryData());
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                }

                //Clear up
                ClearFileDataList();
            }
            else if (nrReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200)
            {
                //Compiled - save file
                QList<QString> lstFI = SplitFilePath(gstrTermFilename);
                if (!QFile::exists(QString(lstFI[0]).append(lstFI[1]).append(".uwc")))
                {
                    //Remove file
                    QFile::remove(QString(lstFI[0]).append(lstFI[1]).append(".uwc"));
                }

                QFile file(QString(lstFI[0]).append(lstFI[1]).append(".uwc"));
                if (file.open(QIODevice::WriteOnly))
                {
                    file.write(nrReply->readAll());
                }
                file.flush();
                file.close();

                gstrTermFilename = QString(lstFI[0]).append(lstFI[1]).append(".uwc");

                if (ui->check_ShowFileSize->isChecked())
                {
                    //Display size
                    gbaDisplayBuffer.append("\n-- XCompile complete (").append(CleanFilesize(gstrTermFilename)).append(") --\n");
                }
                else
                {
                    //Normal message
                    gbaDisplayBuffer.append("\n-- XCompile complete --\n");
                }

                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
                if (gchTermMode == MODE_SERVER_COMPILE)
                {
                    //Done
                    gtmrDownloadTimeoutTimer.stop();
                    gstrHexData = "";
                    gchTermMode = 0;
                    gchTermMode2 = 0;
                    gbTermBusy = false;
                    ui->btn_Cancel->setEnabled(false);
                }
                else
                {
                    //Next step
                    if (gchTermMode == MODE_SERVER_COMPILE_LOAD)
                    {
                        //Just load the file
                        gchTermMode = MODE_LOAD;
                    }
                    else
                    {
                        //Load the file then run it
                        gchTermMode = MODE_LOAD_RUN;
                    }

                    //Loading a compiled application
                    LoadFile(false);

                    //Download to the device
                    gchTermMode2 = MODE_COMPILE;
                    QByteArray baTmpBA = QString("AT+DEL \"").append(gstrDownloadFilename).append("\" +").toUtf8();
                    gspSerialPort.write(baTmpBA);
                    gintQueuedTXBytes += baTmpBA.size();
                    DoLineEnd();
                    gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
                    if (ui->check_SkipDL->isChecked() == false)
                    {
                        //Output download details
                        if (ui->check_ShowCLRF->isChecked() == true)
                        {
                            //Escape \t, \r and \n
                            baTmpBA.replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n");
                        }

                        //Replace unprintable characters
                        baTmpBA.replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f");

                        //Update display buffer
                        gbaDisplayBuffer.append(baTmpBA);
                        if (!gtmrTextUpdateTimer.isActive())
                        {
                            gtmrTextUpdateTimer.start();
                        }
                    }

                    //Start the timeout timer
                    gtmrDownloadTimeoutTimer.start();
                }
            }
            else
            {
                //Unknown response
                gtmrDownloadTimeoutTimer.stop();
                gstrHexData = "";
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
                gchTermMode = 0;
                gchTermMode2 = 0;
                gbTermBusy = false;
                ui->btn_Cancel->setEnabled(false);

                QString strMessage = tr("Unknown response from server.");
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
            }
            ui->statusBar->showMessage("");
        }
        else if (gchTermMode == MODE_CHECK_ERROR_CODE_VERSIONS)
        {
            //Error code update response
            QByteArray baTmpBA = nrReply->readAll();
            QJsonParseError jpeJsonError;
            QJsonDocument jdJsonData = QJsonDocument::fromJson(baTmpBA, &jpeJsonError);

            if (jpeJsonError.error == QJsonParseError::NoError)
            {
                //Decoded JSON
                QJsonObject joJsonObject = jdJsonData.object();

                //Server responded with error
                if (joJsonObject["Result"].toString() == "-1")
                {
                    //Outdated version
                    ui->label_ErrorCodeUpdate->setText("Update is available!");
                    QPalette palBGColour = QPalette();
                    palBGColour.setColor(QPalette::Active, QPalette::WindowText, Qt::darkGreen);
                    palBGColour.setColor(QPalette::Inactive, QPalette::WindowText, Qt::darkGreen);
                    palBGColour.setColor(QPalette::Disabled, QPalette::WindowText, Qt::darkGreen);
                    ui->label_ErrorCodeUpdate->setPalette(palBGColour);
                }
                else if (joJsonObject["Result"].toString() == "-2")
                {
                    //Server error
                    QString strMessage = QString("A server error was encountered whilst checking for an updated error code file.");
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                }
                else if (joJsonObject["Result"].toString() == "1")
                {
                    //Version is OK
                    ui->label_ErrorCodeUpdate->setText("No updates avialable.");
                    QPalette palBGColour = QPalette();
                    ui->label_ErrorCodeUpdate->setPalette(palBGColour);
                }
                else
                {
                    //Server responded with error
                    QString strMessage = QString("Server responded with error code ").append(joJsonObject["Result"].toString()).append("; ").append(joJsonObject["Error"].toString());
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                }
            }
            else
            {
                //Error whilst decoding JSON
                QString strMessage = QString("Unable to decode JSON data from server, debug data: ").append(jdJsonData.toBinaryData());
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
            }

            gchTermMode = 0;
            gchTermMode2 = 0;
            gbTermBusy = false;
            ui->btn_Cancel->setEnabled(false);
            ui->btn_ErrorCodeUpdate->setEnabled(true);
            ui->btn_ErrorCodeDownload->setEnabled(true);
            ui->btn_UwTerminalXUpdate->setEnabled(true);
            ui->btn_ModuleFirmware->setEnabled(true);
            ui->btn_OnlineXComp_Supported->setEnabled(true);
            ui->statusBar->showMessage("");
        }
        else if (gchTermMode == MODE_CHECK_UWTERMINALX_VERSIONS)
        {
            //UwTerminalX update response
            QByteArray baTmpBA = nrReply->readAll();
            QJsonParseError jpeJsonError;
            QJsonDocument jdJsonData = QJsonDocument::fromJson(baTmpBA, &jpeJsonError);

            if (jpeJsonError.error == QJsonParseError::NoError)
            {
                //Decoded JSON
                QJsonObject joJsonObject = jdJsonData.object();

                //Server responded with error
                if (joJsonObject["Result"].toString() == "-1")
                {
                    //Outdated version
                    ui->label_UwTerminalXUpdate->setText(QString("Update available: ").append(joJsonObject["Version"].toString()));
                    QPalette palBGColour = QPalette();
                    palBGColour.setColor(QPalette::Active, QPalette::WindowText, Qt::darkGreen);
                    palBGColour.setColor(QPalette::Inactive, QPalette::WindowText, Qt::darkGreen);
                    palBGColour.setColor(QPalette::Disabled, QPalette::WindowText, Qt::darkGreen);
                    ui->label_UwTerminalXUpdate->setPalette(palBGColour);
                }
                else if (joJsonObject["Result"].toString() == "-2")
                {
                    //Server error
                    QString strMessage = QString("A server error was encountered whilst checking for an updated error code file.");
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                }
                else if (joJsonObject["Result"].toString() == "1")
                {
                    //Version is OK
                    ui->label_UwTerminalXUpdate->setText("No updates avialable.");
                    QPalette palBGColour = QPalette();
                    ui->label_UwTerminalXUpdate->setPalette(palBGColour);
                }
                else
                {
                    //Server responded with error
                    QString strMessage = QString("Server responded with error code ").append(joJsonObject["Result"].toString()).append("; ").append(joJsonObject["Error"].toString());
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                }
            }
            else
            {
                //Error whilst decoding JSON
                QString strMessage = QString("Unable to decode JSON data from server, debug data: ").append(jdJsonData.toBinaryData());
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
            }

            gchTermMode = 0;
            gchTermMode2 = 0;
            gbTermBusy = false;
            ui->btn_Cancel->setEnabled(false);
            ui->btn_ErrorCodeUpdate->setEnabled(true);
            ui->btn_ErrorCodeDownload->setEnabled(true);
            ui->btn_UwTerminalXUpdate->setEnabled(true);
            ui->btn_ModuleFirmware->setEnabled(true);
            ui->btn_OnlineXComp_Supported->setEnabled(true);
            ui->statusBar->showMessage("");
        }
        else if (gchTermMode == MODE_UPDATE_ERROR_CODE)
        {
            //Error code update
            if (nrReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200)
            {
                //Got updated error code file
#ifndef SKIPERRORCODEFORM
                if (gecErrorCodeForm != 0)
                {
                    //Clear error code file
                    gecErrorCodeForm->SetErrorObject(0);
                }
#endif
                delete gpErrorMessages;
                gbErrorsLoaded = false;
#if TARGET_OS_MAC
                if (QFile::exists(QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/codes.csv")))
                {
                    //Remove file
                    QFile::remove(QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/codes.csv"));
                }
#else
                if (QFile::exists("codes.csv"))
                {
                    //Remove file
                    QFile::remove("codes.csv");
                }
#endif

#if TARGET_OS_MAC
                QFile file(QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/codes.csv"));
#else
                QFile file("codes.csv");
#endif
                if (file.open(QIODevice::WriteOnly))
                {
                    file.write(nrReply->readAll());
                    file.flush();
                    file.close();

                    //Reopen error code file and update status
#if TARGET_OS_MAC
                    gpErrorMessages = new QSettings(QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/codes.csv"), QSettings::IniFormat);
#else
                    gpErrorMessages = new QSettings(QString("codes.csv"), QSettings::IniFormat);
#endif
                    ui->label_ErrorCodeUpdate->setText("Error code file updated!");
                    gbErrorsLoaded = true;

#ifndef SKIPERRORCODEFORM
                    if (gecErrorCodeForm != 0)
                    {
                        //Update error code form
                        gecErrorCodeForm->SetErrorObject(gpErrorMessages);
                    }
#endif
                }
                else
                {
                    //Failed to open error code file
                    QString strMessage = tr("Failed to open error code file for writing (codes.csv), ensure relevant permissions exist for this file and try again.");
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                }
            }
            else
            {
                //Unknown response
                QString strMessage = tr("Unknown response from server.");
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
            }

            //Reset everything
            gchTermMode = 0;
            gchTermMode2 = 0;
            gbTermBusy = false;
            ui->btn_Cancel->setEnabled(false);
            ui->btn_ErrorCodeUpdate->setEnabled(true);
            ui->btn_ErrorCodeDownload->setEnabled(true);
            ui->btn_UwTerminalXUpdate->setEnabled(true);
            ui->btn_ModuleFirmware->setEnabled(true);
            ui->btn_OnlineXComp_Supported->setEnabled(true);
            ui->statusBar->showMessage("");
        }
        else if (gchTermMode == MODE_CHECK_FIRMWARE_VERSIONS)
        {
            //Response containing latest firmware versions for modules
            QByteArray baTmpBA = nrReply->readAll();
            QJsonParseError jpeJsonError;
            QJsonDocument jdJsonData = QJsonDocument::fromJson(baTmpBA, &jpeJsonError);

            if (jpeJsonError.error == QJsonParseError::NoError)
            {
                //Decoded JSON
                QJsonObject joJsonObject = jdJsonData.object();

                if (joJsonObject["Result"].toString() == "1")
                {
                    //Update version list
                    QString strTmpStr;
                    foreach (const QString &strKey, joJsonObject.keys())
                    {
                        if (strKey != "Result")
                        {
                            //Firmware version
                            strTmpStr.append(strKey).append(": ").append(joJsonObject[strKey].toString()).append("\r\n");
                        }
                    }
                    strTmpStr.remove(strTmpStr.length()-2, 2);
                    ui->label_Firmware->setText(strTmpStr);

                    //Change colour to green
                    QPalette palBGColour = QPalette();
                    palBGColour.setColor(QPalette::Active, QPalette::WindowText, Qt::darkGreen);
                    palBGColour.setColor(QPalette::Inactive, QPalette::WindowText, Qt::darkGreen);
                    palBGColour.setColor(QPalette::Disabled, QPalette::WindowText, Qt::darkGreen);
                    ui->label_Firmware->setPalette(palBGColour);
                }
                else
                {
                    //Server responded with error
                    QString strMessage = QString("Server responded with error code ").append(joJsonObject["Result"].toString()).append("; ").append(joJsonObject["Error"].toString());
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                }
            }
            else
            {
                //Error whilst decoding JSON
                QString strMessage = QString("Unable to decode JSON data from server, debug data: ").append(jdJsonData.toBinaryData());
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
            }

            gchTermMode = 0;
            gchTermMode2 = 0;
            gbTermBusy = false;
            ui->btn_Cancel->setEnabled(false);
            ui->btn_ErrorCodeUpdate->setEnabled(true);
            ui->btn_ErrorCodeDownload->setEnabled(true);
            ui->btn_UwTerminalXUpdate->setEnabled(true);
            ui->btn_ModuleFirmware->setEnabled(true);
            ui->btn_OnlineXComp_Supported->setEnabled(true);
            ui->statusBar->showMessage("");
        }
        else if (gchTermMode == MODE_CHECK_FIRMWARE_SUPPORT)
        {
            //Supported XCompiler response
            QString strMessage = QString("Supported devices and firmwares using Online XCompiler: \n\n").append(nrReply->readAll()).append("\n\nAs more devices and firmwares are released, these new versions will be added to the Online XCompile server.");
            gpmErrorForm->show();
            gpmErrorForm->SetMessage(&strMessage);

            //
            gchTermMode = 0;
            gchTermMode2 = 0;
            gbTermBusy = false;
            ui->btn_Cancel->setEnabled(false);
            ui->btn_ErrorCodeUpdate->setEnabled(true);
            ui->btn_ErrorCodeDownload->setEnabled(true);
            ui->btn_UwTerminalXUpdate->setEnabled(true);
            ui->btn_ModuleFirmware->setEnabled(true);
            ui->btn_OnlineXComp_Supported->setEnabled(true);
            ui->statusBar->showMessage("");
        }
    }

    //Queue the network reply object to be deleted
    nrReply->deleteLater();
}

//=============================================================================
//=============================================================================
#ifdef UseSSL
void
MainWindow::sslErrors(
    QNetworkReply* nrReply,
    QList<QSslError> lstSSLErrors
    )
{
    //Error detected with SSL
    if (sslcLairdSSLNew != NULL && nrReply->sslConfiguration().peerCertificate() == *sslcLairdSSLNew)
    {
        //Server certificate matches
        nrReply->ignoreSslErrors(lstSSLErrors);
    }
}
#endif

//=============================================================================
//=============================================================================
void
MainWindow::on_check_OnlineXComp_stateChanged(
    int
    )
{
    //Online XCompiler checkbox state changed
    ui->label_OnlineXCompInfo->setEnabled(ui->check_OnlineXComp->isChecked());
    gpTermSettings->setValue("OnlineXComp", (ui->check_OnlineXComp->isChecked() == true ? 1 : 0));
}

//=============================================================================
//=============================================================================
QList<QString>
MainWindow::SplitFilePath(
    QString strFilename
    )
{
    //Extracts various parts from a file path; [0] path, [1] filename, [2] file extension
    QFileInfo fiFile(strFilename);
    QString strFilenameOnly = fiFile.fileName();
    QString strFileExtension = "";
    if (strFilenameOnly.indexOf(".") != -1)
    {
        //Dot found, only keep characters up to the dot
        strFileExtension = strFilenameOnly.mid(strFilenameOnly.lastIndexOf(".")+1, -1);
        strFilenameOnly = strFilenameOnly.left(strFilenameOnly.lastIndexOf("."));
    }

    //Return an array with path, filename and extension
    QList<QString> lstReturnData;
    lstReturnData << fiFile.path().append("/") << strFilenameOnly << strFileExtension;
    return lstReturnData;
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_ErrorCodeUpdate_clicked(
    )
{
    //Check for updates to error codes
    if (gbTermBusy == false)
    {
        //Send request
        if (LookupDNSName() == true)
        {
            gbTermBusy = true;
            gchTermMode = MODE_CHECK_ERROR_CODE_VERSIONS;
            gchTermMode2 = MODE_CHECK_ERROR_CODE_VERSIONS;
            ui->btn_Cancel->setEnabled(true);
            ui->btn_ErrorCodeUpdate->setEnabled(false);
            ui->btn_ErrorCodeDownload->setEnabled(false);
            ui->btn_UwTerminalXUpdate->setEnabled(false);
            ui->btn_ModuleFirmware->setEnabled(false);
            ui->btn_OnlineXComp_Supported->setEnabled(false);
            gnmrReply = gnmManager->get(QNetworkRequest(QUrl(QString(WebProtocol).append("://").append(gstrResolvedServer).append("/update_errorcodes.php?Ver=").append(gpErrorMessages->value("Version", "0.00").toString()))));
            ui->statusBar->showMessage("Checking for Error Code file updates...");
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_UwTerminalXUpdate_clicked(
    )
{
    //Check for updates to UwTerminalX
    if (gbTermBusy == false)
    {
        //Send request
        if (LookupDNSName() == true)
        {
            gbTermBusy = true;
            gchTermMode = MODE_CHECK_UWTERMINALX_VERSIONS;
            gchTermMode2 = MODE_CHECK_UWTERMINALX_VERSIONS;
            ui->btn_Cancel->setEnabled(true);
            ui->btn_ErrorCodeUpdate->setEnabled(false);
            ui->btn_ErrorCodeDownload->setEnabled(false);
            ui->btn_UwTerminalXUpdate->setEnabled(false);
            ui->btn_ModuleFirmware->setEnabled(false);
            ui->btn_OnlineXComp_Supported->setEnabled(false);
            gnmrReply = gnmManager->get(QNetworkRequest(QUrl(QString(WebProtocol).append("://").append(gstrResolvedServer).append("/update_uwterminalx.php?Ver=").append(UwVersion))));
            ui->statusBar->showMessage("Checking for UwTerminalX updates...");
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_Echo_stateChanged(
    int
    )
{
    //Local echo checkbox state changed
    ui->text_TermEditData->mbLocalEcho = ui->check_Echo->isChecked();
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_ErrorCodeDownload_clicked(
    )
{
    //Download latest error code file button clicked
    if (gbTermBusy == false)
    {
        //Send request
        if (LookupDNSName() == true)
        {
            gbTermBusy = true;
            gchTermMode = MODE_UPDATE_ERROR_CODE;
            gchTermMode2 = MODE_UPDATE_ERROR_CODE;
            ui->btn_Cancel->setEnabled(true);
            ui->btn_ErrorCodeUpdate->setEnabled(false);
            ui->btn_ErrorCodeDownload->setEnabled(false);
            ui->btn_UwTerminalXUpdate->setEnabled(false);
            ui->btn_ModuleFirmware->setEnabled(false);
            ui->btn_OnlineXComp_Supported->setEnabled(false);
            gnmrReply = gnmManager->get(QNetworkRequest(QUrl(QString(WebProtocol).append("://").append(gstrResolvedServer).append("/codes.csv"))));
            ui->statusBar->showMessage("Downloading Error Code file...");
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_combo_PredefinedDevice_currentIndexChanged(
    int intIndex
    )
{
    //Load settings for current device
    ui->combo_Baud->setCurrentText(gpPredefinedDevice->value(QString("Port").append(QString::number(intIndex+1).append("Baud")), "115200").toString());
    ui->combo_Parity->setCurrentIndex(gpPredefinedDevice->value(QString("Port").append(QString::number(intIndex+1).append("Parity")), "0").toInt());
    ui->combo_Stop->setCurrentText(gpPredefinedDevice->value(QString("Port").append(QString::number(intIndex+1).append("Stop")), "1").toString());
    ui->combo_Data->setCurrentText(gpPredefinedDevice->value(QString("Port").append(QString::number(intIndex+1).append("Data")), "8").toString());
    ui->combo_Handshake->setCurrentIndex(gpPredefinedDevice->value(QString("Port").append(QString::number(intIndex+1).append("Flow")), "1").toInt());
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_PredefinedAdd_clicked(
    )
{
    //Adds a new predefined device entry
    ui->combo_PredefinedDevice->addItem("New");
    ui->combo_PredefinedDevice->setCurrentIndex(ui->combo_PredefinedDevice->count()-1);
    gpPredefinedDevice->setValue(QString("Port").append(QString::number((ui->combo_PredefinedDevice->count()))).append("Name"), "New");
    gpPredefinedDevice->setValue(QString("Port").append(QString::number((ui->combo_PredefinedDevice->count()))).append("Baud"), ui->combo_Baud->currentText());
    gpPredefinedDevice->setValue(QString("Port").append(QString::number((ui->combo_PredefinedDevice->count()))).append("Parity"), ui->combo_Parity->currentIndex());
    gpPredefinedDevice->setValue(QString("Port").append(QString::number((ui->combo_PredefinedDevice->count()))).append("Stop"), ui->combo_Stop->currentText());
    gpPredefinedDevice->setValue(QString("Port").append(QString::number((ui->combo_PredefinedDevice->count()))).append("Data"), ui->combo_Data->currentText());
    gpPredefinedDevice->setValue(QString("Port").append(QString::number((ui->combo_PredefinedDevice->count()))).append("Flow"), ui->combo_Handshake->currentIndex());
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_PredefinedDelete_clicked(
    )
{
    //Remove current device configuration
    if (ui->combo_PredefinedDevice->count() > 0)
    {
        //Item exists, delete selected item
        unsigned int uiDeviceNumber = ui->combo_PredefinedDevice->currentIndex();
        unsigned int i = uiDeviceNumber+2;
        gpPredefinedDevice->remove(QString("Port").append(QString::number(i-1)).append("Name"));
        gpPredefinedDevice->remove(QString("Port").append(QString::number(i-1)).append("Baud"));
        gpPredefinedDevice->remove(QString("Port").append(QString::number(i-1)).append("Parity"));
        gpPredefinedDevice->remove(QString("Port").append(QString::number(i-1)).append("Stop"));
        gpPredefinedDevice->remove(QString("Port").append(QString::number(i-1)).append("Data"));
        gpPredefinedDevice->remove(QString("Port").append(QString::number(i-1)).append("Flow"));
        while (!gpPredefinedDevice->value(QString("Port").append(QString::number(i)).append("Name")).isNull())
        {
            //Shift element up
            gpPredefinedDevice->setValue(QString("Port").append(QString::number(i-1)).append("Name"), gpPredefinedDevice->value(QString("Port").append(QString::number(i)).append("Name")).toString());
            gpPredefinedDevice->setValue(QString("Port").append(QString::number(i-1)).append("Baud"), gpPredefinedDevice->value(QString("Port").append(QString::number(i)).append("Baud")).toInt());
            gpPredefinedDevice->setValue(QString("Port").append(QString::number(i-1)).append("Parity"), gpPredefinedDevice->value(QString("Port").append(QString::number(i)).append("Parity")).toInt());
            gpPredefinedDevice->setValue(QString("Port").append(QString::number(i-1)).append("Stop"), gpPredefinedDevice->value(QString("Port").append(QString::number(i)).append("Stop")).toInt());
            gpPredefinedDevice->setValue(QString("Port").append(QString::number(i-1)).append("Data"), gpPredefinedDevice->value(QString("Port").append(QString::number(i)).append("Data")).toInt());
            gpPredefinedDevice->setValue(QString("Port").append(QString::number(i-1)).append("Flow"), gpPredefinedDevice->value(QString("Port").append(QString::number(i)).append("Flow")).toInt());
            ++i;
        }
        if (!gpPredefinedDevice->value(QString("Port").append(QString::number(i-1)).append("Name")).isNull())
        {
            //Remove last element
            gpPredefinedDevice->remove(QString("Port").append(QString::number(i-1)).append("Name"));
            gpPredefinedDevice->remove(QString("Port").append(QString::number(i-1)).append("Baud"));
            gpPredefinedDevice->remove(QString("Port").append(QString::number(i-1)).append("Parity"));
            gpPredefinedDevice->remove(QString("Port").append(QString::number(i-1)).append("Stop"));
            gpPredefinedDevice->remove(QString("Port").append(QString::number(i-1)).append("Data"));
            gpPredefinedDevice->remove(QString("Port").append(QString::number(i-1)).append("Flow"));
        }
        ui->combo_PredefinedDevice->removeItem(uiDeviceNumber);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::DroppedFile(
    QString strFilename
    )
{
    //File dragged for download
    if (gbTermBusy == false)
    {
        //Terminal not busy. Check file extension
        if (strFilename.right(3).toLower() == ".sb")
        {
            //smartBASIC source file - compile
            gchTermMode = MODE_COMPILE_LOAD;
            gstrTermFilename = strFilename;

            //Get the version number
            gbTermBusy = true;
            gchTermMode2 = 0;
            gchTermBusyLines = 0;
            gstrTermBusyData = tr("");
            if (ui->check_CheckLicense->isChecked())
            {
                //Check license
                gspSerialPort.write("at i 4");
                gintQueuedTXBytes += 6;
                DoLineEnd();
                gpMainLog->WriteLogData("at i 4\n");
            }
            gspSerialPort.write("at i 0");
            gintQueuedTXBytes += 6;
            DoLineEnd();
            gpMainLog->WriteLogData("at i 0\n");
            gspSerialPort.write("at i 13");
            gintQueuedTXBytes += 7;
            DoLineEnd();
            gpMainLog->WriteLogData("at i 13\n");
        }
        else
        {
            //Normal download
            gchTermMode = MODE_LOAD;
            gstrTermFilename = strFilename;
            gbTermBusy = true;
            LoadFile(false);

            //Download to the device
            gchTermMode2 = 1;
            QByteArray baTmpBA = QString("AT+DEL \"").append(gstrDownloadFilename).append("\" +").toUtf8();
            gspSerialPort.write(baTmpBA);
            gintQueuedTXBytes += baTmpBA.size();
            DoLineEnd();
            gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
            if (ui->check_SkipDL->isChecked() == false)
            {
                //Output download details
                if (ui->check_ShowCLRF->isChecked() == true)
                {
                    //Escape \t, \r and \n
                    baTmpBA.replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n");
                }

                //Replace unprintable characters
                baTmpBA.replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f");

                //Update display buffer
                gbaDisplayBuffer.append(baTmpBA);
                if (!gtmrTextUpdateTimer.isActive())
                {
                    gtmrTextUpdateTimer.start();
                }
            }
        }

        //Start the timeout timer
        gtmrDownloadTimeoutTimer.start();

        //Enable cancel button
        ui->btn_Cancel->setEnabled(true);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_SaveDevice_clicked(
    )
{
    //Saves changes to a configuration
    ui->combo_PredefinedDevice->setItemText(ui->combo_PredefinedDevice->currentIndex(), ui->combo_PredefinedDevice->currentText());
    gpPredefinedDevice->setValue(QString("Port").append(QString::number(((ui->combo_PredefinedDevice->currentIndex()+1)))).append("Name"), ui->combo_PredefinedDevice->currentText());
    gpPredefinedDevice->setValue(QString("Port").append(QString::number(((ui->combo_PredefinedDevice->currentIndex()+1)))).append("Baud"), ui->combo_Baud->currentText());
    gpPredefinedDevice->setValue(QString("Port").append(QString::number(((ui->combo_PredefinedDevice->currentIndex()+1)))).append("Parity"), ui->combo_Parity->currentIndex());
    gpPredefinedDevice->setValue(QString("Port").append(QString::number(((ui->combo_PredefinedDevice->currentIndex()+1)))).append("Stop"), ui->combo_Stop->currentText());
    gpPredefinedDevice->setValue(QString("Port").append(QString::number(((ui->combo_PredefinedDevice->currentIndex()+1)))).append("Data"), ui->combo_Data->currentText());
    gpPredefinedDevice->setValue(QString("Port").append(QString::number(((ui->combo_PredefinedDevice->currentIndex()+1)))).append("Flow"), ui->combo_Handshake->currentIndex());
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_ModuleFirmware_clicked(
    )
{
    //Checks what the latest module firmware versions are
    if (gbTermBusy == false)
    {
        //Send request
        if (LookupDNSName() == true)
        {
            gbTermBusy = true;
            gchTermMode = MODE_CHECK_FIRMWARE_VERSIONS;
            gchTermMode2 = MODE_CHECK_FIRMWARE_VERSIONS;
            ui->btn_Cancel->setEnabled(true);
            ui->btn_ErrorCodeUpdate->setEnabled(false);
            ui->btn_ErrorCodeDownload->setEnabled(false);
            ui->btn_UwTerminalXUpdate->setEnabled(false);
            ui->btn_ModuleFirmware->setEnabled(false);
            ui->btn_OnlineXComp_Supported->setEnabled(false);
            gnmrReply = gnmManager->get(QNetworkRequest(QUrl(QString(WebProtocol).append("://").append(gstrResolvedServer).append("/firmwares.php"))));
            ui->statusBar->showMessage("Checking for latest firmware versions...");
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::ContextMenuClosed(
    )
{
    //Right click context menu closed, send message to text edit object
    ui->text_TermEditData->mbContextMenuOpen = false;
    ui->text_TermEditData->UpdateDisplay();
}

//=============================================================================
//=============================================================================
bool
MainWindow::event(
    QEvent *evtEvent
    )
{
    if (evtEvent->type() == QEvent::WindowActivate && gspSerialPort.isOpen() == true && ui->selector_Tab->currentIndex() == TabTerminal)
    {
        //Focus on the terminal
        ui->text_TermEditData->setFocus();
    }
    return QMainWindow::event(evtEvent);
}

//=============================================================================
//=============================================================================
void
MainWindow::SerialPortClosing(
    )
{
    //Called when the serial port is closing
    ui->image_CTS->setPixmap(*gpEmptyCirclePixmap);
    ui->image_DCD->setPixmap(*gpEmptyCirclePixmap);
    ui->image_DSR->setPixmap(*gpEmptyCirclePixmap);
    ui->image_RI->setPixmap(*gpEmptyCirclePixmap);

    ui->image_CTSb->setPixmap(*gpEmptyCirclePixmap);
    ui->image_DCDb->setPixmap(*gpEmptyCirclePixmap);
    ui->image_DSRb->setPixmap(*gpEmptyCirclePixmap);
    ui->image_RIb->setPixmap(*gpEmptyCirclePixmap);

#ifndef SKIPSCRIPTINGFORM
    if (gusScriptingForm != 0)
    {
        //Notify scripting form
        gusScriptingForm->SerialPortStatus(false);
    }
#endif
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_LogFileSelect_clicked(
    )
{
    //Updates the log file
#ifndef SKIPAUTOMATIONFORM
    if (guaAutomationForm != 0)
    {
        guaAutomationForm->TempAlwaysOnTop(0);
    }
#endif
    QString strLogFilename = QFileDialog::getSaveFileName(this, "Select Log File", ui->edit_LogFile->text(), "Log Files (*.log);;All Files (*.*)");
#ifndef SKIPAUTOMATIONFORM
    if (guaAutomationForm != 0)
    {
        guaAutomationForm->TempAlwaysOnTop(1);
    }
#endif
    if (!strLogFilename.isEmpty())
    {
        //Update log file
        ui->edit_LogFile->setText(strLogFilename);
        on_edit_LogFile_editingFinished();
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_edit_LogFile_editingFinished(
    )
{
    //Log filename has changed
    gpTermSettings->setValue("LogFile", ui->edit_LogFile->text());
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_LogEnable_stateChanged(
    int
    )
{
    //Logging enabled/disabled changed
    gpTermSettings->setValue("LogEnable", (ui->check_LogEnable->isChecked() == true ? 1 : 0));
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_LogAppend_stateChanged(
    int
    )
{
    //Logging append/clearing changed
    gpTermSettings->setValue("LogMode", (ui->check_LogAppend->isChecked() == true ? 1 : 0));
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Help_clicked(
    )
{
    //Opens the help PDF file
#ifdef __APPLE__
    if (QFile::exists(QString(gstrMacBundlePath).append("Help.pdf")))
#else
    if (QFile::exists("Help.pdf"))
#endif
    {
        //File present - open
#ifdef __APPLE__
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString(gstrMacBundlePath).append("Help.pdf")));
#else
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo("Help.pdf").absoluteFilePath()));
#endif
    }
    else
    {
        //File not present, open on website instead
        if (QDesktopServices::openUrl(QUrl(QString("http://").append(ServerHost).append("/uwterminalx_help.pdf"))) == false)
        {
            //Failed to open
            QString strMessage = tr("Help file (Help.pdf) was not found and an error occured whilst attempting to open the online version.");
            gpmErrorForm->show();
            gpmErrorForm->SetMessage(&strMessage);
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_combo_LogDirectory_currentIndexChanged(
    int
    )
{
    //Refresh the list of log files
    on_btn_LogRefresh_clicked();
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_LogRefresh_clicked(
    )
{
    //Refreshes the log files availanble for viewing
    ui->combo_LogFile->clear();
    ui->combo_LogFile->addItem("- No file selected -");
    QString strDirPath;
    if (ui->combo_LogDirectory->currentIndex() == 1)
    {
        //Log file directory
#if TARGET_OS_MAC
        QFileInfo a(QString((ui->edit_LogFile->text().left(1) == "/" || ui->edit_LogFile->text().left(1) == "\\") ? "" : QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/")).append(ui->edit_LogFile->text()));
#else
        QFileInfo a(ui->edit_LogFile->text());
#endif
        strDirPath = a.absolutePath();
    }
    else
    {
        //Application directory
#if TARGET_OS_MAC
        strDirPath = QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/");
#else
        strDirPath = "./";
#endif
    }

    //Apply file filters
    QDir dirLogDir(strDirPath);
    QFileInfoList filFiles;
    filFiles = dirLogDir.entryInfoList(QStringList() << "*.log");
    if (filFiles.count() > 0)
    {
        //At least one file was found
        int i = 0;
        while (i < filFiles.count())
        {
            //List all files
            ui->combo_LogFile->addItem(filFiles.at(i).fileName());
            ++i;
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Licenses_clicked(
    )
{
    //Show license text
    QString strMessage = tr("UwTerminalX uses the Qt framework version 5, which is licensed under the GPLv3 (not including later versions).\nUwTerminalX uses and may be linked statically to various other libraries including Xau, XCB, expat, fontconfig, zlib, bz2, harfbuzz, freetype, udev, dbus, icu, unicode, UPX, OpenSSL. The licenses for these libraries are provided below:\n\n\n\nLib Xau:\n\nCopyright 1988, 1993, 1994, 1998  The Open Group\n\nPermission to use, copy, modify, distribute, and sell this software and its\ndocumentation for any purpose is hereby granted without fee, provided that\nthe above copyright notice appear in all copies and that both that\ncopyright notice and this permission notice appear in supporting\ndocumentation.\nThe above copyright notice and this permission notice shall be included in\nall copies or substantial portions of the Software.\nTHE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\nIMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\nFITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE\nOPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN\nAN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN\nCONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n\nExcept as contained in this notice, the name of The Open Group shall not be\nused in advertising or otherwise to promote the sale, use or other dealings\nin this Software without prior written authorization from The Open Group.\n\n\n\nxcb:\n\nCopyright (C) 2001-2006 Bart Massey, Jamey Sharp, and Josh Triplett.\nAll Rights Reserved.\n\nPermission is hereby granted, free of charge, to any person\nobtaining a copy of this software and associated\ndocumentation files (the 'Software'), to deal in the\nSoftware without restriction, including without limitation\nthe rights to use, copy, modify, merge, publish, distribute,\nsublicense, and/or sell copies of the Software, and to\npermit persons to whom the Software is furnished to do so,\nsubject to the following conditions:\n\nThe above copyright notice and this permission notice shall\nbe included in all copies or substantial portions of the\nSoftware.\n\nTHE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY\nKIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE\nWARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR\nPURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS\nBE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER\nIN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\nOUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR\nOTHER DEALINGS IN THE SOFTWARE.\n\nExcept as contained in this notice, the names of the authors\nor their institutions shall not be used in advertising or\notherwise to promote the sale, use or other dealings in this\nSoftware without prior written authorization from the\nauthors.\n\n\n\nexpat:\n\nCopyright (c) 1998, 1999, 2000 Thai Open Source Software Center Ltd\n   and Clark Cooper\nCopyright (c) 2001, 2002, 2003, 2004, 2005, 2006 Expat maintainers.\nPermission is hereby granted, free of charge, to any person obtaining\na copy of this software and associated documentation files (the\n'Software'), to deal in the Software without restriction, including\nwithout limitation the rights to use, copy, modify, merge, publish,\ndistribute, sublicense, and/or sell copies of the Software, and to\npermit persons to whom the Software is furnished to do so, subject to\nthe following conditions:\n\nThe above copyright notice and this permission notice shall be included\nin all copies or substantial portions of the Software.\n\nTHE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,\nEXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\nMERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\nIN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY\nCLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,\nTORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE\nSOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n\n\n\nfontconfig:\n\nCopyright © 2001,2003 Keith Packard\n\nPermission to use, copy, modify, distribute, and sell this software and its\ndocumentation for any purpose is hereby granted without fee, provided that\nthe above copyright notice appear in all copies and that both that\ncopyright notice and this permission notice appear in supporting\ndocumentation, and that the name of Keith Packard not be used in\nadvertising or publicity pertaining to distribution of the software without\nspecific, written prior permission.  Keith Packard makes no\nrepresentations about the suitability of this software for any purpose.  It\nis provided 'as is' without express or implied warranty.\n\nKEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,\nINCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO\nEVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR\nCONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,\nDATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER\nTORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR\nPERFORMANCE OF THIS SOFTWARE.\n\nz:\n\n (C) 1995-2013 Jean-loup Gailly and Mark Adler\n\n  This software is provided 'as-is', without any express or implied\n  warranty.  In no event will the authors be held liable for any damages\n  arising from the use of this software.\n\n  Permission is granted to anyone to use this software for any purpose,\n  including commercial applications, and to alter it and redistribute it\n  freely, subject to the following restrictions:\n\n  1. The origin of this software must not be misrepresented; you must not\n     claim that you wrote the original software. If you use this software\n     in a product, an acknowledgment in the product documentation would be\n     appreciated but is not required.\n  2. Altered source versions must be plainly marked as such, and must not be\n     misrepresented as being the original software.\n  3. This notice may not be removed or altered from any source distribution.\n\n  Jean-loup Gailly        Mark Adler\n  jloup@gzip.org          madler@alumni.caltech.edu\n\n\n\nbz2:\n\n\nThis program, 'bzip2', the associated library 'libbzip2', and all\ndocumentation, are copyright (C) 1996-2010 Julian R Seward.  All\nrights reserved.\n\nRedistribution and use in source and binary forms, with or without\nmodification, are permitted provided that the following conditions\nare met:\n\n1. Redistributions of source code must retain the above copyright\n   notice, this list of conditions and the following disclaimer.\n\n2. The origin of this software must not be misrepresented; you must\n   not claim that you wrote the original software.  If you use this\n   software in a product, an acknowledgment in the product\n   documentation would be appreciated but is not required.\n\n3. Altered source versions must be plainly marked as such, and must\n   not be misrepresented as being the original software.\n\n4. The name of the author may not be used to endorse or promote\n   products derived from this software without specific prior written\n   permission.\n\nTHIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS\nOR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\nWARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\nARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY\nDIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\nDAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE\nGOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS\nINTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,\nWHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING\nNEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\nSOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\nJulian Seward, jseward@bzip.org\nbzip2/libbzip2 version 1.0.6 of 6 September 2010\n\n\n\nharfbuzz:\n\nHarfBuzz is licensed under the so-called 'Old MIT' license.  Details follow.\n\nCopyright © 2010,2011,2012  Google, Inc.\nCopyright © 2012  Mozilla Foundation\nCopyright © 2011  Codethink Limited\nCopyright © 2008,2010  Nokia Corporation and/or its subsidiary(-ies)\nCopyright © 2009  Keith Stribley\nCopyright © 2009  Martin Hosken and SIL International\nCopyright © 2007  Chris Wilson\nCopyright © 2006  Behdad Esfahbod\nCopyright © 2005  David Turner\nCopyright © 2004,2007,2008,2009,2010  Red Hat, Inc.\nCopyright © 1998-2004  David Turner and Werner Lemberg\n\nFor full copyright notices consult the individual files in the package.\n\nPermission is hereby granted, without written agreement and without\nlicense or royalty fees, to use, copy, modify, and distribute this\nsoftware and its documentation for any purpose, provided that the\nabove copyright notice and the").append(" following two paragraphs appear in\nall copies of this software.\n\nIN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR\nDIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES\nARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN\nIF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH\nDAMAGE.\n\nTHE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,\nBUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND\nFITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS\nON AN 'AS IS' BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO\nPROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.\n\n\n\nfreetype:\n\nThe  FreeType 2  font  engine is  copyrighted  work and  cannot be  used\nlegally  without a  software license.   In  order to  make this  project\nusable  to a vast  majority of  developers, we  distribute it  under two\nmutually exclusive open-source licenses.\n\nThis means  that *you* must choose  *one* of the  two licenses described\nbelow, then obey  all its terms and conditions when  using FreeType 2 in\nany of your projects or products.\n\n  - The FreeType License, found in  the file `FTL.TXT', which is similar\n    to the original BSD license *with* an advertising clause that forces\n    you  to  explicitly cite  the  FreeType  project  in your  product's\n    documentation.  All  details are in the license  file.  This license\n    is  suited  to products  which  don't  use  the GNU  General  Public\n    License.\n\n    Note that  this license  is  compatible  to the  GNU General  Public\n    License version 3, but not version 2.\n\n  - The GNU General Public License version 2, found in  `GPLv2.TXT' (any\n    later version can be used  also), for programs which already use the\n    GPL.  Note  that the  FTL is  incompatible  with  GPLv2 due  to  its\n    advertisement clause.\n\nThe contributed BDF and PCF drivers come with a license similar  to that\nof the X Window System.  It is compatible to the above two licenses (see\nfile src/bdf/README and src/pcf/README).\n\nThe gzip module uses the zlib license (see src/gzip/zlib.h) which too is\ncompatible to the above two licenses.\n\nThe MD5 checksum support (only used for debugging in development builds)\nis in the public domain.\n\n\n\nudev:\n\nCopyright (C) 2003 Greg Kroah-Hartman <greg@kroah.com>\nCopyright (C) 2003-2010 Kay Sievers <kay@vrfy.org>\n\nThis program is free software: you can redistribute it and/or modify\nit under the terms of the GNU General Public License as published by\nthe Free Software Foundation, either version 2 of the License, or\n(at your option) any later version.\n\nThis program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License\nalong with this program.  If not, see <http://www.gnu.org/licenses/>.\n\n\n\ndbus:\n\nD-Bus is licensed to you under your choice of the Academic Free\nLicense version 2.1, or the GNU General Public License version 2\n(or, at your option any later version).\n\n\n\nicu:\n\nICU License - ICU 1.8.1 and later\nCOPYRIGHT AND PERMISSION NOTICE\nCopyright (c) 1995-2015 International Business Machines Corporation and others\nAll rights reserved.\nPermission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the 'Software'), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, provided that the above copyright notice(s) and this permission notice appear in all copies of the Software and that both the above copyright notice(s) and this permission notice appear in supporting documentation.\nTHE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.\nExcept as contained in this notice, the name of a copyright holder shall not be used in advertising or otherwise to promote the sale, use or other dealings in this Software without prior written authorization of the copyright holder.\n\n\n\nUnicode:\n\nCOPYRIGHT AND PERMISSION NOTICE\n\nCopyright © 1991-2015 Unicode, Inc. All rights reserved.\nDistributed under the Terms of Use in\nhttp://www.unicode.org/copyright.html.\n\nPermission is hereby granted, free of charge, to any person obtaining\na copy of the Unicode data files and any associated documentation\n(the 'Data Files') or Unicode software and any associated documentation\n(the 'Software') to deal in the Data Files or Software\nwithout restriction, including without limitation the rights to use,\ncopy, modify, merge, publish, distribute, and/or sell copies of\nthe Data Files or Software, and to permit persons to whom the Data Files\nor Software are furnished to do so, provided that\n(a) this copyright and permission notice appear with all copies\nof the Data Files or Software,\n(b) this copyright and permission notice appear in associated\ndocumentation, and\n(c) there is clear notice in each modified Data File or in the Software\nas well as in the documentation associated with the Data File(s) or\nSoftware that the data or software has been modified.\n\nTHE DATA FILES AND SOFTWARE ARE PROVIDED 'AS IS', WITHOUT WARRANTY OF\nANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE\nWARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\nNONINFRINGEMENT OF THIRD PARTY RIGHTS.\nIN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS\nNOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL\nDAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,\nDATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER\nTORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR\nPERFORMANCE OF THE DATA FILES OR SOFTWARE.\n\nExcept as contained in this notice, the name of a copyright holder\nshall not be used in advertising or otherwise to promote the sale,\nuse or other dealings in these Data Files or Software without prior\nwritten authorization of the copyright holder.\n\n\nUPX:\n\nCopyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer\nCopyright (C) 1996-2013 László Molnár\nCopyright (C) 2000-2013 John F. Reiser\n\nAll Rights Reserved. This program may be used freely, and you are welcome to redistribute and/or modify it under certain conditions.\n\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the UPX License Agreement for more details: http://upx.sourceforge.net/upx-license.html\r\n\r\n\r\nOpenSSL:\r\n\r\nCopyright (c) 1998-2016 The OpenSSL Project.  All rights reserved.\r\n\r\nRedistribution and use in source and binary forms, with or without\r\nmodification, are permitted provided that the following conditions\r\nare met:\r\n\r\n1. Redistributions of source code must retain the above copyright\r\n   notice, this list of conditions and the following disclaimer. \r\n\r\n2. Redistributions in binary form must reproduce the above copyright\r\n   notice, this list of conditions and the following disclaimer in\r\n   the documentation and/or other materials provided with the\r\n   distribution.\r\n\r\n3. All advertising materials mentioning features or use of this\r\n   software must display the following acknowledgment:\r\n   'This product includes software developed by the OpenSSL Project\r\n   for use in the OpenSSL Toolkit. (http://www.openssl.org/)'\r\n\r\n4. The names 'OpenSSL Toolkit' and 'OpenSSL Project' must not be used to\r\n   endorse or promote products derived from this software without\r\n   prior written permission. For written permission, please contact\r\n   openssl-core@openssl.org.\r\n\r\n5. Products derived from this software may not be called 'OpenSSL'\r\n   nor may 'OpenSSL' appear in their names without prior written\r\n   permission of the OpenSSL Project.\r\n\r\n6. Redistributions of any form whatsoever must retain the following\r\n   acknowledgment:\r\n   'This product includes software developed by the OpenSSL Project\r\n   for use in the OpenSSL Toolkit (http://www.openssl.org/)'\r\n\r\nTHIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY\r\nEXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\r\nIMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\r\nPURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR\r\nITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\r\nSPECIAL, EXEMPLARY, OR").append(" CONSEQUENTIAL DAMAGES (INCLUDING, BUT\r\nNOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\r\nLOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\r\nHOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,\r\nSTRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)\r\nARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED\r\nOF THE POSSIBILITY OF SUCH DAMAGE.\r\n====================================================================\r\n\r\nThis product includes cryptographic software written by Eric Young\r\n(eay@cryptsoft.com).  This product includes software written by Tim\r\nHudson (tjh@cryptsoft.com).\r\n\r\n\r\n Original SSLeay License\r\n -----------------------\r\n\r\nCopyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)\r\nAll rights reserved.\r\n\r\nThis package is an SSL implementation written\r\nby Eric Young (eay@cryptsoft.com).\r\nThe implementation was written so as to conform with Netscapes SSL.\r\n\r\nThis library is free for commercial and non-commercial use as long as\r\nthe following conditions are aheared to.  The following conditions\r\napply to all code found in this distribution, be it the RC4, RSA,\r\nlhash, DES, etc., code; not just the SSL code.  The SSL documentation\r\nincluded with this distribution is covered by the same copyright terms\r\nexcept that the holder is Tim Hudson (tjh@cryptsoft.com).\r\n\r\nCopyright remains Eric Young's, and as such any Copyright notices in\r\nthe code are not to be removed.\r\nIf this package is used in a product, Eric Young should be given attribution\r\nas the author of the parts of the library used.\r\nThis can be in the form of a textual message at program startup or\r\nin documentation (online or textual) provided with the package.\r\n\r\nRedistribution and use in source and binary forms, with or without\r\nmodification, are permitted provided that the following conditions\r\nare met:\r\n1. Redistributions of source code must retain the copyright\r\n   notice, this list of conditions and the following disclaimer.\r\n2. Redistributions in binary form must reproduce the above copyright\r\n   notice, this list of conditions and the following disclaimer in the\r\n   documentation and/or other materials provided with the distribution.\r\n3. All advertising materials mentioning features or use of this software\r\n   must display the following acknowledgement:\r\n   'This product includes cryptographic software written by\r\n    Eric Young (eay@cryptsoft.com)'\r\n   The word 'cryptographic' can be left out if the rouines from the library\r\n   being used are not cryptographic related :-).\r\n4. If you include any Windows specific code (or a derivative thereof) from \r\n   the apps directory (application code) you must include an acknowledgement:\r\n   'This product includes software written by Tim Hudson (tjh@cryptsoft.com)'\r\n\r\nTHIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND\r\nANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\r\nIMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\r\nARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE\r\nFOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\r\nDAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\r\nOR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\r\nHOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\r\nLIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\r\nOUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\r\nSUCH DAMAGE.\r\n\r\nThe licence and distribution terms for any publically available version or\r\nderivative of this code cannot be changed.  i.e. this code cannot simply be\r\ncopied and put under another distribution licence\r\n[including the GNU Public Licence.]");
    gpmErrorForm->show();
    gpmErrorForm->SetMessage(&strMessage);
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_OnlineXComp_Supported_clicked(
    )
{
    //Check for supported online XCompilers and firmware versions
    if (gbTermBusy == false)
    {
        //Send request
        if (LookupDNSName() == true)
        {
            gbTermBusy = true;
            gchTermMode = MODE_CHECK_FIRMWARE_SUPPORT;
            gchTermMode2 = MODE_CHECK_FIRMWARE_SUPPORT;
            ui->btn_Cancel->setEnabled(true);
            ui->btn_ErrorCodeUpdate->setEnabled(false);
            ui->btn_ErrorCodeDownload->setEnabled(false);
            ui->btn_UwTerminalXUpdate->setEnabled(false);
            ui->btn_ModuleFirmware->setEnabled(false);
            ui->btn_OnlineXComp_Supported->setEnabled(false);
            gnmrReply = gnmManager->get(QNetworkRequest(QUrl(QString(WebProtocol).append("://").append(gstrResolvedServer).append("/compiler_list.php"))));
            ui->statusBar->showMessage("Checking for supported XCompilers...");
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_SkipDL_stateChanged(
    int
    )
{
    //Skip download option changed
    gpTermSettings->setValue("SkipDownloadDisplay", (ui->check_SkipDL->isChecked() == true ? 1 : 0));
}

//=============================================================================
//=============================================================================
bool
MainWindow::LookupDNSName(
    )
{
    //Function to lookup hostname of the cloud XCompile server (a workaround for a bug causing a segmentation fault on Linux)
    if (gstrResolvedServer == "")
    {
        //Name not yet resolved
        QHostInfo hiIP = QHostInfo::fromName(gpTermSettings->value("OnlineXCompServer", ServerHost).toString());
        if (hiIP.error() == QHostInfo::NoError)
        {
            //Resolved hostname
            if (hiIP.addresses().isEmpty())
            {
                //No results returned
                QString strMessage = QString("Failed to retrieve an IP address for the cloud XCompile hostname (").append(gpTermSettings->value("OnlineXCompServer", ServerHost).toString()).append("): No IP address is listed for this host.");
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
                return false;
            }
            else
            {
                //Found the host
                gstrResolvedServer = hiIP.addresses().first().toString();

                //Setup QNetwork object
                gnmManager = new QNetworkAccessManager();
                connect(gnmManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
#ifdef UseSSL
                connect(gnmManager, SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)), this, SLOT(sslErrors(QNetworkReply*, QList<QSslError>)));
#endif
                return true;
            }
        }
        else
        {
            //Failed to resolve hostname
            QString strMessage = QString("Failed to resolve cloud XCompile hostname (").append(gpTermSettings->value("OnlineXCompServer", ServerHost).toString()).append("): ").append(hiIP.errorString())
#ifndef _WIN32
#ifndef __APPLE__
                    //Linux, show warning regarding missing DNS libraries
                    .append("\r\n\r\nHave you installed the required additional packages? Please see https://github.com/LairdCP/UwTerminalX/wiki/Installing for further details under the Linux section.")
#endif
#endif
            ;
            gpmErrorForm->show();
            gpmErrorForm->SetMessage(&strMessage);
            return false;
        }
    }
    return true;
}

//=============================================================================
//=============================================================================
QString
MainWindow::AtiToXCompName(
    QString strAtiResp
    )
{
    //Function to extract XCompiler name from ATI response
    if (strAtiResp.length() > MaxDevNameSize)
    {
        //Shorten device name
        strAtiResp = strAtiResp.left(MaxDevNameSize);
    }

    //Only allow a-z, A-Z, 0-9 and underscores in device name
    int intI = 0;
    while (intI < strAtiResp.length())
    {
        //Check each character
        char cTmpC = strAtiResp.at(intI).toLatin1();
        if (!(cTmpC > 47 && cTmpC < 58) && !(cTmpC > 64 && cTmpC < 91) && !(cTmpC > 96 && cTmpC < 123) && cTmpC != 95)
        {
            //Replace non-alphanumeric character with an underscore
            strAtiResp[intI] = '_';
        }
        ++intI;
    }
    return strAtiResp;
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_WebBrowse_clicked(
    )
{
    //Browse applications/modules button clicked
    QString strURL = "";
    if (ui->combo_WebSelection->currentIndex() == 0)
    {
        //BL600 Github
        strURL = "https://github.com/LairdCP/BL600-Applications";
    }
    else if (ui->combo_WebSelection->currentIndex() == 1)
    {
        //BL620 Github
        strURL = "https://github.com/LairdCP/BL620-Applications";
    }
    else if (ui->combo_WebSelection->currentIndex() == 2)
    {
        //BL652 Github
        strURL = "https://github.com/LairdCP/BL652-Applications";
    }
    else if (ui->combo_WebSelection->currentIndex() == 3)
    {
        //BT900 Github
        strURL = "https://github.com/LairdCP/BT900-Applications";
    }
    else if (ui->combo_WebSelection->currentIndex() == 4)
    {
        //RM186/RM191 (RM1xx) Github
        strURL = "https://github.com/LairdCP/RM1xx-Applications";
    }
    else if (ui->combo_WebSelection->currentIndex() == 5)
    {
        //Laird Bluetooth modules page
        strURL = "http://www.lairdtech.com/product-categories/embedded-wireless/bluetooth-modules";
    }
    else if (ui->combo_WebSelection->currentIndex() == 6)
    {
        //Laird EWS support page
        strURL = "https://laird-ews-support.desk.com";
    }

    //Open URL
    if (QDesktopServices::openUrl(QUrl(strURL)) == false)
    {
        //Failed to open URL
        QString strMessage = tr("An error occured whilst attempting to open a web browser, please ensure you have a web browser installed and configured. URL: ").append(strURL);
        gpmErrorForm->show();
        gpmErrorForm->SetMessage(&strMessage);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_EditViewFolder_clicked(
    )
{
    //Open application folder
#if TARGET_OS_MAC
    QString strFullDirname = QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/");
#else
    QString strFullDirname = "./";
#endif

    //Open folder
    QDesktopServices::openUrl(QUrl::fromLocalFile(strFullDirname));
}

//=============================================================================
//=============================================================================
void
MainWindow::on_combo_EditFile_currentIndexChanged(
    int
    )
{
    if (gbEditFileModified == true)
    {
        //Confirm if user wants to discard changes
        if (QMessageBox::question(this, "Discard changes?", "You have unsaved changes to this file, do you wish to discard them and load another file?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
        {
            ui->combo_EditFile->setCurrentIndex(giEditFileType);
            return;
        }
    }

    //Clear the edit box and status text
    ui->text_EditData->clear();
    ui->label_EditInfo->setText("");

    if (ui->combo_EditFile->currentIndex() != 0)
    {
        //Load file data
        QString strFullFilename;

        //Allow edits
        ui->text_EditData->setReadOnly(false);

#if TARGET_OS_MAC
        strFullFilename = QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/");
#else
        strFullFilename = "./";
#endif

        //Create the full filename
        strFullFilename = strFullFilename.append("/").append(ui->combo_EditFile->currentIndex() == 1 ? "UwTerminalX.ini" : "Devices.ini");

        //Open the log file for reading
        QFile fileLogFile(strFullFilename);
        if (fileLogFile.open(QFile::ReadOnly | QFile::Text))
        {
            //Get the contents of the log file
            ui->text_EditData->setPlainText(fileLogFile.readAll());
            fileLogFile.close();
            gbEditFileModified = false;
            giEditFileType = ui->combo_EditFile->currentIndex();

            //Information about the file
            QFileInfo fiFileInfo(strFullFilename);
            char cPrefixes[4] = {'K', 'M', 'G', 'T'};
            float fltFilesize = fiFileInfo.size();
            unsigned char cPrefix = 0;
            while (fltFilesize > 1024)
            {
                //Go to next prefix
                fltFilesize = fltFilesize/1024;
                ++cPrefix;
            }

            //Create the filesize string
            QString strFilesize = QString::number(fltFilesize);
            if (strFilesize.indexOf(".") != -1 && strFilesize.length() > (strFilesize.indexOf(".") + 3))
            {
                //Reduce filesize length
                strFilesize = strFilesize.left((strFilesize.indexOf(".") + 3));
            }

            //Update the string to file information of the current file
            ui->label_EditInfo->setText(QString("Created: ").append(fiFileInfo.created().toString("hh:mm dd/MM/yyyy")).append(", Modified: ").append(fiFileInfo.lastModified().toString("hh:mm dd/MM/yyyy")).append(", Size: ").append(strFilesize));

            //Check if a prefix needs adding
            if (cPrefix > 0)
            {
                //Add size prefix
                ui->label_EditInfo->setText(ui->label_EditInfo->text().append(cPrefixes[cPrefix-1]));
            }

            //Append the Byte unit
            ui->label_EditInfo->setText(ui->label_EditInfo->text().append("B"));
        }
        else
        {
            //Configuration file opening failed
            ui->label_EditInfo->setText("Failed to open file.");
        }
    }
    else
    {
        //Lock edit from input
        ui->text_EditData->setReadOnly(true);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_EditSave_clicked(
    )
{
    if (ui->combo_EditFile->currentIndex() != 0)
    {
        //Save file data
        QString strFullFilename;

#if TARGET_OS_MAC
        strFullFilename = QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/");
#else
        strFullFilename = "./";
#endif

        //Create the full filename
        strFullFilename = strFullFilename.append("/").append(ui->combo_EditFile->currentIndex() == 1 ? "UwTerminalX.ini" : "Devices.ini");

        //Open the log file for reading
        QFile fileLogFile(strFullFilename);
        if (fileLogFile.open(QFile::WriteOnly | QFile::Text))
        {
            //Get the contents of the log file
            fileLogFile.write(ui->text_EditData->toPlainText().toUtf8());
            fileLogFile.close();
            gbEditFileModified = false;
        }
        else
        {
            //Log file opening failed
            ui->label_EditInfo->setText("Failed to open file for writing.");
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_EditLoad_clicked(
    )
{
    if (ui->combo_EditFile->currentIndex() != 0)
    {
        //Reload file data
        on_combo_EditFile_currentIndexChanged(0);
    }
}

#ifndef __APPLE__
//=============================================================================
//=============================================================================
void
MainWindow::on_btn_EditExternal_clicked(
    )
{
    if (ui->combo_EditFile->currentIndex() == 0)
    {
        //Opens the UwTerminalX configuration file
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo("UwTerminalX.ini").absoluteFilePath()));
    }
    else if (ui->combo_EditFile->currentIndex() == 1)
    {
        //Opens the predefined devices configuration file
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo("Devices.ini").absoluteFilePath()));
    }
}
#endif

//=============================================================================
//=============================================================================
void
MainWindow::LoadSettings(
    )
{
#if TARGET_OS_MAC
    if (!QDir().exists(QStandardPaths::writableLocation(QStandardPaths::DataLocation)))
    {
        //Create UwTerminalX directory in application support
        QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    }
    gpTermSettings = new QSettings(QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/UwTerminalX.ini"), QSettings::IniFormat); //Handle to settings
    gpErrorMessages = new QSettings(QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/codes.csv"), QSettings::IniFormat); //Handle to error codes
    gpPredefinedDevice = new QSettings(QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/Devices.ini"), QSettings::IniFormat); //Handle to predefined devices
#else
    //Open files in same directory
    gpTermSettings = new QSettings(QString("UwTerminalX.ini"), QSettings::IniFormat); //Handle to settings
    gpErrorMessages = new QSettings(QString("codes.csv"), QSettings::IniFormat); //Handle to error codes
    gpPredefinedDevice = new QSettings(QString("Devices.ini"), QSettings::IniFormat); //Handle to predefined devices
#endif

    //Check if error code file exists
#if TARGET_OS_MAC
    if (QFile::exists(QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/codes.csv")))
#else
    if (QFile::exists("codes.csv"))
#endif
    {
        //Error code file has been loaded
        gbErrorsLoaded = true;
    }

    //Check settings
#if TARGET_OS_MAC
    if (!QFile::exists(QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/UwTerminalX.ini")) || gpTermSettings->value("ConfigVersion").toString() != UwVersion)
#else
    if (!QFile::exists("UwTerminalX.ini") || gpTermSettings->value("ConfigVersion").toString() != UwVersion)
#endif
    {
        //Extract old configuration version
        if (!gpTermSettings->value("ConfigVersion").isNull())
        {
            QRegularExpression reTempRE("([0-9]+)\\.([0-9]+)([a-zA-Z]{0,1})");
            QRegularExpressionMatch remTempREM = reTempRE.match(gpTermSettings->value("ConfigVersion").toString());
            if (remTempREM.hasMatch() == true)
            {
                //Update configuration
                UpdateSettings(remTempREM.captured(1).toInt(), remTempREM.captured(2).toInt(), remTempREM.captured(3).isEmpty() || remTempREM.captured(3).isNull() ? 0 :  remTempREM.captured(3).at(0));
            }
        }

        //No settings, or some config values not present defaults;
        if (gpTermSettings->value("LogFile").isNull())
        {
            gpTermSettings->setValue("LogFile", DefaultLogFile); //Default log file
        }
        if (gpTermSettings->value("LogMode").isNull())
        {
            gpTermSettings->setValue("LogMode", DefaultLogMode); //Clear log before opening, 0 = no, 1 = yes
        }
        if (gpTermSettings->value("LogEnable").isNull())
        {
            gpTermSettings->setValue("LogEnable", DefaultLogEnable); //0 = disabled, 1 = enable
        }
        if (gpTermSettings->value("CompilerDir").isNull())
        {
            gpTermSettings->setValue("CompilerDir", DefaultCompilerDir); //Directory that compilers go in
        }
        if (gpTermSettings->value("CompilerSubDirs").isNull())
        {
            gpTermSettings->setValue("CompilerSubDirs", DefaultCompilerSubDirs); //0 = normal, 1 = use BL600, BL620, BT900 etc. subdir
        }
        if (gpTermSettings->value("DelUWCAfterDownload").isNull())
        {
            gpTermSettings->setValue("DelUWCAfterDownload", DefaultDelUWCAfterDownload); //0 = no, 1 = yes (delets UWC file after it's been downloaded to the target device)
        }
        if (gpTermSettings->value("SysTrayIcon").isNull())
        {
            gpTermSettings->setValue("SysTrayIcon", DefaultSysTrayIcon); //0 = no, 1 = yes (Shows a system tray icon and provides balloon messages)
        }
        if (gpTermSettings->value("SerialSignalCheckInterval").isNull())
        {
            gpTermSettings->setValue("SerialSignalCheckInterval", DefaultSerialSignalCheckInterval); //How often to check status of CTS, DSR, etc. signals in mS (lower = faster but more CPU usage)
        }
        if (gpTermSettings->value("PrePostXCompRun").isNull())
        {
            gpTermSettings->setValue("PrePostXCompRun", DefaultPrePostXCompRun); //If pre/post XCompiler executable is enabled: 1 = enable, 0 = disable
        }
        if (gpTermSettings->value("PrePostXCompFail").isNull())
        {
            gpTermSettings->setValue("PrePostXCompFail", DefaultPrePostXCompFail); //If post XCompiler executable should run if XCompilation fails: 1 = yes, 0 = no
        }
        if (gpTermSettings->value("PrePostXCompMode").isNull())
        {
            gpTermSettings->setValue("PrePostXCompMode", DefaultPrePostXCompMode); //If pre/post XCompiler command runs before or after XCompiler: 0 = before, 1 = after
        }
        if (gpTermSettings->value("PrePostXCompPath").isNull())
        {
            gpTermSettings->setValue("PrePostXCompPath", DefaultPrePostXCompPath); //Filename of pre/post XCompiler executable (with additional arguments)
        }
        if (gpTermSettings->value("OnlineXComp").isNull())
        {
            gpTermSettings->setValue("OnlineXComp", DefaultOnlineXComp); //If Online XCompiler support is enabled: 1 = enable, 0 = disable
        }
        if (gpTermSettings->value("OnlineXCompServer").isNull())
        {
            gpTermSettings->setValue("OnlineXCompServer", ServerHost); //Online XCompiler server IP/Hostname
        }
        if (gpTermSettings->value("TextUpdateInterval").isNull())
        {
            gpTermSettings->setValue("TextUpdateInterval", DefaultTextUpdateInterval); //Interval between screen updates in mS, lower = faster but can be problematic when receiving/sending large amounts of data (200 is good for this)
        }
        if (gpTermSettings->value("SkipDownloadDisplay").isNull())
        {
            gpTermSettings->setValue("SkipDownloadDisplay", DefaultSkipDownloadDisplay); //If the at+fwrh download display should be skipped or not (1 = skip, 0 = show)
        }
        if (gpTermSettings->value("ShiftEnterLineSeparator").isNull())
        {
            gpTermSettings->setValue("ShiftEnterLineSeparator", DefaultShiftEnterLineSeparator); //Shift+enter input (1 = line separater, 0 = newline character)
        }
#ifdef UseSSL
        if (gpTermSettings->value("SSLEnable").isNull())
        {
            gpTermSettings->setValue("SSLEnable", DefaultSSLEnable); //If SSL should be used for online functionality or not (1 = use SSL, 0 = use HTTP)
            if (DefaultSSLEnable == true)
            {
                //HTTPS
                WebProtocol = "https";
            }
            else
            {
                //HTTP
                WebProtocol = "http";
            }
        }
#endif
        gpTermSettings->setValue("ConfigVersion", UwVersion);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::UpdateSettings(
    int intMajor,
    int intMinor,
    QChar qcDelta
    )
{
    //Adds new configuration options
    if (intMajor <= 1)
    {
        if (intMinor <= 4)
        {
            if (qcDelta == 0 || (qcDelta >= 'a' && qcDelta <= 'b'))
            {
                //Add new RM186 and RM191 devices
                int i = 1;
                while (i < 255)
                {
                    if (gpPredefinedDevice->value(QString("Port").append(QString::number(i)).append("Name")).isNull())
                    {
                        break;
                    }
                    ++i;
                }

                if (i < 254)
                {
                    //RM186/RM191
                    QString strTmpStr = QString("Port").append(QString::number(i));
                    gpPredefinedDevice->setValue(QString(strTmpStr).append("Name"), "RM186/RM191");
                    gpPredefinedDevice->setValue(QString(strTmpStr).append("Baud"), "115200");
                    gpPredefinedDevice->setValue(QString(strTmpStr).append("Parity"), "0");
                    gpPredefinedDevice->setValue(QString(strTmpStr).append("Stop"), "1");
                    gpPredefinedDevice->setValue(QString(strTmpStr).append("Data"), "8");
                    gpPredefinedDevice->setValue(QString(strTmpStr).append("Flow"), "1");
                }
            }
        }

        if (intMinor <= 5)
        {
            if (qcDelta == 0)
            {
                //Add new BL652 device
                int i = 1;
                while (i < 255)
                {
                    if (gpPredefinedDevice->value(QString("Port").append(QString::number(i)).append("Name")).isNull())
                    {
                        break;
                    }
                    ++i;
                }

                if (i < 254)
                {
                    //BL652
                    QString strTmpStr = QString("Port").append(QString::number(i));
                    gpPredefinedDevice->setValue(QString(strTmpStr).append("Name"), "BL652");
                    gpPredefinedDevice->setValue(QString(strTmpStr).append("Baud"), "115200");
                    gpPredefinedDevice->setValue(QString(strTmpStr).append("Parity"), "0");
                    gpPredefinedDevice->setValue(QString(strTmpStr).append("Stop"), "1");
                    gpPredefinedDevice->setValue(QString(strTmpStr).append("Data"), "8");
                    gpPredefinedDevice->setValue(QString(strTmpStr).append("Flow"), "1");
                }
            }
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_LogViewExternal_clicked(
    )
{
    //View log in external editor
    if (ui->combo_LogFile->currentIndex() >= 1)
    {
        //Create the full filename
        QString strFullFilename;

        if (ui->combo_LogDirectory->currentIndex() == 1)
        {
            //Log file directory
#if TARGET_OS_MAC
            QFileInfo fiFileInfo(QString((ui->edit_LogFile->text().left(1) == "/" || ui->edit_LogFile->text().left(1) == "\\") ? "" : QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/")).append(ui->edit_LogFile->text()));
#else
            QFileInfo fiFileInfo(ui->edit_LogFile->text());
#endif
            strFullFilename = fiFileInfo.absolutePath();
        }
        else
        {
            //Application directory
#if TARGET_OS_MAC
            strFullFilename = QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/");
#else
            strFullFilename = "./";
#endif
        }
        strFullFilename = strFullFilename.append("/").append(ui->combo_LogFile->currentText());

        //Open file
        QDesktopServices::openUrl(QUrl::fromLocalFile(strFullFilename));
    }
    else
    {
        //Close
        ui->text_LogData->clear();
        ui->label_LogInfo->clear();
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_LogViewFolder_clicked(
    )
{
    //Open log folder
    QString strFullDirname;

    if (ui->combo_LogDirectory->currentIndex() == 1)
    {
        //Log file directory
#if TARGET_OS_MAC
        QFileInfo fiFileInfo(QString((ui->edit_LogFile->text().left(1) == "/" || ui->edit_LogFile->text().left(1) == "\\") ? "" : QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/")).append(ui->edit_LogFile->text()));
#else
        QFileInfo fiFileInfo(ui->edit_LogFile->text());
#endif
        strFullDirname = fiFileInfo.absolutePath();
    }
    else
    {
        //Application directory
#if TARGET_OS_MAC
        strFullDirname = QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/");
#else
        strFullDirname = "./";
#endif
    }

    //Open folder
    QDesktopServices::openUrl(QUrl::fromLocalFile(strFullDirname));
}

//=============================================================================
//=============================================================================
void
MainWindow::on_text_EditData_textChanged(
    )
{
    //Mark file as edited
    gbEditFileModified = true;
}

//=============================================================================
//=============================================================================
void
MainWindow::on_combo_LogFile_currentIndexChanged(
    int
    )
{
    //List item changed - load log file
    QString strFullFilename;

    if (ui->combo_LogDirectory->currentIndex() == 1)
    {
        //Log file directory
#if TARGET_OS_MAC
        QFileInfo fiFileInfo(QString((ui->edit_LogFile->text().left(1) == "/" || ui->edit_LogFile->text().left(1) == "\\") ? "" : QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/")).append(ui->edit_LogFile->text()));
#else
        QFileInfo fiFileInfo(ui->edit_LogFile->text());
#endif
        strFullFilename = fiFileInfo.absolutePath();
    }
    else
    {
        //Application directory
#if TARGET_OS_MAC
        strFullFilename = QString(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).append("/");
#else
        strFullFilename = "./";
#endif
    }

    ui->text_LogData->clear();
    ui->label_LogInfo->clear();
    if (ui->combo_LogFile->currentIndex() >= 1)
    {
        //Create the full filename
        strFullFilename = strFullFilename.append("/").append(ui->combo_LogFile->currentText());

        //Open the log file for reading
        QFile fileLogFile(strFullFilename);
        if (fileLogFile.open(QFile::ReadOnly | QFile::Text))
        {
            //Get the contents of the log file
            ui->text_LogData->setPlainText(fileLogFile.readAll());
            fileLogFile.close();

            //Information about the log file
            QFileInfo fiFileInfo(strFullFilename);
            char cPrefixes[4] = {'K', 'M', 'G', 'T'};
            float fltFilesize = fiFileInfo.size();
            unsigned char cPrefix = 0;
            while (fltFilesize > 1024)
            {
                //Go to next prefix
                fltFilesize = fltFilesize/1024;
                ++cPrefix;
            }

            //Create the filesize string
            QString strFilesize = QString::number(fltFilesize);
            if (strFilesize.indexOf(".") != -1 && strFilesize.length() > (strFilesize.indexOf(".") + 3))
            {
                //Reduce filesize length
                strFilesize = strFilesize.left((strFilesize.indexOf(".") + 3));
            }

            //Update the string to file information of the current log
            ui->label_LogInfo->setText(QString("Created: ").append(fiFileInfo.created().toString("hh:mm dd/MM/yyyy")).append(", Modified: ").append(fiFileInfo.lastModified().toString("hh:mm dd/MM/yyyy")).append(", Size: ").append(strFilesize));

            //Check if a prefix needs adding
            if (cPrefix > 0)
            {
                //Add size prefix
                ui->label_LogInfo->setText(ui->label_LogInfo->text().append(cPrefixes[cPrefix-1]));
            }

            //Append the Byte unit
            ui->label_LogInfo->setText(ui->label_LogInfo->text().append("B"));
        }
        else
        {
            //Log file opening failed
            ui->label_LogInfo->setText("Failed to open log file.");
        }
    }
    else
    {
        //Close
        ui->text_LogData->clear();
        ui->label_LogInfo->clear();
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_ReloadLog_clicked(
    )
{
    //Reload log
    on_combo_LogFile_currentIndexChanged(ui->combo_LogFile->currentIndex());
}

//=============================================================================
//=============================================================================
#ifdef UseSSL
void
MainWindow::on_check_EnableSSL_stateChanged(
    int
    )
{
    //Update SSL preference
    gpTermSettings->setValue("SSLEnable", ui->check_EnableSSL->isChecked());
    if (ui->check_EnableSSL->isChecked() == true)
    {
        //HTTPS
        WebProtocol = "https";
    }
    else
    {
        //HTTP
        WebProtocol = "http";
    }
}
#endif

//=============================================================================
//=============================================================================
void
MainWindow::on_check_ShowFileSize_stateChanged(
    int
    )
{
    //Update show file size preference
    gpTermSettings->setValue("ShowFileSize", ui->check_ShowFileSize->isChecked());
}

//=============================================================================
//=============================================================================
QString
MainWindow::CleanFilesize(
    QString strFilename
    )
{
    //Cleanly calculates the filesize of an application rounding up
    QFile fileFileName(strFilename);
    if (!fileFileName.exists())
    {
        //File does not exist
        return "Size not known";
    }
    else
    {
        //File exists
        float intSize = fileFileName.size();
        if (intSize > 1073741824)
        {
            //GB (If this occurs then something went very, very wrong)
            intSize = std::ceil(intSize/10737418.24)/100;
            return RemoveZeros(QString::number(intSize, 'f', 2)).append("GB");
        }
        else if (intSize > 1048576)
        {
            //MB (This should never occur)
            intSize = std::ceil(intSize/10485.76)/100;
            return RemoveZeros(QString::number(intSize, 'f', 2)).append("MB");
        }
        else if (intSize > 1024)
        {
            //KB
            intSize = std::ceil(intSize/10.24)/100;
            return RemoveZeros(QString::number(intSize, 'f', 2)).append("KB");
        }
        else
        {
            //Bytes
            intSize = std::ceil(intSize*100)/100;
            return RemoveZeros(QString::number(intSize, 'f', 2)).append("B");
        }
    }
}

//=============================================================================
//=============================================================================
QString
MainWindow::RemoveZeros(
    QString strData
    )
{
    //Removes trailing zeros and decimal point
    if (strData.right(2) == "00")
    {
        //Remove trailing zeros and decimal point
        return strData.left(strData.size()-3);
    }
    else if (strData.right(1) == "0")
    {
        //Remove trailing zero
        return strData.left(strData.size()-1);
    }
    else
    {
        //Nothing to remove
        return strData;
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_DetectBaud_clicked(
    )
{
    //Automatic baud rate detection button clicked
    if (gbTermBusy == false && ui->combo_COM->currentText().length() > 0)
    {
        //Display message dialogue
        QMessageBox mbAutoDetect;
        mbAutoDetect.setWindowTitle("Automatic Module Baud Rate Detection");
        mbAutoDetect.setText(QString("Are you sure you want to automatically detect the module baud rate? Please ensure you have selected the correct COM port before initiating this.\r\nAutomatically detect baud rate of module on port ").append(ui->combo_COM->currentText()));
        mbAutoDetect.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        mbAutoDetect.setDefaultButton(QMessageBox::Yes);

        if (mbAutoDetect.exec() == QMessageBox::Yes)
        {
            if (gbLoopbackMode == false)
            {
                //Start at baud rate 2400: for a BREAK reset. It would be very unusual for anyone to be using baud rates 2400 or 4800 on modules
                ui->combo_Baud->setCurrentIndex(1);
                OpenDevice();

                if (gspSerialPort.isOpen() == true)
                {
                    //Disable DTR as it is usually connected to the autorun pin on development boards
                    ui->check_DTR->setChecked(false);
                    ui->check_SpeedDTR->setChecked(false);
                    gspSerialPort.setBreakEnabled(true);

                    //This is a short time for a BREAK as modules should not be operating at 2400 baud.
                    gtmrBaudTimer.start(500);

                    //Use the batch receive buffer to reduce memory consumption
                    gbaBatchReceive.clear();

                    //We're now busy
                    gbTermBusy = true;
                    gbAutoBaud = true;
                    gchTermMode = 50;
                    ui->btn_Cancel->setEnabled(true);
                }
            }
            else
            {
                //Busy or open, show message to user
                QString strMessage = tr("Cannot automatically determine the module baud rate: loopback mode is enabled. Please disable it and retry.");
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
            }
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::DetectBaudTimeout(
    )
{
    //Automatic baud rate detection timeout
    if (gbAutoBaud == true)
    {
        if (ui->combo_Baud->currentIndex() == ui->combo_Baud->count()-1)
        {
            //Finished checking baud rates, module not detected. Tidy up
            gbTermBusy = false;
            gbAutoBaud = false;
            gchTermMode = 0;
            ui->btn_Cancel->setEnabled(false);

            //Output failure message
            QString strMessage = tr("Failed to detect a Laird module on port ").append(ui->combo_COM->currentText()).append(".\r\nPlease check that all cables are connected properly, switches are correctly set, VSP mode is disabled and that there is no autorun application running.");
            gpmErrorForm->show();
            gpmErrorForm->SetMessage(&strMessage);
        }
        else
        {
            //Move on to the next baud rate
            if (gspSerialPort.isOpen() == true)
            {
                //Port is open so no error has occured
                gspSerialPort.close();
                gpSignalTimer->stop();
                ui->combo_Baud->setCurrentIndex(ui->combo_Baud->currentIndex()+1);
                OpenDevice();

                if (gspSerialPort.isOpen() == true)
                {
                    //Start module timer
                    gtmrBaudTimer.start(AutoBaudTimeout);

                    //Send module identify command
                    QByteArray baTmpBA = "  AT I 0";
                    gspSerialPort.write(baTmpBA);
                    gintQueuedTXBytes += baTmpBA.size();
                    DoLineEnd();
                    gspSerialPort.write(baTmpBA);
                    gintQueuedTXBytes += baTmpBA.size();
                    DoLineEnd();
                }
                else
                {
                    //Failed to open so clean up
                    gbTermBusy = false;
                    gbAutoBaud = false;
                    gchTermMode = 0;
                    ui->btn_Cancel->setEnabled(false);
                }
            }
            else
            {
                //Port is not open, tidy up
                gbTermBusy = false;
                gbAutoBaud = false;
                gchTermMode = 0;
                ui->btn_Cancel->setEnabled(false);
            }
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_ConfirmClear_stateChanged(
    int
    )
{
    //Update clearing confirmation setting
    gpTermSettings->setValue("ConfirmClear", (ui->check_ConfirmClear->isChecked() == true ? 1 : 0));
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_LineSeparator_stateChanged(
    int
    )
{
    //Update line separator setting
    gpTermSettings->setValue("ShiftEnterLineSeparator", (ui->check_LineSeparator->isChecked() == true ? 1 : 0));

    //Notify scroll edit
    ui->text_TermEditData->mbLineSeparator = ui->check_LineSeparator->isChecked();
}

//=============================================================================
//=============================================================================
#ifndef SKIPERRORCODEFORM
void
MainWindow::on_btn_Error_clicked(
    )
{
    //Open error form dialogue
    if (gecErrorCodeForm == 0)
    {
        //Initialise error code form
        gecErrorCodeForm = new UwxErrorCode(this);
        gecErrorCodeForm->SetErrorObject(gpErrorMessages);
    }
    gecErrorCodeForm->show();
}
#endif

#ifndef SKIPSCRIPTINGFORM
//=============================================================================
//=============================================================================
void
MainWindow::ScriptStartRequest(
    )
{
    //Request from scripting form to start running script
    unsigned char chReason = ScriptingReasonOK;

    if (!gspSerialPort.isOpen())
    {
        //Serial port is not open
        chReason = ScriptingReasonPortClosed;
    }
    else if (gbTermBusy == true)
    {
        //Terminal is busy
        chReason = ScriptingReasonTermBusy;
    }
    else
    {
        //Terminal is free: allow script execution
        gbScriptingRunning = true;
        gbTermBusy = true;
        gchTermMode = 50;
    }

    //Return the result to the scripting form
    gusScriptingForm->ScriptStartResult(gbScriptingRunning, chReason);
}

//=============================================================================
//=============================================================================
void
MainWindow::ScriptFinished(
    )
{
    //Script execution has finished
    gbTermBusy = false;
    gbScriptingRunning = false;
    gchTermMode = 0;
}
#endif

//=============================================================================
//=============================================================================
void
MainWindow::on_check_SpeedRTS_stateChanged(
    int
    )
{
    //RTS checkbox on speed test page state changed
    ui->check_RTS->setChecked(ui->check_SpeedRTS->isChecked());
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_SpeedDTR_stateChanged(
    int
    )
{
    //DTR checkbox on speed test page state changed
    ui->check_DTR->setChecked(ui->check_SpeedDTR->isChecked());
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_SpeedClear_clicked(
    )
{
    //Clear speed test display
    ui->text_SpeedEditData->clear();
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_SpeedClose_clicked(
    )
{
    //Close/open port on speed test page pressed
    on_btn_TermClose_clicked();
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_SpeedStart_clicked(
    )
{
    //Show speed test menu
    gpSpeedMenu->popup(QCursor::pos());
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_SpeedStop_clicked(
    )
{
    //Speed testing stop button pressed
    if (gtmrSpeedTestDelayTimer != 0)
    {
        //Clean up delayed send data timer
        if (gtmrSpeedTestDelayTimer->isActive())
        {
            if (gchSpeedTestMode == SpeedModeRecv)
            {
                //Cancel instantly
                gchSpeedTestMode = SpeedModeInactive;
            }
            gtmrSpeedTestDelayTimer->stop();
        }
        disconnect(gtmrSpeedTestDelayTimer, SIGNAL(timeout()), this, SLOT(SpeedTestStartTimer()));
        disconnect(gtmrSpeedTestDelayTimer, SIGNAL(timeout()), this, SLOT(SpeedTestStopTimer()));
        delete gtmrSpeedTestDelayTimer;
        gtmrSpeedTestDelayTimer = 0;
    }

    if ((gchSpeedTestMode == SpeedModeSendRecv || gchSpeedTestMode == SpeedModeRecv) && (gintSpeedBytesReceived10s > 0 || ui->edit_SpeedBytesRec10s->text().toInt() > 0))
    {
        //Data has been received in the past 10 seconds: start a timer before stopping to catch the extra data packets
        gchSpeedTestMode = SpeedModeRecv;
        gtmrSpeedTestDelayTimer = new QTimer();
        gtmrSpeedTestDelayTimer->setSingleShot(true);
        connect(gtmrSpeedTestDelayTimer, SIGNAL(timeout()), this, SLOT(SpeedTestStopTimer()));
        gtmrSpeedTestDelayTimer->start(5000);

        //Show message that test will end soon
        ui->statusBar->showMessage("Waiting 5 seconds for packets to be received... Click cancel again to stop instantly.");
    }
    else if (gchSpeedTestMode == SpeedModeSendRecv || gchSpeedTestMode == SpeedModeSend)
    {
        //Delay for 5 seconds for buffer to clear
        gchSpeedTestMode = SpeedModeInactive;
        gtmrSpeedTestDelayTimer = new QTimer();
        gtmrSpeedTestDelayTimer->setSingleShot(true);
        connect(gtmrSpeedTestDelayTimer, SIGNAL(timeout()), this, SLOT(SpeedTestStopTimer()));
        gtmrSpeedTestDelayTimer->start(5000);

        //Show message that test will end soon
        ui->statusBar->showMessage("Waiting 5 seconds for buffers to empty... Click cancel again to stop instantly.");
    }
    else
    {
        //Change control status
        ui->btn_SpeedStop->setEnabled(false);
        ui->btn_SpeedStart->setEnabled(true);
        ui->combo_SpeedDataType->setEnabled(true);
        if (ui->combo_SpeedDataType->currentIndex() == 1)
        {
            //Enable string options
            ui->edit_SpeedTestData->setEnabled(true);
            ui->check_SpeedStringUnescape->setEnabled(true);
        }

        //Update values
        OutputSpeedTestAvgStats((gtmrSpeedTimer.nsecsElapsed() < 1000000000 ? 1000000000 : gtmrSpeedTimer.nsecsElapsed()/1000000000));

        //Set speed test as no longer running
        gchSpeedTestMode = SpeedModeInactive;
        gbSpeedTestRunning = false;

        if (gtmrSpeedTimer.isValid())
        {
            //Invalidate speed test timer
            gtmrSpeedTimer.invalidate();
        }
        if (gtmrSpeedTestStats.isActive())
        {
            //Stop stats update timer
            gtmrSpeedTestStats.stop();
        }
        if (gtmrSpeedTestStats10s.isActive())
        {
            //Stop 10 second stats update timer
            gtmrSpeedTestStats10s.stop();
        }

        //Clear buffers
        gbaSpeedMatchData.clear();
        gbaSpeedReceivedData.clear();

        //Show message that test has finished
        ui->statusBar->showMessage("Speed testing finished.");
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::SpeedMenuSelected(
    QAction *qaAction
    )
{
    //Speed test menu item selected
    QChar chItem = qaAction->data().toChar();

    if (gspSerialPort.isOpen() == true && gbLoopbackMode == false && gbTermBusy == false)
    {
        //Check size of string if sending data
        if (ui->combo_SpeedDataType->currentIndex() != 0 && !(ui->edit_SpeedTestData->text().length() > 3))
        {
            //Invalid string size
            QString strMessage = tr("Error: Test data string must be a minimum of 4 bytes for speed testing.");
            gpmErrorForm->show();
            gpmErrorForm->SetMessage(&strMessage);
            return;
        }

        //Enable testing
        gintSpeedTestDataBits = gspSerialPort.dataBits();
        gintSpeedTestStartStopParityBits = gspSerialPort.stopBits() + 1 + (gspSerialPort.parity() == QSerialPort::NoParity ? 0 : 1); //Odd/even parity is one bit and include start bit
        gintSpeedTestBytesBits = ui->combo_SpeedDataDisplay->currentIndex();
        gbSpeedTestRunning = true;
        ui->btn_SpeedStop->setEnabled(true);
        ui->btn_SpeedStart->setEnabled(false);
        ui->combo_SpeedDataType->setEnabled(false);
        ui->edit_SpeedTestData->setEnabled(false);
        ui->check_SpeedStringUnescape->setEnabled(false);

        if (gtmrSpeedTimer.isValid())
        {
            //Invalidate timer so it can be restarted
            gtmrSpeedTimer.invalidate();
        }
        gtmrSpeedTimer.start();

        //Reset all counters and variables
        gintSpeedBytesReceived = 0;
        gintSpeedBytesReceived10s = 0;
        gintSpeedBytesSent = 0;
        gintSpeedBytesSent10s = 0;
        gintSpeedBufferCount = 0;
        gintSpeedTestStatPacketsSent = 0;
        gintSpeedTestStatPacketsReceived = 0;
        gintSpeedTestReceiveIndex = 0;
        gintSpeedTestStatSuccess = 0;
        gintSpeedTestStatErrors = 0;

        //Clear all text boxes
        ui->edit_SpeedPacketsBad->setText("0");
        ui->edit_SpeedPacketsRec->setText("0");
        ui->edit_SpeedPacketsRec10s->setText("0");
        ui->edit_SpeedPacketsSent->setText("0");
        ui->edit_SpeedPacketsSent10s->setText("0");
        ui->edit_SpeedPacketsErrorRate->setText("0");
        ui->edit_SpeedPacketsGood->setText("0");
        ui->edit_SpeedPacketsRecAvg->setText("0");
        ui->edit_SpeedPacketsSentAvg->setText("0");
        ui->edit_SpeedBytesRec->setText("0");
        ui->edit_SpeedBytesRec10s->setText("0");
        ui->edit_SpeedBytesRecAvg->setText("0");
        ui->edit_SpeedBytesSent->setText("0");
        ui->edit_SpeedBytesSent10s->setText("0");
        ui->edit_SpeedBytesSentAvg->setText("0");

        //Clear all labels
        ui->label_SpeedRx->setText("0");
        ui->label_SpeedTx->setText("0");
        ui->label_SpeedTime->setText("00:00:00:00");

        //Clear received buffer and data match buffer
        gbaSpeedMatchData.clear();
        gbaSpeedReceivedData.clear();

        //Check if this is a string match or throughput-only test
        if (ui->combo_SpeedDataType->currentIndex() != 0)
        {
            //Escape character codes if enabled
            if (ui->check_SpeedStringUnescape->isChecked())
            {
                //Escape
                QString strTmpDat = ui->edit_SpeedTestData->text();
                UwxEscape::EscapeCharacters(&strTmpDat);
                gbaSpeedMatchData = strTmpDat.toUtf8();
            }
            else
            {
                //Normal
                gbaSpeedMatchData = ui->edit_SpeedTestData->text().toUtf8();
            }

            //Set length of match data
            gintSpeedTestMatchDataLength = gbaSpeedMatchData.length();
        }

        if (chItem == SpeedMenuActionRecv)
        {
            //Receive only test
            gchSpeedTestMode = SpeedModeRecv;
        }
        else if (chItem == SpeedMenuActionSend)
        {
            //Send only test
            gchSpeedTestMode = SpeedModeSend;

            //Send data
            SendSpeedTestData(SpeedTestChunkSize);
        }
        else if (chItem == SpeedMenuActionSendRecv || chItem == SpeedMenuActionSendRecv5Delay || chItem == SpeedMenuActionSendRecv10Delay || chItem == SpeedMenuActionSendRecv15Delay)
        {
            //Send and receive test
            gchSpeedTestMode = SpeedModeSendRecv;

            if (chItem == SpeedMenuActionSendRecv5Delay || chItem == SpeedMenuActionSendRecv10Delay || chItem == SpeedMenuActionSendRecv15Delay)
            {
                //Send after delay
                gtmrSpeedTestDelayTimer = new QTimer();
                gtmrSpeedTestDelayTimer->setSingleShot(true);
                connect(gtmrSpeedTestDelayTimer, SIGNAL(timeout()), this, SLOT(SpeedTestStartTimer()));
                gtmrSpeedTestDelayTimer->start(chItem == SpeedMenuActionSendRecv15Delay ? 15000 : (chItem == SpeedMenuActionSendRecv10Delay ? 10000 : 5000));
            }
            else
            {
                //Send immediately
                SendSpeedTestData(SpeedTestChunkSize);
            }
        }

        //Show message in status bar
        ui->statusBar->showMessage(QString((gchSpeedTestMode == SpeedModeSendRecv ? "Send & Receive" : (gchSpeedTestMode == SpeedModeRecv ? "Receive only" : (gchSpeedTestMode == SpeedModeSend ? "Send only" : "Unknown")))).append(" Speed testing started."));

        //Start timers
        gtmrSpeedTestStats.start();
        gtmrSpeedTestStats10s.start();
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::OutputSpeedTestStats(
    )
{
    //Output speed test (10s) stats
    if (gintSpeedBytesSent > 0)
    {
        //Sending active
        if (ui->combo_SpeedDataDisplay->currentIndex() == 1)
        {
            //Data bits
            ui->edit_SpeedBytesSent10s->setText(QString::number(gintSpeedBytesSent10s*gintSpeedTestDataBits));
        }
        else if (ui->combo_SpeedDataDisplay->currentIndex() == 2)
        {
            //All bits
            ui->edit_SpeedBytesSent10s->setText(QString::number(gintSpeedBytesSent10s*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
        }
        else
        {
            //Bytes
            ui->edit_SpeedBytesSent10s->setText(QString::number(gintSpeedBytesSent10s));
        }
        ui->edit_SpeedPacketsSent10s->setText(QString::number(gintSpeedBytesSent10s/gintSpeedTestMatchDataLength));
        gintSpeedBytesSent10s = 0;
    }
    if ((gchSpeedTestMode & SpeedModeRecv) == SpeedModeRecv)
    {
        //Receiving active
        if (ui->combo_SpeedDataDisplay->currentIndex() == 1)
        {
            //Data bits
            ui->edit_SpeedBytesRec10s->setText(QString::number(gintSpeedBytesReceived10s*gintSpeedTestDataBits));
        }
        else if (ui->combo_SpeedDataDisplay->currentIndex() == 2)
        {
            //All bits
            ui->edit_SpeedBytesRec10s->setText(QString::number(gintSpeedBytesReceived10s*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
        }
        else
        {
            //Bytes
            ui->edit_SpeedBytesRec10s->setText(QString::number(gintSpeedBytesReceived10s));
        }
        if (ui->combo_SpeedDataType->currentIndex() != 0)
        {
            //Show stats about packets
            ui->edit_SpeedPacketsRec10s->setText(QString::number(gintSpeedBytesReceived10s/gintSpeedTestMatchDataLength));
        }
        gintSpeedBytesReceived10s = 0;
    }

    //Update average speed test statistics
    OutputSpeedTestAvgStats(gtmrSpeedTimer.nsecsElapsed()/1000000000);
}

//=============================================================================
//=============================================================================
void
MainWindow::on_combo_SpeedDataType_currentIndexChanged(
    int
    )
{
    //Speed test type changed
    if (ui->combo_SpeedDataType->currentIndex() == 0)
    {
        //Throughput only
        ui->edit_SpeedTestData->setEnabled(false);
        ui->edit_SpeedPacketsSent->setEnabled(false);
        ui->edit_SpeedPacketsSent10s->setEnabled(false);
        ui->edit_SpeedPacketsSentAvg->setEnabled(false);
        ui->edit_SpeedPacketsRec->setEnabled(false);
        ui->edit_SpeedPacketsRec10s->setEnabled(false);
        ui->edit_SpeedPacketsRecAvg->setEnabled(false);
        ui->edit_SpeedPacketsGood->setEnabled(false);
        ui->edit_SpeedPacketsBad->setEnabled(false);
        ui->edit_SpeedPacketsErrorRate->setEnabled(false);


        //Disable sending modes
        gpSpeedMenu->actions()[1]->setEnabled(false);
        gpSpeedMenu->actions()[2]->setEnabled(false);
        gpSpeedMenu->actions()[3]->setEnabled(false);
        gpSpeedMenu->actions()[4]->setEnabled(false);
        gpSpeedMenu->actions()[5]->setEnabled(false);
    }
    else if (ui->combo_SpeedDataType->currentIndex() == 1)
    {
        //String
        ui->edit_SpeedTestData->setEnabled(true);
        ui->edit_SpeedPacketsSent->setEnabled(true);
        ui->edit_SpeedPacketsSent10s->setEnabled(true);
        ui->edit_SpeedPacketsSentAvg->setEnabled(true);
        ui->edit_SpeedPacketsRec->setEnabled(true);
        ui->edit_SpeedPacketsRec10s->setEnabled(true);
        ui->edit_SpeedPacketsRecAvg->setEnabled(true);
        ui->edit_SpeedPacketsGood->setEnabled(true);
        ui->edit_SpeedPacketsBad->setEnabled(true);
        ui->edit_SpeedPacketsErrorRate->setEnabled(true);


        //Enable sending modes
        gpSpeedMenu->actions()[1]->setEnabled(true);
        gpSpeedMenu->actions()[2]->setEnabled(true);
        gpSpeedMenu->actions()[3]->setEnabled(true);
        gpSpeedMenu->actions()[4]->setEnabled(true);
        gpSpeedMenu->actions()[5]->setEnabled(true);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_SpeedCopy_clicked(
    )
{
    //Copies some data to the clipboard about the test
    QString strTmpStr = ui->edit_SpeedTestData->text();
    QString strResultStr;
//TODO: rx all bit (when in byte/data bit mode) is WRONG (too small)
    if (ui->combo_SpeedDataDisplay->currentIndex() == 1)
    {
        //Data bits
        strResultStr = QString("\r\n    > Tx (Bytes): ").
        append(QString::number(ui->edit_SpeedBytesSent->text().toUInt()/gintSpeedTestDataBits)).
        append("\r\n    > Tx (Data bits): ").
        append(ui->edit_SpeedBytesSent->text()).
        append("\r\n    > Tx (All bits): ").
        append(QString::number(ui->edit_SpeedBytesSent->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)/gintSpeedTestDataBits)).
        append("\r\n    > Tx last 10s (Bytes): ").
        append(QString::number(ui->edit_SpeedBytesSent10s->text().toUInt()/gintSpeedTestDataBits)).
        append("\r\n    > Tx last 10s (Data bits): ").
        append(ui->edit_SpeedBytesSent10s->text()).
        append("\r\n    > Tx last 10s (All bits): ").
        append(QString::number(ui->edit_SpeedBytesSent10s->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)/gintSpeedTestDataBits)).
        append("\r\n    > Tx average (Bytes): ").
        append(QString::number(ui->edit_SpeedBytesSentAvg->text().toUInt()/gintSpeedTestDataBits)).
        append("\r\n    > Tx average (Data bits): ").
        append(ui->edit_SpeedBytesSentAvg->text()).
        append("\r\n    > Tx average (All bits): ").
        append(QString::number(ui->edit_SpeedBytesSentAvg->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)/gintSpeedTestDataBits)).
        append("\r\n    > Rx (Bytes): ").
        append(QString::number(ui->edit_SpeedBytesRec->text().toUInt()/gintSpeedTestDataBits)).
        append("\r\n    > Rx (Data bits): ").
        append(ui->edit_SpeedBytesRec->text()).
        append("\r\n    > Rx (All bits): ").
        append(QString::number(ui->edit_SpeedBytesRec->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)/gintSpeedTestDataBits)).
        append("\r\n    > Rx last 10s (Bytes): ").
        append(QString::number(ui->edit_SpeedBytesRec10s->text().toUInt()/gintSpeedTestDataBits)).
        append("\r\n    > Rx last 10s (Data bits): ").
        append(ui->edit_SpeedBytesRec10s->text()).
        append("\r\n    > Rx last 10s (All bits): ").
        append(QString::number(ui->edit_SpeedBytesRec10s->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)/gintSpeedTestDataBits)).
        append("\r\n    > Rx average (Bytes): ").
        append(QString::number(ui->edit_SpeedBytesRecAvg->text().toUInt()/gintSpeedTestDataBits)).
        append("\r\n    > Rx average (Data bits): ").
        append(ui->edit_SpeedBytesRecAvg->text()).
        append("\r\n    > Rx average (All bits): ").
        append(QString::number(ui->edit_SpeedBytesRecAvg->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)/gintSpeedTestDataBits));
    }
    else if (ui->combo_SpeedDataDisplay->currentIndex() == 2)
    {
        //All bits
        strResultStr = QString("\r\n    > Tx (Bytes): ").
        append(QString::number(ui->edit_SpeedBytesSent->text().toUInt()/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Tx (Data bits): ").
        append(QString::number(ui->edit_SpeedBytesSent->text().toUInt()*gintSpeedTestDataBits/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Tx (All bits): ").
        append(ui->edit_SpeedBytesSent->text()).
        append("\r\n    > Tx last 10s (Bytes): ").
        append(QString::number(ui->edit_SpeedBytesSent10s->text().toUInt()/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Tx last 10s (Data bits): ").
        append(QString::number(ui->edit_SpeedBytesSent10s->text().toUInt()*gintSpeedTestDataBits/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Tx last 10s (All bits): ").
        append(ui->edit_SpeedBytesSent10s->text()).
        append("\r\n    > Tx average (Bytes): ").
        append(QString::number(ui->edit_SpeedBytesSentAvg->text().toUInt()/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Tx average (Data bits): ").
        append(QString::number(ui->edit_SpeedBytesSentAvg->text().toUInt()*gintSpeedTestDataBits/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Tx average (All bits): ").
        append(ui->edit_SpeedBytesSentAvg->text()).
        append("\r\n    > Rx (Bytes): ").
        append(QString::number(ui->edit_SpeedBytesRec->text().toUInt()/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Rx (Data bits): ").
        append(QString::number(ui->edit_SpeedBytesRec->text().toUInt()*gintSpeedTestDataBits/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Rx (All bits): ").
        append(ui->edit_SpeedBytesRec->text()).
        append("\r\n    > Rx last 10s (Bytes): ").
        append(QString::number(ui->edit_SpeedBytesRec10s->text().toUInt()/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Rx last 10s (Data bits): ").
        append(QString::number(ui->edit_SpeedBytesRec10s->text().toUInt()*gintSpeedTestDataBits/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Rx last 10s (All bits): ").
        append(ui->edit_SpeedBytesRec10s->text()).
        append("\r\n    > Rx average (Bytes): ").
        append(QString::number(ui->edit_SpeedBytesRecAvg->text().toUInt()/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Rx average (Data bits): ").
        append(QString::number(ui->edit_SpeedBytesRecAvg->text().toUInt()*gintSpeedTestDataBits/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Rx average (All bits): ").
        append(ui->edit_SpeedBytesRecAvg->text());
    }
    else
    {
        //Bytes
        strResultStr = QString("\r\n    > Tx (Bytes): ").
        append(ui->edit_SpeedBytesSent->text()).
        append("\r\n    > Tx (Data bits): ").
        append(QString::number(ui->edit_SpeedBytesSent->text().toUInt()*gintSpeedTestDataBits)).
        append("\r\n    > Tx (All bits): ").
        append(QString::number(ui->edit_SpeedBytesSent->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Tx last 10s (Bytes): ").
        append(ui->edit_SpeedBytesSent10s->text()).
        append("\r\n    > Tx last 10s (Data bits): ").
        append(QString::number(ui->edit_SpeedBytesSent10s->text().toUInt()*gintSpeedTestDataBits)).
        append("\r\n    > Tx last 10s (All bits): ").
        append(QString::number(ui->edit_SpeedBytesSent10s->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Tx average (Bytes): ").
        append(ui->edit_SpeedBytesSentAvg->text()).
        append("\r\n    > Tx average (Data bits): ").
        append(QString::number(ui->edit_SpeedBytesSentAvg->text().toUInt()*gintSpeedTestDataBits)).
        append("\r\n    > Tx average (All bits): ").
        append(QString::number(ui->edit_SpeedBytesSentAvg->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Rx (Bytes): ").
        append(ui->edit_SpeedBytesRec->text()).
        append("\r\n    > Rx (Data bits): ").
        append(QString::number(ui->edit_SpeedBytesRec->text().toUInt()*gintSpeedTestDataBits)).
        append("\r\n    > Rx (All bits): ").
        append(QString::number(ui->edit_SpeedBytesRec->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Rx last 10s (Bytes): ").
        append(ui->edit_SpeedBytesRec10s->text()).
        append("\r\n    > Rx last 10s (Data bits): ").
        append(QString::number(ui->edit_SpeedBytesRec10s->text().toUInt()*gintSpeedTestDataBits)).
        append("\r\n    > Rx last 10s (All bits): ").
        append(QString::number(ui->edit_SpeedBytesRec10s->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits))).
        append("\r\n    > Rx average (Bytes): ").
        append(ui->edit_SpeedBytesRecAvg->text()).
        append("\r\n    > Rx average (Data bits): ").
        append(QString::number(ui->edit_SpeedBytesRecAvg->text().toUInt()*gintSpeedTestDataBits)).
        append("\r\n    > Rx average (All bits): ").
        append(QString::number(ui->edit_SpeedBytesRecAvg->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
    }
    UwxEscape::EscapeCharacters(&strTmpStr);
    QApplication::clipboard()->setText(QString("=================================\r\n  UwTerminalX ").
        append(UwVersion).append(" Speed Test\r\n       ").
        append(QDate::currentDate().toString("dd/MM/yyyy")).
        append(" @ ").append(QTime::currentTime().toString("hh:mm")).
        append("\r\n---------------------------------\r\nSettings:").
        append("\r\n    > Port: ").
        append(ui->label_SpeedConn->text()).
        append("\r\n    > Data Type: ").
        append(ui->combo_SpeedDataType->currentText()).
        append("\r\n    > Data String: ").
        append(ui->edit_SpeedTestData->text()).
        append("\r\n    > Data String Length: ").
        append(QString::number(ui->edit_SpeedTestData->text().length())).
        append("\r\n    > Data String Size (bytes): ").
        append(QString::number(ui->edit_SpeedTestData->text().toUtf8().size())).
        append("\r\n    > Unescaped Data String: ").
        append(strTmpStr).
        append("\r\n    > Unescaped Data String Length: ").
        append(QString::number(strTmpStr.length())).
        append("\r\n    > Unescaped Data String Size (bytes): ").
        append(QString::number(strTmpStr.toUtf8().size())).
        append("\r\n    > Unescape: ").
        append((ui->check_SpeedStringUnescape->isChecked() ? "Yes" : "No")).
        append("\r\n    > Test Type: ").
        append((gchSpeedTestMode == SpeedModeSendRecv ? "Send/Receive" : (gchSpeedTestMode == SpeedModeSend ? "Send" : (gchSpeedTestMode == SpeedModeRecv ? "Receive" : "Inactive")))).
        append("\r\n---------------------------------\r\nResults:\r\n    > Test time: ").
        append(ui->label_SpeedTime->text()).
        append(strResultStr).
        append("\r\n    > Tx (Packets): ").
        append(ui->edit_SpeedPacketsSent->text()).
        append("\r\n    > Tx last 10s (Packets): ").
        append(ui->edit_SpeedPacketsSent10s->text()).
        append("\r\n    > Tx average (Packets): ").
        append(ui->edit_SpeedPacketsSentAvg->text()).
        append("\r\n    > Rx (Packets): ").
        append(ui->edit_SpeedPacketsRec->text()).
        append("\r\n    > Rx last 10s (Packets): ").
        append(ui->edit_SpeedPacketsRec10s->text()).
        append("\r\n    > Rx average (Packets): ").
        append(ui->edit_SpeedPacketsRecAvg->text()).
        append("\r\n    > Rx Good (Packets): ").
        append(ui->edit_SpeedPacketsGood->text()).
        append("\r\n    > Rx Bad (Packets): ").
        append(ui->edit_SpeedPacketsBad->text()).
        append("\r\n    > Rx Error Rate % (Packets): ").
        append(ui->edit_SpeedPacketsErrorRate->text()).
        append("\r\n=================================\r\n"));
}

//=============================================================================
//=============================================================================
void
MainWindow::SendSpeedTestData(
    int intMaxLength
    )
{
    //Send string out. It's OK to send less than the maximum length but not more
    int intSendTimes = (intMaxLength / gintSpeedTestMatchDataLength);
    if (ui->check_SpeedShowTX->isChecked())
    {
        //Show TX data in terminal
        while (intSendTimes > 0)
        {
            //Append to buffer
            gbaSpeedDisplayBuffer.append(gbaSpeedMatchData);
            --intSendTimes;
        }
        if (!gtmrSpeedUpdateTimer.isActive())
        {
            gtmrSpeedUpdateTimer.start();
        }

        //Reset variable for actual sending
        intSendTimes = (intMaxLength / gintSpeedTestMatchDataLength);
    }

    while (intSendTimes > 0)
    {
        //Send out until finished
        gspSerialPort.write(gbaSpeedMatchData);
        gintSpeedBufferCount += gintSpeedTestMatchDataLength;
        --intSendTimes;
        ++gintSpeedTestStatPacketsSent;
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::SpeedTestBytesWritten(
    qint64 intByteCount
    )
{
    //Serial port bytes have been written in speed test mode
    if ((gchSpeedTestMode & SpeedModeSend) == SpeedModeSend)
    {
        //Sending data in speed test
        gintSpeedBufferCount -= intByteCount;
        if (gintSpeedBufferCount <= SpeedTestMinBufSize)
        {
            //Buffer has space: send more data
            SendSpeedTestData(SpeedTestChunkSize);
        }
    }

    //Add to bytes sent counters
    gintSpeedBytesSent += intByteCount;
    gintSpeedBytesSent10s += intByteCount;
}

//=============================================================================
//=============================================================================
void
MainWindow::SpeedTestReceive(
    )
{
    //Receieved data from serial port in speed test mode
    QByteArray baOrigData = gspSerialPort.readAll();
    if ((gchSpeedTestMode & SpeedModeRecv) == SpeedModeRecv)
    {
        //Check data as in receieve mode
        gintSpeedBytesReceived += baOrigData.size();
        gintSpeedBytesReceived10s += baOrigData.size();

        if (ui->check_SpeedShowRX->isChecked() == true)
        {
            //Append RX data to buffer
            gbaSpeedDisplayBuffer.append(baOrigData);
            if (!gtmrSpeedUpdateTimer.isActive())
            {
                gtmrSpeedUpdateTimer.start();
            }
        }

        if (ui->combo_SpeedDataType->currentIndex() != 0)
        {
            //Test data is OK
            gbaSpeedReceivedData.append(baOrigData);
            while (gbaSpeedReceivedData.length() > 0)
            {
                //Data to check
                int SizeToTest = gintSpeedTestMatchDataLength - gintSpeedTestReceiveIndex;
                if (SizeToTest > gbaSpeedReceivedData.length())
                {
                    SizeToTest = gbaSpeedReceivedData.length();
                }

                if (gbaSpeedReceivedData.left(SizeToTest) == gbaSpeedMatchData.mid(gintSpeedTestReceiveIndex, SizeToTest))
                {
                    //Good
                    gbaSpeedReceivedData.remove(0, SizeToTest);
                    gintSpeedTestReceiveIndex = gintSpeedTestReceiveIndex + SizeToTest;
                    if (gintSpeedTestReceiveIndex >= gintSpeedTestMatchDataLength)
                    {
                        ++gintSpeedTestStatSuccess;
                        ++gintSpeedTestStatPacketsReceived;
                        gintSpeedTestReceiveIndex = 0;
                    }
                }
                else
                {
                    //Bad
                    ++gintSpeedTestStatErrors;

                    if (ui->check_SpeedShowErrors->isChecked())
                    {
                        //Show error
                        gbaSpeedDisplayBuffer.append(QString("\r\nError: Data mismatch.\r\n\tExpected: ").append(gbaSpeedMatchData.mid(gintSpeedTestReceiveIndex, SizeToTest)).append("\r\n\tGot     : ").append(gbaSpeedReceivedData.left(SizeToTest)).append("\r\n"));
                        if (!gtmrSpeedUpdateTimer.isActive())
                        {
                            gtmrSpeedUpdateTimer.start();
                        }
                    }

                    //Search for start character (ignoring first character)
                    int StartChar = gbaSpeedReceivedData.indexOf(gbaSpeedMatchData.at(0), 1);
                    if (StartChar == -1)
                    {
                        //Not found, clear whole receive buffer
                        gbaSpeedReceivedData.clear();
                        gintSpeedTestReceiveIndex = 0;
                        gintSpeedTestReceiveIndex = gintSpeedTestReceiveIndex + SizeToTest;
                    }
                    else
                    {
                        //Found, remove until this character
                        gbaSpeedReceivedData.remove(0, StartChar);
                        gintSpeedTestReceiveIndex = 0;
                    }
                    ++gintSpeedTestStatPacketsReceived;
                }
            }
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::UpdateSpeedTestValues(
    )
{
    //Update speed test statistics
    unsigned long lngElapsed = gtmrSpeedTimer.nsecsElapsed()/1000000;
    unsigned int intHours = (lngElapsed / 3600000);
    unsigned char chMinutes = (lngElapsed / 60000) % 60;
    unsigned char chSeconds = (lngElapsed % 60000) / 1000;
    unsigned int intMiliseconds = (lngElapsed % 600)/10;
    ui->label_SpeedTime->setText(QString((intHours < 10 ? "0" : "")).append(QString::number(intHours)).append((chMinutes < 10 ? ":0" : ":")).append(QString::number(chMinutes)).append((chSeconds < 10 ? ":0" : ":")).append(QString::number(chSeconds)).append((intMiliseconds < 10 ? ".0" : ".")).append(QString::number(intMiliseconds)));

    if (gintSpeedBytesSent > 0)
    {
        //Update sent data
        ui->label_SpeedTx->setText(QString::number(gintSpeedBytesSent));
        if (ui->combo_SpeedDataDisplay->currentIndex() == 1)
        {
            //Data bits
            ui->edit_SpeedBytesSent->setText(QString::number(gintSpeedBytesSent*gintSpeedTestDataBits));
        }
        else if (ui->combo_SpeedDataDisplay->currentIndex() == 2)
        {
            //All bits
            ui->edit_SpeedBytesSent->setText(QString::number(gintSpeedBytesSent*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
        }
        else
        {
            //Bytes
            ui->edit_SpeedBytesSent->setText(QString::number(gintSpeedBytesSent));
        }
        ui->edit_SpeedPacketsSent->setText(QString::number(gintSpeedTestStatPacketsSent));
    }

    if ((gchSpeedTestMode & SpeedModeRecv) == SpeedModeRecv)
    {
        //Receive mode active
        ui->label_SpeedRx->setText(QString::number(gintSpeedBytesReceived));
        if (ui->combo_SpeedDataDisplay->currentIndex() == 1)
        {
            //Data bits
            ui->edit_SpeedBytesRec->setText(QString::number(gintSpeedBytesReceived*gintSpeedTestDataBits));
        }
        else if (ui->combo_SpeedDataDisplay->currentIndex() == 2)
        {
            //All bits
            ui->edit_SpeedBytesRec->setText(QString::number(gintSpeedBytesReceived*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
        }
        else
        {
            //Bytes
            ui->edit_SpeedBytesRec->setText(QString::number(gintSpeedBytesReceived));
        }
        ui->edit_SpeedPacketsRec->setText(QString::number(gintSpeedTestStatPacketsReceived));
        ui->edit_SpeedPacketsBad->setText(QString::number(gintSpeedTestStatErrors));
        ui->edit_SpeedPacketsGood->setText(QString::number(gintSpeedTestStatSuccess));
        if (gintSpeedTestStatErrors > 0)
        {
            //Calculate error rate (up to 2 decimal places and rounding up)
            ui->edit_SpeedPacketsErrorRate->setText(QString::number(std::ceil((float)gintSpeedTestStatErrors*10000.0/(float)(gintSpeedTestStatSuccess+gintSpeedTestStatErrors))/100.0));
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::SpeedTestStartTimer(
    )
{
    //Timer expired, begin sending speed test data
    disconnect(gtmrSpeedTestDelayTimer, SIGNAL(timeout()), this, SLOT(SpeedTestStartTimer()));
    delete gtmrSpeedTestDelayTimer;
    gtmrSpeedTestDelayTimer = 0;
    SendSpeedTestData(SpeedTestChunkSize);
}

//=============================================================================
//=============================================================================
void
MainWindow::SpeedTestStopTimer(
    )
{
    //Timer expired, stop receiving speed test data
    disconnect(gtmrSpeedTestDelayTimer, SIGNAL(timeout()), this, SLOT(SpeedTestStopTimer()));
    delete gtmrSpeedTestDelayTimer;
    gtmrSpeedTestDelayTimer = 0;
    ui->btn_SpeedStop->setEnabled(false);
    ui->btn_SpeedStart->setEnabled(true);
    ui->combo_SpeedDataType->setEnabled(true);
    if (ui->combo_SpeedDataType->currentIndex() == 1)
    {
        //Enable string options
        ui->edit_SpeedTestData->setEnabled(true);
        ui->check_SpeedStringUnescape->setEnabled(true);
    }

    //Update values
    OutputSpeedTestAvgStats(gtmrSpeedTimer.nsecsElapsed()/1000000000);

    //Set speed test as no longer running
    gchSpeedTestMode = SpeedModeInactive;
    gbSpeedTestRunning = false;

    if (gtmrSpeedTimer.isValid())
    {
        //Invalidate speed test timer
        gtmrSpeedTimer.invalidate();
    }
    if (gtmrSpeedTestStats.isActive())
    {
        //Stop stats update timer
        gtmrSpeedTestStats.stop();
    }
    if (gtmrSpeedTestStats10s.isActive())
    {
        //Stop 10 second stats update timer
        gtmrSpeedTestStats10s.stop();
    }

    //Clear buffers
    gbaSpeedMatchData.clear();
    gbaSpeedReceivedData.clear();

    //Show finished message in status bar
    ui->statusBar->showMessage("Speed testing finished.");
}

//=============================================================================
//=============================================================================
void
MainWindow::OutputSpeedTestAvgStats(
    unsigned long lngElapsed
    )
{
    //Update average statistics
    if (gintSpeedBytesSent > 0)
    {
        //Sending has activity
        if (ui->combo_SpeedDataDisplay->currentIndex() == 1)
        {
            //Data bits
            ui->edit_SpeedBytesSentAvg->setText(QString::number(gintSpeedBytesSent*gintSpeedTestDataBits/lngElapsed));
        }
        else if (ui->combo_SpeedDataDisplay->currentIndex() == 2)
        {
            //All bits
            ui->edit_SpeedBytesSentAvg->setText(QString::number(gintSpeedBytesSent*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)/lngElapsed));
        }
        else
        {
            //Bytes
            ui->edit_SpeedBytesSentAvg->setText(QString::number(gintSpeedBytesSent/lngElapsed));
        }
        ui->edit_SpeedPacketsSentAvg->setText(QString::number(gintSpeedBytesSent/gintSpeedTestMatchDataLength/lngElapsed));
    }

    if ((gchSpeedTestMode & SpeedModeRecv) == SpeedModeRecv)
    {
        //Receiving active
        if (ui->combo_SpeedDataDisplay->currentIndex() == 1)
        {
            //Data bits
            ui->edit_SpeedBytesRecAvg->setText(QString::number(gintSpeedBytesReceived*gintSpeedTestDataBits/lngElapsed));
        }
        else if (ui->combo_SpeedDataDisplay->currentIndex() == 2)
        {
            //All bits
            ui->edit_SpeedBytesRecAvg->setText(QString::number(gintSpeedBytesReceived*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)/lngElapsed));
        }
        else
        {
            //Bytes
            ui->edit_SpeedBytesRecAvg->setText(QString::number(gintSpeedBytesReceived/lngElapsed));
        }
        if (ui->combo_SpeedDataType->currentIndex() != 0)
        {
            //Show stats about packets
            ui->edit_SpeedPacketsRecAvg->setText(QString::number(gintSpeedBytesReceived/gintSpeedTestMatchDataLength/lngElapsed));
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::UpdateDisplayText(
    )
{
    //Updates the speed display with data from the buffer
    unsigned int uiAnchor = 0;
    unsigned int uiPosition = 0;

    //Slider not held down, update
    unsigned int Pos;
    if (ui->text_SpeedEditData->verticalScrollBar()->sliderPosition() == ui->text_SpeedEditData->verticalScrollBar()->maximum())
    {
        //Scroll to bottom
        Pos = 65535;
    }
    else
    {
        //Stay here
        Pos = ui->text_SpeedEditData->verticalScrollBar()->sliderPosition();
    }

    //Append data
    ui->text_SpeedEditData->moveCursor(QTextCursor::End);
    ui->text_SpeedEditData->insertPlainText(gbaSpeedDisplayBuffer);

    //Go back to previous position
    if (Pos == 65535)
    {
        //Bottom
        ui->text_SpeedEditData->verticalScrollBar()->setValue(ui->text_SpeedEditData->verticalScrollBar()->maximum());
        if (uiAnchor == 0 && uiPosition == 0)
        {
            ui->text_SpeedEditData->moveCursor(QTextCursor::End);
        }
    }
    else
    {
        //Maintain
        ui->text_SpeedEditData->verticalScrollBar()->setValue(Pos);
    }

    //Clear the buffer
    gbaSpeedDisplayBuffer.clear();
}

//=============================================================================
//=============================================================================
void
MainWindow::on_combo_SpeedDataDisplay_currentIndexChanged(
    int
    )
{
    //Change speed test display to bits or bytes
    if (ui->combo_SpeedDataDisplay->currentIndex() == 0)
    {
        //Display in bytes
        ui->group_SpeedBytesBits->setTitle("Bytes");
        ui->edit_SpeedBytesSent->setText(QString::number(ui->edit_SpeedBytesSent->text().toUInt()/(gintSpeedTestDataBits + (gintSpeedTestBytesBits == 2 ? 1 + gintSpeedTestStartStopParityBits : 0))));
        ui->edit_SpeedBytesRec->setText(QString::number(ui->edit_SpeedBytesRec->text().toUInt()/(gintSpeedTestDataBits + (gintSpeedTestBytesBits == 2 ? 1 + gintSpeedTestStartStopParityBits : 0))));
        ui->edit_SpeedBytesSent10s->setText(QString::number(ui->edit_SpeedBytesSent10s->text().toUInt()/(gintSpeedTestDataBits + (gintSpeedTestBytesBits == 2 ? 1 + gintSpeedTestStartStopParityBits : 0))));
        ui->edit_SpeedBytesRec10s->setText(QString::number(ui->edit_SpeedBytesRec10s->text().toUInt()/(gintSpeedTestDataBits + (gintSpeedTestBytesBits == 2 ? 1 + gintSpeedTestStartStopParityBits : 0))));
        ui->edit_SpeedBytesSentAvg->setText(QString::number(ui->edit_SpeedBytesSentAvg->text().toUInt()/(gintSpeedTestDataBits + (gintSpeedTestBytesBits == 2 ? 1 + gintSpeedTestStartStopParityBits : 0))));
        ui->edit_SpeedBytesRecAvg->setText(QString::number(ui->edit_SpeedBytesRecAvg->text().toUInt()/(gintSpeedTestDataBits + (gintSpeedTestBytesBits == 2 ? 1 + gintSpeedTestStartStopParityBits : 0))));
    }
    else if (ui->combo_SpeedDataDisplay->currentIndex() == 1)
    {
        //Display in bits (data only)
        ui->group_SpeedBytesBits->setTitle("Data Bits");
        if (gintSpeedTestBytesBits == 0)
        {
            //From bytes
            ui->edit_SpeedBytesSent->setText(QString::number(ui->edit_SpeedBytesSent->text().toUInt()*gintSpeedTestDataBits));
            ui->edit_SpeedBytesRec->setText(QString::number(ui->edit_SpeedBytesRec->text().toUInt()*gintSpeedTestDataBits));
            ui->edit_SpeedBytesSent10s->setText(QString::number(ui->edit_SpeedBytesSent10s->text().toUInt()*gintSpeedTestDataBits));
            ui->edit_SpeedBytesRec10s->setText(QString::number(ui->edit_SpeedBytesRec10s->text().toUInt()*gintSpeedTestDataBits));
            ui->edit_SpeedBytesSentAvg->setText(QString::number(ui->edit_SpeedBytesSentAvg->text().toUInt()*gintSpeedTestDataBits));
            ui->edit_SpeedBytesRecAvg->setText(QString::number(ui->edit_SpeedBytesRecAvg->text().toUInt()*gintSpeedTestDataBits));
        }
        else
        {
            //From bits
            ui->edit_SpeedBytesSent->setText(QString::number(ui->edit_SpeedBytesSent->text().toUInt()*gintSpeedTestDataBits/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
            ui->edit_SpeedBytesRec->setText(QString::number(ui->edit_SpeedBytesRec->text().toUInt()*gintSpeedTestDataBits/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
            ui->edit_SpeedBytesSent10s->setText(QString::number(ui->edit_SpeedBytesSent10s->text().toUInt()*gintSpeedTestDataBits/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
            ui->edit_SpeedBytesRec10s->setText(QString::number(ui->edit_SpeedBytesRec10s->text().toUInt()*gintSpeedTestDataBits/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
            ui->edit_SpeedBytesSentAvg->setText(QString::number(ui->edit_SpeedBytesSentAvg->text().toUInt()*gintSpeedTestDataBits/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
            ui->edit_SpeedBytesRecAvg->setText(QString::number(ui->edit_SpeedBytesRecAvg->text().toUInt()*gintSpeedTestDataBits/(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
        }
    }
    else
    {
        //Display in bits (including start/stop bits)
        ui->group_SpeedBytesBits->setTitle("All Bits (Data, Start, Stop and Parity bits)");
        if (gintSpeedTestBytesBits == 0)
        {
            //From bytes
            ui->edit_SpeedBytesSent->setText(QString::number(ui->edit_SpeedBytesSent->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
            ui->edit_SpeedBytesRec->setText(QString::number(ui->edit_SpeedBytesRec->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
            ui->edit_SpeedBytesSent10s->setText(QString::number(ui->edit_SpeedBytesSent10s->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
            ui->edit_SpeedBytesRec10s->setText(QString::number(ui->edit_SpeedBytesRec10s->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
            ui->edit_SpeedBytesSentAvg->setText(QString::number(ui->edit_SpeedBytesSentAvg->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
            ui->edit_SpeedBytesRecAvg->setText(QString::number(ui->edit_SpeedBytesRecAvg->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)));
        }
        else
        {
            //From bits
            ui->edit_SpeedBytesSent->setText(QString::number(ui->edit_SpeedBytesSent->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)/gintSpeedTestDataBits));
            ui->edit_SpeedBytesRec->setText(QString::number(ui->edit_SpeedBytesRec->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)/gintSpeedTestDataBits));
            ui->edit_SpeedBytesSent10s->setText(QString::number(ui->edit_SpeedBytesSent10s->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)/gintSpeedTestDataBits));
            ui->edit_SpeedBytesRec10s->setText(QString::number(ui->edit_SpeedBytesRec10s->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)/gintSpeedTestDataBits));
            ui->edit_SpeedBytesSentAvg->setText(QString::number(ui->edit_SpeedBytesSentAvg->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)/gintSpeedTestDataBits));
            ui->edit_SpeedBytesRecAvg->setText(QString::number(ui->edit_SpeedBytesRecAvg->text().toUInt()*(gintSpeedTestDataBits + gintSpeedTestStartStopParityBits)/gintSpeedTestDataBits));
        }
    }
    gintSpeedTestBytesBits = ui->combo_SpeedDataDisplay->currentIndex();
    //Need to account for moving from 10/8 bits back to bytes
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_CheckLicense_stateChanged(
    int
    )
{
    //Option for checking module license on download changed
    gpTermSettings->setValue("LicenseCheck", (ui->check_CheckLicense->isChecked() == true ? 1 : 0));
}

//=============================================================================
//=============================================================================
void
MainWindow::ClearFileDataList(
    )
{
    //Clears the file data list and deletes all list entries
    qint16 i = lstFileData.length()-1;
    while (i >= 0)
    {
        //Free up each element and delete it
        FileSStruct *tempFileS = lstFileData.at(i);
        lstFileData.removeAt(i);
        delete tempFileS;
        --i;
    }
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
