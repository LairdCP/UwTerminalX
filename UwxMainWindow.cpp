/******************************************************************************
** Copyright (C) 2014-2015 Ezurio Ltd
**
** Project: UwTerminalX
**
** Module: UwxMainWindow.cpp
**
** Notes:
**
*******************************************************************************/

/******************************************************************************/
// Include Files
/******************************************************************************/
#include "UwxMainWindow.h"
#include "ui_UwxMainWindow.h"
#include "UwxAutomation.h"

/******************************************************************************/
// Conditional Compile Defines
/******************************************************************************/
#ifdef QT_DEBUG
    //Inclde debug output when compiled for debugging
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
    //Assume linux
    #define OS "Linux"
#endif

/******************************************************************************/
// Global/Static Variable Declarations
/******************************************************************************/
PopupMessage *gpmErrorForm; //Error message form
UwxAutomation *guaAutomationForm; //Automation form

//gchTermMode:
//6 = download file?
//7 = download file+?
//8 = ?

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
    gpTermSettings = new QSettings(QString(gstrMacBundlePath).append("UwTerminalX.ini"), QSettings::IniFormat); //Handle to settings
    gpErrorMessages = new QSettings(QString(gstrMacBundlePath).append("codes.csv"), QSettings::IniFormat); //Handle to error codes

    //Fix mac's resize
    resize(660, 360);
#else
    //Open files in same directory
    gpTermSettings = new QSettings(QString("UwTerminalX.ini"), QSettings::IniFormat); //Handle to settings
    gpErrorMessages = new QSettings(QString("codes.csv"), QSettings::IniFormat); //Handle to error codes
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

    //Disable redo/undo in read only terminal data and clear display buffer byte array.
    gbaDisplayBuffer.clear();

    //Check settings
#if TARGET_OS_MAC
    if (!QFile::exists(QString(gstrMacBundlePath).append("UwTerminalX.ini")))
#else
    if (!QFile::exists("UwTerminalX.ini"))
#endif
    {
        //No settings, set defaults
        gpTermSettings->setValue("LogFile", "UwTerminalX.log"); //Default log file
        gpTermSettings->setValue("LogMode", "0"); //Clear log before opening, 0 = no, 1 = yes
        gpTermSettings->setValue("LogLevel", "1"); //0 = none, 1 = single file, 2 = 1 + new file for each session
        gpTermSettings->setValue("CompilerDir", "compilers/"); //Directory that compilers go in
        gpTermSettings->setValue("CompilerSubDirs", "0"); //0 = normal, 1 = use BL600, BL620, BT900 etc. subdir
        gpTermSettings->setValue("DelUWCAfterDownload", "0"); //0 = no, 1 = yes (delets UWC file after it's been downloaded to the target device)
        gpTermSettings->setValue("SysTrayIcon", "1"); //0 = no, 1 = yes (Shows a system tray icon and provides balloon messages)
        gpTermSettings->setValue("SerialSignalCheckInterval", "50"); //How often to check status of CTS, DSR, etc. signals in mS (lower = faster but more CPU usage)
        gpTermSettings->setValue("PrePostXCompRun", "0"); //If pre/post XCompiler executable is enabled: 1 = enable, 0 = disable
        gpTermSettings->setValue("PrePostXCompFail", "0"); //If post XCompiler executable should run if XCompilation fails: 1 = yes, 0 = no
        gpTermSettings->setValue("PrePostXCompMode", "0"); //If pre/post XCompiler command runs before or after XCompiler: 0 = before, 1 = after
        gpTermSettings->setValue("PrePostXCompPath", ""); //Filename of pre/post XCompiler executable (with additional arguments)
    }

    //Create logging handle and variables for logging mode
    gpMainLog = new LrdLogger;
    bool bLoggerOpened = false;
    unsigned char chLoggerMode = 0;

    //Move to 'About' tab
    ui->selector_Tab->setCurrentIndex(2);

    //Set default values for combo boxes on 'Config' tab
    ui->combo_Baud->setCurrentIndex(8);
    ui->combo_Stop->setCurrentIndex(0);
    ui->combo_Data->setCurrentIndex(1);
    ui->combo_Handshake->setCurrentIndex(2);

    //Load images
    gimEmptyCircleImage = QImage(":/images/EmptyCircle.png");
    gimRedCircleImage = QImage(":/images/RedCircle.png");
    gimGreenCircleImage = QImage(":/images/GreenCircle.png");
    gimUw16Image = QImage(":/images/UwTerminal16.ico");
    gimUw32Image = QImage(":/images/UwTerminal32.ico");

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

    //Connect process termination to signal
    connect(&gprocCompileProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(process_finished(int, QProcess::ExitStatus)));

    //Connect quit signals
    connect(ui->btn_Decline, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btn_Quit, SIGNAL(clicked()), this, SLOT(close()));

    //Connect key-press signals
    connect(ui->text_TermEditData, SIGNAL(EnterPressed()), this, SLOT(EnterPressed()));
    connect(ui->text_TermEditData, SIGNAL(KeyPressed(int)), this, SLOT(KeyPressed(int)));

    //Initialise popup message
    gpmErrorForm = new PopupMessage;

    //Initialise automation popup
    guaAutomationForm = new UwxAutomation;

    //Populate the list of devices
    MainWindow::RefreshSerialDevices();

    //Display version
    ui->statusBar->showMessage(QString("UwTerminalX version ").append(UwVersion).append(" (").append(OS).append("), Built ").append(__DATE__).append(" Using QT ").append(QT_VERSION_STR)
#ifdef QT_DEBUG
    .append(" [DEBUG BUILD]")
#endif
    );
    MainWindow::setWindowTitle(QString("UwTerminalX (v").append(UwVersion).append(")"));

    //Check command line
    QStringList slArgs = QCoreApplication::arguments();
    unsigned char chi = 1;
    bool bArgCom = false;
    while (chi < slArgs.length())
    {
        if (slArgs[chi] == "ACCEPT")
        {
            //Skip the front panel - disable buttons
            ui->btn_Accept->setEnabled(false);
            ui->btn_Decline->setEnabled(false);

            //Switch to config tab
            ui->selector_Tab->setCurrentIndex(1);

            //Default empty images
            ui->image_CTS->setPixmap(*gpEmptyCirclePixmap);
            ui->image_DCD->setPixmap(*gpEmptyCirclePixmap);;
            ui->image_DSR->setPixmap(*gpEmptyCirclePixmap);;
            ui->image_RI->setPixmap(*gpEmptyCirclePixmap);
        }
        else if (slArgs[chi].left(4) == "COM=")
        {
            //Set com port
            ui->combo_COM->setCurrentText(slArgs[chi].right(slArgs[chi].length()-4));
            bArgCom = true;
        }
        else if (slArgs[chi].left(5) == "BAUD=")
        {
            //Set baud rate
            ui->combo_Baud->setCurrentText(slArgs[chi].right(slArgs[chi].length()-5));
        }
        else if (slArgs[chi].left(5) == "STOP=")
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
        else if (slArgs[chi].left(5) == "DATA=")
        {
            //Set data bits
            if (slArgs[chi].right(1) == "7")
            {
                //Seven
                ui->combo_Data->setCurrentIndex(0);
            }
            else if (slArgs[chi].right(1) == "8")
            {
                //Eight
                ui->combo_Data->setCurrentIndex(1);
            }
        }
        else if (slArgs[chi].left(4) == "PAR=")
        {
            //Set parity
            if (slArgs[chi].right(1).toInt() >= 0 && slArgs[chi].right(1).toInt() < 3)
            {
                ui->combo_Parity->setCurrentIndex(slArgs[chi].right(1).toInt());
            }
        }
        else if (slArgs[chi].left(5) == "FLOW=")
        {
            //Set flow control
            if (slArgs[chi].right(1).toInt() >= 0 && slArgs[chi].right(1).toInt() < 3)
            {
                ui->combo_Handshake->setCurrentIndex(slArgs[chi].right(1).toInt());
            }
        }
        else if (slArgs[chi].left(7) == "ENDCHR=")
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
        else if (slArgs[chi] == "POLL")
        {
            //Enables poll mode
            ui->check_Poll->setChecked(true);
        }
        else if (slArgs[chi].left(10) == "LOCALECHO=")
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
        else if (slArgs[chi].left(9) == "LINEMODE=")
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
        else if (slArgs[chi] == "LOG")
        {
            //Enables logging
            if (bLoggerOpened == true)
            {
                //Clear the file contents
                gpMainLog->ClearLog();

                //Add log opened message
                gpMainLog->WriteLogData(tr("-").repeated(31));
                gpMainLog->WriteLogData(tr("\n Log opened ").append(QDate::currentDate().toString("dd/MM/yyyy")).append(" @ ").append(QTime::currentTime().toString("hh:mm")).append(" \n"));
                gpMainLog->WriteLogData(tr("-").repeated(31).append("\n\n"));
            }
            chLoggerMode = 1;
        }
        else if (slArgs[chi] == "LOG+")
        {
            //Enables appending to the previous log file instead of erasing
            chLoggerMode = 2;
        }
        else if (slArgs[chi].left(4) == "LOG=" && bLoggerOpened == false)
        {
            //Specifies log filename
            if (chLoggerMode == 1)
            {
                //Clear log file before opening
                QFile::remove(slArgs[chi].mid(4, -1));
            }

            if (gpMainLog->OpenLogFile(slArgs[chi].mid(4, -1)) == LOG_OK)
            {
                //Log opened
                gpMainLog->WriteLogData(tr("-").repeated(31));
                gpMainLog->WriteLogData(tr("\n Log opened ").append(QDate::currentDate().toString("dd/MM/yyyy")).append(" @ ").append(QTime::currentTime().toString("hh:mm")).append(" \n"));
                gpMainLog->WriteLogData(tr("-").repeated(31).append("\n\n"));
                gbMainLogEnabled = true;
            }
            else
            {
                //Log not writeable
                QString strMessage = tr("Error whilst opening log:\nPlease ensure you have access to the log file ").append(slArgs[chi].mid(4, -1)).append(" and have enough free space on your hard drive.");
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
            }
            bLoggerOpened = true;
        }
        else if (slArgs[chi] == "SHOWCRLF")
        {
            //Displays \t, \r, \n etc. as \t, \r, \n instead of [tab], [new line], [carriage return]
            ui->check_ShowCLRF->setChecked(true);
        }
        else if (slArgs[chi] == "CONNECT")
        {
            //Connect to device at startup
            if (ui->btn_Accept->isEnabled() == false && bArgCom == true)
            {
                //Enough information to connect!
                MainWindow::OpenSerial();
            }
        }
        ++chi;
    }

    if (bLoggerOpened == false)
    {
        //Log file was not opened from command line
        if (gpTermSettings->value("LogLevel", "1").toInt() == 1 || gpTermSettings->value("LogLevel", "1").toInt() == 2)
        {
            //Logging is enabled
#if TARGET_OS_MAC
            if (gpMainLog->OpenLogFile(QString(gstrMacBundlePath).append(gpTermSettings->value("LogFile").toString())) == LOG_OK)
#else
            if (gpMainLog->OpenLogFile(gpTermSettings->value("LogFile").toString()) == LOG_OK)
#endif
            {
                //Log opened
                if (gpTermSettings->value("LogMode", "0").toBool() == true)
                {
                    //Clear the log file
                    gpMainLog->ClearLog();
                }
                gpMainLog->WriteLogData(tr("-").repeated(31));
                gpMainLog->WriteLogData(tr("\n Log opened ").append(QDate::currentDate().toString("dd/MM/yyyy")).append(" @ ").append(QTime::currentTime().toString("hh:mm")).append(" \n"));
                gpMainLog->WriteLogData(tr("-").repeated(31).append("\n\n"));
                gbMainLogEnabled = true;
            }
            else
            {
                //Log not writeable
                QString strMessage = tr("Error whilst opening log.\nPlease ensure you have access to the log file ").append(gpTermSettings->value("LogFile").toString()).append(" and have enough free space on your hard drive.");
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
            }
        }
    }

    //Create menu items
    gpMenu = new QMenu(this);
    gpMenu->addAction(new QAction("XCompile", this));
    gpMenu->addAction(new QAction("XCompile + Load", this));
    gpMenu->addAction(new QAction("XCompile + Load + Run", this));
#ifdef OnlineXComp
    gpMenu->addAction(new QAction("Online XCompile", this));
    gpMenu->addAction(new QAction("Online XCompile + Load", this));
    gpMenu->addAction(new QAction("Online XCompile + Load + Run", this));
#endif
    gpMenu->addAction(new QAction("Load", this));
    gpMenu->addAction(new QAction("Load + Run", this));
    gpMenu->addAction(new QAction("Lookup Selected Error-Code (Hex)", this));
    gpMenu->addAction(new QAction("Lookup Selected Error-Code (Int)", this));
    gpMenu->addAction(new QAction("Enable Loopback (Rx->Tx)", this));
    gpSMenu1 = gpMenu->addMenu("Download");
    gpSMenu2 = gpSMenu1->addMenu("BASIC");
    gpSMenu2->addAction(new QAction("Load Precompiled BASIC", this));
    gpSMenu2->addAction(new QAction("Erase File", this)); //AT+DEL (from file open)
    gpSMenu2->addAction(new QAction("Dir", this)); //AT+DIR
    gpSMenu2->addAction(new QAction("Run", this));
    gpSMenu3 = gpSMenu1->addMenu("Data");
    gpSMenu3->addAction(new QAction("Data File +", this));
    gpSMenu3->addAction(new QAction("Erase File +", this));
    gpSMenu3->addAction(new QAction("Multi Data File +", this)); //Downloads more than 1 data file
    gpSMenu1->addAction(new QAction("Stream File Out", this));
    gpMenu->addAction(new QAction("Font", this));
    gpMenu->addAction(new QAction("Run", this));
    gpMenu->addAction(new QAction("Automation", this));
    gpMenu->addAction(new QAction("Batch", this));
    gpMenu->addAction(new QAction("Clear Display", this));
    gpMenu->addAction(new QAction("Clear RX/TX count", this));
    gpMenu->addSeparator();
    gpMenu->addAction(new QAction("Copy", this));
    gpMenu->addAction(new QAction("Copy All", this));
    gpMenu->addAction(new QAction("Select All", this));

    //Create balloon menu items
    gpBalloonMenu = new QMenu(this);
    gpBalloonMenu->addAction(new QAction("Show UwTerminalX", this));
    gpBalloonMenu->addAction(new QAction("Exit", this));

    //Disable unimplemented actions
    gpSMenu3->actions()[2]->setEnabled(false); //Multi Data File +

    //Connect the menu actions
    connect(gpMenu, SIGNAL(triggered(QAction*)), this, SLOT(triggered(QAction*)), Qt::AutoConnection);
    connect(gpBalloonMenu, SIGNAL(triggered(QAction*)), this, SLOT(balloontriggered(QAction*)), Qt::AutoConnection);

    //Configure the timeout timer
    gtmrDownloadTimeoutTimer.setSingleShot(true);
    gtmrDownloadTimeoutTimer.setInterval(3000);
    connect(&gtmrDownloadTimeoutTimer, SIGNAL(timeout()), this, SLOT(DevRespTimeout()));

    //Configure the signal timer
    gpSignalTimer = new QTimer(this);
    connect(gpSignalTimer, SIGNAL(timeout()), this, SLOT(SerialStatusSlot()));

    //Connect serial ready and serial error signals
    connect(&gspSerialPort, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(&gspSerialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(SerialError(QSerialPort::SerialPortError)));
    connect(&gspSerialPort, SIGNAL(bytesWritten(qint64)), this, SLOT(SerialBytesWritten(qint64)));

    //Populate window handles for automation object
    guaAutomationForm->SetPopupHandle(gpmErrorForm);
    guaAutomationForm->SetMainHandle(this);

    //Set update text display timer to be single shot only and connect to slot
    gtmrTextUpdateTimer.setSingleShot(true);
    gtmrTextUpdateTimer.setInterval(200);
    connect(&gtmrTextUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateReceiveText()));

    //Setup timer for batch file timeout
    gtmrBatchTimeoutTimer.setSingleShot(true);
    connect(&gtmrBatchTimeoutTimer, SIGNAL(timeout()), this, SLOT(BatchTimeoutSlot()));

    //Setup poll timer
    gtmrPollTimer.setInterval(500);
    connect(&gtmrPollTimer, SIGNAL(timeout()), this, SLOT(PollUSB()));

    //Add tooltips
    ui->check_Poll->setToolTip("Enable this to poll the device if it disconnects and automatically re-establish a connection.");
    ui->check_ShowCLRF->setToolTip("Enable this to escape various characters (CR will show as \\r, LF will show as \\n and Tab will show as \\t).");

    //Give focus to accept button
    if (ui->btn_Accept->isEnabled() == true)
    {
        ui->btn_Accept->setFocus();
    }

    //Enable system tray if it's available and enabled
    if (gpTermSettings->value("SysTrayIcon", "1").toBool() == true && QSystemTrayIcon::isSystemTrayAvailable())
    {
        //System tray enabled and available on system, set it up with contect menu/icon and show it
        gpSysTray = new QSystemTrayIcon;
        gpSysTray->setContextMenu(gpBalloonMenu);
        gpSysTray->setIcon(QIcon(*gpUw16Pixmap));
        gpSysTray->show();
        gbSysTrayEnabled = true;
    }

    //Update pre/post XCompile executable options
    ui->check_PreXCompRun->setChecked(gpTermSettings->value("PrePostXCompRun", "0").toBool());
    ui->check_PreXCompFail->setChecked(gpTermSettings->value("PrePostXCompFail", "0").toBool());
    if (gpTermSettings->value("PrePostXCompMode", "0").toInt() == 1)
    {
        //Post-XCompiler run
        ui->radio_XCompPost->setChecked(true);
    }
    else
    {
        //Pre-XCompiler run
        ui->radio_XCompPre->setChecked(true);
    }
    ui->edit_PreXCompFilename->setText(gpTermSettings->value("PrePostXCompPath", "").toString());

    //Update GUI for pre/post XComp executable
    on_check_PreXCompRun_stateChanged(ui->check_PreXCompRun->isChecked()*2);

#ifdef OnlineXComp
    //Setup QNetwork for Online XCompiler
    gnmManager = new QNetworkAccessManager();
    connect(gnmManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
#endif
}

//=============================================================================
//=============================================================================
MainWindow::~MainWindow()
{
    //Disconnect all signals
    disconnect(this, SLOT(process_finished(int, QProcess::ExitStatus)));
    disconnect(this, SLOT(close()));
    disconnect(this, SLOT(EnterPressed()));
    disconnect(this, SLOT(KeyPressed(int)));
    disconnect(this, SLOT(triggered(QAction*)));
    disconnect(this, SLOT(balloontriggered(QAction*)));
    disconnect(this, SLOT(DevRespTimeout()));
    disconnect(this, SLOT(SerialStatusSlot()));
    disconnect(this, SLOT(readData()));
    disconnect(this, SLOT(SerialError(QSerialPort::SerialPortError)));
    disconnect(this, SLOT(SerialBytesWritten(qint64)));
    disconnect(this, SLOT(UpdateReceiveText()));
    disconnect(this, SLOT(BatchTimeoutSlot()));
    disconnect(this, SLOT(PollUSB()));
#ifdef OnlineXComp
    disconnect(this, SLOT(replyFinished(QNetworkReply*)));
#endif

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
        delete gpMainLog;
    }

    //Close popups if open
    if (gpmErrorForm->isVisible())
    {
        //Close warning message
        gpmErrorForm->close();
    }
    if (guaAutomationForm->isVisible())
    {
        //Close automation form
        guaAutomationForm->close();
    }

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

    //Delete variables
    delete gpTermSettings;
    delete gpErrorMessages;
    delete gpSignalTimer;
    delete gpBalloonMenu;
    delete gpSMenu3;
    delete gpSMenu2;
    delete gpSMenu1;
    delete gpMenu;
    delete gpEmptyCirclePixmap;
    delete gpRedCirclePixmap;
    delete gpGreenCirclePixmap;
    delete gpmErrorForm;
    delete guaAutomationForm;
#ifdef OnlineXComp
    delete gnmManager;
#endif
    delete ui;
}

//=============================================================================
//=============================================================================
void
MainWindow::closeEvent
    (
    QCloseEvent *event
    )
{
    //Runs when the form is closed. Close child popups to exit the application
    if (gpmErrorForm->isVisible())
    {
        //Close warning message form
        gpmErrorForm->close();
    }
    if (guaAutomationForm->isVisible())
    {
        //Close automation form
        guaAutomationForm->close();
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Accept_clicked
    (
    )
{
    //Runs when accept button is clicked - disable buttons
    ui->btn_Accept->setEnabled(false);
    ui->btn_Decline->setEnabled(false);

    //Switch to config tab
    ui->selector_Tab->setCurrentIndex(1);

    //Default empty images
    ui->image_CTS->setPixmap(*gpEmptyCirclePixmap);
    ui->image_DCD->setPixmap(*gpEmptyCirclePixmap);
    ui->image_DSR->setPixmap(*gpEmptyCirclePixmap);
    ui->image_RI->setPixmap(*gpEmptyCirclePixmap);
}

//=============================================================================
//=============================================================================
void
MainWindow::on_selector_Tab_currentChanged
    (
    int intIndex
    )
{
    if (ui->btn_Accept->isEnabled() == true && intIndex != 3)
    {
        //Not accepted the terms yet
        ui->selector_Tab->setCurrentIndex(2);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Connect_clicked
    (
    )
{
    //Connect to COM port button clicked.
    if (gtmrPollTimer.isActive())
    {
        //Stop polling USB
        gtmrPollTimer.stop();
    }
    MainWindow::OpenSerial();
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_TermClose_clicked
    (
    )
{
    if (ui->btn_TermClose->text() == "&Open")
    {
        //Open connection
        if (gtmrPollTimer.isActive())
        {
            //Stop polling USB
            gtmrPollTimer.stop();
        }
        MainWindow::OpenSerial();
    }
    else if (ui->btn_TermClose->text() == "C&lose")
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
        ui->check_Echo->setEnabled(false);
        ui->check_Line->setEnabled(false);
        ui->check_RTS->setEnabled(false);

        //Disable text entry
        ui->text_TermEditData->setReadOnly(true);

        //Change status message
        ui->statusBar->showMessage("");

        //Change button text
        ui->btn_TermClose->setText("&Open");

        //Notify automation form
        guaAutomationForm->ConnectionChange(false);

        //Disallow file drops
        setAcceptDrops(false);
    }

    //Update images
    MainWindow::UpdateImages();
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Refresh_clicked
    (
    )
{
    //Refresh the list of serial ports
    MainWindow::RefreshSerialDevices();
}

//=============================================================================
//=============================================================================
void
MainWindow::RefreshSerialDevices
    (
    )
{
    //Clears and refreshes the list of serial devices
    ui->combo_COM->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->combo_COM->addItem(info.portName());
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_TermClear_clicked
    (
    )
{
    //Clears the screen of the terminal tab
    ui->text_TermEditData->ClearDatIn();
}

//=============================================================================
//=============================================================================
void
MainWindow::readData
    (
    )
{
    //Read the data into a buffer and copy it to edit for the display data
    QByteArray baOrigData = gspSerialPort.readAll();;

    if (ui->check_SkipDL->isChecked() == false || (gbTermBusy == false || (gbTermBusy == true && baOrigData.length() > 6)))
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

    if (gbStreamingBatch == true)
    {
        //Batch stream in progress
        gbaBatchReceive += baOrigData;
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
                MainWindow::DoLineEnd();
                gpMainLog->WriteLogData(QString(baFileData).append("\n"));
//        gintStreamBytesRead += FileData.length();
                gtmrBatchTimeoutTimer.start(BatchTimeout);
            }
            gbaBatchReceive.clear();
        }
        else if (gbaBatchReceive.indexOf("\n01\t") != -1 && gbaBatchReceive.indexOf("\r", gbaBatchReceive.indexOf("\n01\t")+4) != -1)
        {
            //Failure code
            QRegularExpression reTempRE("\t([a-zA-Z0-9]{1,9})\r");
            QRegularExpressionMatch remTempREM = reTempRE.match(gbaBatchReceive);
            if (remTempREM.hasMatch() == true)
            {
                //Got the error code
                gbaDisplayBuffer.append("Error during batch command, error code: ").append(remTempREM.captured(1)).append("\n");
            }
            else
            {
                //Unknown error code
                gbaDisplayBuffer.append("Error during batch command, unknown error code.\n");
            }
            gtmrTextUpdateTimer.start();

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

    if (gbTermBusy == true && gchTermMode2 == 0)
    {
        //Currently waiting for a response
        gstrTermBusyData = gstrTermBusyData.append(baOrigData);
        gchTermBusyLines = gchTermBusyLines + baOrigData.count("\n");
        if (gchTermBusyLines == 4)
        {
            //Enough data, check it.
            QRegularExpression reTempRE("\n10	0	([a-zA-Z0-9]{2,14})\r\n00\r\n10	13	([a-zA-Z0-9]{4}) ([a-zA-Z0-9]{4}) \r\n00\r");
            QRegularExpressionMatch remTempREM = reTempRE.match(gstrTermBusyData);
            if (remTempREM.hasMatch() == true)
            {
                if (gchTermMode == 9 || gchTermMode == 10 || gchTermMode == 11)
                {
#ifdef OnlineXComp
                    //Check if online XCompiler supports this device
                    gnmManager->get(QNetworkRequest(QUrl(QString("http://").append(ServerHost).append("/supported.php?JSON=1&Dev=").append(remTempREM.captured(1).left(8)).append("&HashA=").append(remTempREM.captured(2)).append("&HashB=").append(remTempREM.captured(3)))));
                    ui->statusBar->showMessage("Device support request sent...", 500);
#endif
                }
                else
                {
                    //Matched and split, now start the compilation!
                    gtmrDownloadTimeoutTimer.stop();
#ifdef _WIN32
                    //Windows
                    if (QFile::exists(QString(gpTermSettings->value("CompilerDir", "compilers/").toString()).append((gpTermSettings->value("CompilerSubDirs", "0").toBool() == true ? remTempREM.captured(1).left(8).append("/") : "")).append("XComp_").append(remTempREM.captured(1).left(8)).append("_").append(remTempREM.captured(2)).append("_").append(remTempREM.captured(3)).append(".exe")) == true)
#elif TARGET_OS_MAC
                    //Mac
                    if (QFile::exists(QString(gstrMacBundlePath).append(gpTermSettings->value("CompilerDir", "compilers/").toString()).append((gpTermSettings->value("CompilerSubDirs", "0").toBool() == true ? remTempREM.captured(1).left(8).append("/") : "")).append("XComp_").append(remTempREM.captured(1).left(8)).append("_").append(remTempREM.captured(2)).append("_").append(remTempREM.captured(3))) == true)
#else
                    //Linux
//                if (QFile::exists(QString(XCompDir).append("XComp_").append(rx.cap(1).left(8)).append("_").append(rx.cap(2)).append("_").append(rx.cap(3)).append(".exe")) == true)
                    if (QFile::exists(QString(gpTermSettings->value("CompilerDir", "compilers/").toString()).append((gpTermSettings->value("CompilerSubDirs", "0").toBool() == true ? remTempREM.captured(1).left(8).append("/") : "")).append("XComp_").append(remTempREM.captured(1).left(8)).append("_").append(remTempREM.captured(2)).append("_").append(remTempREM.captured(3)) == true)
#endif
#pragma warning("Linux XCompile if statement needs fixing")
                    {
                        //XCompiler found! - First run the Pre XCompile program if enabled and it exists
                        if (ui->check_PreXCompRun->isChecked() == true && ui->radio_XCompPre->isChecked() == true)
                        {
                            //Run Pre-XComp program
                            RunPrePostExecutable(gstrTermFilename);
                        }
#ifdef _WIN32
                        //Windows
                        gprocCompileProcess.start(QString(gpTermSettings->value("CompilerDir", "compilers/").toString()).append((gpTermSettings->value("CompilerSubDirs", "0").toBool() == true ? remTempREM.captured(1).left(8).append("/") : "")).append("XComp_").append(remTempREM.captured(1).left(8)).append("_").append(remTempREM.captured(2)).append("_").append(remTempREM.captured(3)).append(".exe"), QStringList(gstrTermFilename));
#elif TARGET_OS_MAC
                        //Mac
                        gprocCompileProcess.start(QString(gstrMacBundlePath).append(gpTermSettings->value("CompilerDir", "compilers/").toString()).append((gpTermSettings->value("CompilerSubDirs", "0").toBool() == true ? remTempREM.captured(1).left(8).append("/") : "")).append("XComp_").append(remTempREM.captured(1).left(8)).append("_").append(remTempREM.captured(2)).append("_").append(remTempREM.captured(3)), QStringList(gstrTermFilename));
                        //gprocCompileProcess.start(QString(gstrMacBundlePath).append(gpTermSettings->value("CompilerDir", "compilers/").toString()).append((gpTermSettings->value("CompilerSubDirs", "0").toBool() == true ? remTempREM.captured(1).left(8).append("/") : "")).append("XComp_").append(remTempREM.captured(1).left(8)).append("_").append(remTempREM.captured(2)).append("_").append(remTempREM.captured(3), QStringList(gstrTermFilename));
#else
                        //Assume linux
//                      gprocCompileProcess.start(QString("wine"), QStringList(QString(gpTermSettings->value("CompilerDir", "compilers/").toString()).append((gpTermSettings->value("CompilerSubDirs", "0").toBool() == true ? remTempREM.captured(1).left(8).append("/") : "")).append("XComp_").append(rx.cap(1).left(8)).append("_").append(rx.cap(2)).append("_").append(rx.cap(3)).append(".exe"))<<gstrTermFilename);
                        gprocCompileProcess.start(QString(gpTermSettings->value("CompilerDir", "compilers/").toString()).append((gpTermSettings->value("CompilerSubDirs", "0").toBool() == true ? remTempREM.captured(1).left(8).append("/") : "")).append("XComp_").append(remTempREM.captured(1).left(8)).append("_").append(remTempREM.captured(2)).append("_").append(remTempREM.captured(3)), QStringList(gstrTermFilename));
#endif
                        //gprocCompileProcess.waitForFinished(-1);
                    }
                    else
                    {
                        //XCompiler not found
                        QString strMessage = tr("Error during XCompile:\nXCompiler \"XComp_").append(remTempREM.captured(1).left(8)).append("_").append(remTempREM.captured(2)).append("_").append(remTempREM.captured(3))
#ifdef _WIN32
                        .append(".exe")
#endif
                        .append("\" was not found.\r\n\r\nPlease ensure you put XCompile binaries in the correct directory (").append(gpTermSettings->value("CompilerDir", "compilers/").toString()).append((gpTermSettings->value("CompilerSubDirs", "0").toBool() == true ? remTempREM.captured(1).left(8) : "")).append(").");
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
                gintQueuedTXBytes += baTmpBA.size();
                MainWindow::DoLineEnd();
                gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
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
                        QByteArray baTmpBA = QString("AT+FWRH \"").append(gstrHexData.left(ui->edit_FWRH->toPlainText().toInt())).append("\"").toUtf8();
                        gspSerialPort.write(baTmpBA);
                        gintQueuedTXBytes += baTmpBA.size();
                        MainWindow::DoLineEnd();
                        gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
                        if (gstrHexData.length() < ui->edit_FWRH->toPlainText().toInt())
                        {
                            //Finished
                            gstrHexData = "";
                            gbaDisplayBuffer.append("-- Finished downloading file --\n");
                            gtmrTextUpdateTimer.start();
                            gspSerialPort.write("AT+FCL");
                            gintQueuedTXBytes += 6;
                            MainWindow::DoLineEnd();
                            gpMainLog->WriteLogData("AT+FCL\n");
                            if (gpTermSettings->value("DelUWCAfterDownload", "0").toBool() == true && gbIsUWCDownload == true && QFile::exists((gstrTermFilename.lastIndexOf(".") >= 0 ? gstrTermFilename.left(gstrTermFilename.lastIndexOf(".")).append(".uwc") : gstrTermFilename.append(".uwc"))))
                            {
                                //Remove UWC
                                QFile::remove((gstrTermFilename.lastIndexOf(".") >= 0 ? gstrTermFilename.left(gstrTermFilename.lastIndexOf(".")).append(".uwc") : gstrTermFilename.append(".uwc")));
                            }
                        }
                        else
                        {
                            //More data to send
                            gstrHexData = gstrHexData.right(gstrHexData.length()-ui->edit_FWRH->toPlainText().toInt());
                            --gchTermMode2;
                            gtmrDownloadTimeoutTimer.start();
                        }
                        //Update amount of data left to send
                        ui->label_TermTxLeft->setText(QString::number(gstrHexData.length()));
                    }
                    else
                    {
                        gstrHexData = "";
                        gbaDisplayBuffer.append("-- Finished downloading file --\n");
                        gtmrTextUpdateTimer.start();
                        gspSerialPort.write("AT+FCL");
                        gintQueuedTXBytes += 6;
                        MainWindow::DoLineEnd();
                        gpMainLog->WriteLogData("AT+FCL\n");
                        if (gpTermSettings->value("DelUWCAfterDownload", "0").toBool() == true && gbIsUWCDownload == true && QFile::exists((gstrTermFilename.lastIndexOf(".") >= 0 ? gstrTermFilename.left(gstrTermFilename.lastIndexOf(".")).append(".uwc") : gstrTermFilename.append(".uwc"))))
                        {
                            //Remove UWC
                            QFile::remove((gstrTermFilename.lastIndexOf(".") >= 0 ? gstrTermFilename.left(gstrTermFilename.lastIndexOf(".")).append(".uwc") : gstrTermFilename.append(".uwc")));
                        }
                        ++gchTermMode2;
                    }
                }
                else
                {
                    //Presume error
                    gbTermBusy = false;
                    gchTermBusyLines = 0;
                    gchTermMode = 0;
                    gchTermMode2 = 0;
                    QString strMessage = tr("Error whilst downloading data to device. If filesystem is full, please restart device with 'atz' and clear the filesystem using 'at&f 1'.\nPlease note this will erase ALL FILES on the device.\n\nReceived: ").append(QString::fromUtf8(baOrigData));
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                    if (gpTermSettings->value("DelUWCAfterDownload", "0").toBool() == true && gbIsUWCDownload == true && QFile::exists((gstrTermFilename.lastIndexOf(".") >= 0 ? gstrTermFilename.left(gstrTermFilename.lastIndexOf(".")).append(".uwc") : gstrTermFilename.append(".uwc"))))
                    {
                        //Remove UWC
                        QFile::remove((gstrTermFilename.lastIndexOf(".") >= 0 ? gstrTermFilename.left(gstrTermFilename.lastIndexOf(".")).append(".uwc") : gstrTermFilename.append(".uwc")));
                    }
                    ui->btn_Cancel->setEnabled(false);
                }
            }
            else if (gchTermMode2 == MODE_COMPILE_LOAD_RUN)
            {
                if (gchTermMode == MODE_COMPILE_LOAD_RUN || gchTermMode == MODE_LOAD_RUN)
                {
                    //Run!
                    MainWindow::RunApplication();
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

//=============================================================================
//=============================================================================
void
    MainWindow::on_text_TermEditData_customContextMenuRequested
    (
    const QPoint &pos
    )
{
    //Creates the custom context menu
    gpMenu->popup(ui->text_TermEditData->viewport()->mapToGlobal(pos));
}

void
MainWindow::triggered
    (
    QAction* qaAction
    )
{
    //Runs when a menu item is selected
    if (gspSerialPort.isOpen() == true && gbLoopbackMode == false)
    {
        //Serial is open, allow xcompile functions
        if (qaAction->text() == "XCompile")
        {
            //Compiles SB applet
            MainWindow::CompileApp(MODE_COMPILE);
        }
        else if (qaAction->text() == "XCompile + Load")
        {
            //Compiles and loads a SB applet
            MainWindow::CompileApp(MODE_COMPILE_LOAD);
        }
        else if (qaAction->text() == "XCompile + Load + Run")
        {
            //Compiles, loads and runs a SB applet
            MainWindow::CompileApp(MODE_COMPILE_LOAD_RUN);
        }
        else if (qaAction->text() == "Load" || qaAction->text() == "Load Precompiled BASIC" || qaAction->text() == "Data File +")
        {
            //Just load an application
            MainWindow::CompileApp(MODE_LOAD);
        }
        else if (qaAction->text() == "Load + Run")
        {
            //Load and run an application
            MainWindow::CompileApp(MODE_LOAD_RUN);
        }
#ifdef OnlineXComp
        else if (qaAction->text() == "Online XCompile")
        {
            //Compiles SB applet
            MainWindow::CompileApp(MODE_SERVER_COMPILE);
        }
        else if (qaAction->text() == "Online XCompile + Load")
        {
            //Compiles and loads a SB applet
            MainWindow::CompileApp(MODE_SERVER_COMPILE_LOAD);
        }
        else if (qaAction->text() == "Online XCompile + Load + Run")
        {
            //Compiles, loads and runs a SB applet
            MainWindow::CompileApp(MODE_SERVER_COMPILE_LOAD_RUN);
        }
#endif
    }

    if (qaAction->text() == "Lookup Selected Error-Code (Hex)")
    {
        //Shows a meaning for the error code selected (number in hex)
        bool bTmpBool;
        unsigned int ErrCode = QString("0x").append(ui->text_TermEditData->textCursor().selection().toPlainText()).toUInt(&bTmpBool, 16);
        if (bTmpBool == true)
        {
            //Converted
            MainWindow::LookupErrorCode(ErrCode);
        }
    }
    else if (qaAction->text() == "Lookup Selected Error-Code (Int)")
    {
        //Shows a meaning for the error code selected (number as int)
        MainWindow::LookupErrorCode(ui->text_TermEditData->textCursor().selection().toPlainText().toInt());
    }
    else if (qaAction->text() == "Enable Loopback (Rx->Tx)" || qaAction->text() == "Disable Loopback (Rx->Tx)")
    {
        //Enable/disable loopback mode
        gbLoopbackMode = !gbLoopbackMode;
        if (gbLoopbackMode == true)
        {
            //Enabled
            gbaDisplayBuffer.append("[Loopback Enabled]\n");
            gpMenu->actions()[7]->setText("Disable Loopback (Rx->Tx)");
        }
        else
        {
            //Disabled
            gbaDisplayBuffer.append("[Loopback Disabled]\n");
            gpMenu->actions()[7]->setText("Enable Loopback (Rx->Tx)");
        }
        gtmrTextUpdateTimer.start();
    }
    else if (qaAction->text() == "Erase File" || qaAction->text() == "Erase File +")
    {
        //Erase file
        if (gbTermBusy == false)
        {
            //Not currently busy
            QString strFilename;
            strFilename = QFileDialog::getOpenFileName(this, "Open File", "", "SmartBasic Applications (*.uwc);;All Files (*.*)");

            if (strFilename.length() > 1)
            {
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
                QByteArray baTmpBA = QString("at+del \"").append(strFilename).append("\"").append((qaAction->text() == "Erase File +" ? " +" : "")).toUtf8();
                gspSerialPort.write(baTmpBA);
                gintQueuedTXBytes += baTmpBA.size();
                DoLineEnd();
                gbaDisplayBuffer.append(baTmpBA);
                gtmrTextUpdateTimer.start();
                gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
            }
        }
    }
    else if (qaAction->text() == "Dir")
    {
        //Dir
        if (gbTermBusy == false)
        {
            //List current directory contents
            gspSerialPort.write("at+dir");
            gintQueuedTXBytes += 6;
            DoLineEnd();
            gbaDisplayBuffer.append("at+dir");
            gtmrTextUpdateTimer.start();
            gpMainLog->WriteLogData("at+dir\n");
        }
    }
    else if (qaAction->text() == "Run")
    {
        //Run an application
        if (gbTermBusy == false)
        {
            //Not currently busy
            QString strFilename;
            strFilename = QFileDialog::getOpenFileName(this, "Open File", "", "SmartBasic Applications (*.uwc);;All Files (*.*)");

            if (strFilename.length() > 1)
            {
                //Delete file
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
                QByteArray baTmpBA = QString("at+run \"").append(strFilename).append("\"").toUtf8();
                gspSerialPort.write(baTmpBA);
                gintQueuedTXBytes += baTmpBA.size();
                DoLineEnd();
                gbaDisplayBuffer.append(baTmpBA);
                gtmrTextUpdateTimer.start();
                gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
            }
        }
    }
    else if (qaAction->text() == "Stream File Out")
    {
        //Stream out a file
        if (gbTermBusy == false)
        {
            //Not currently busy
            QString strDataFilename = QFileDialog::getOpenFileName(this, tr("Open File To Stream"), "", tr("Text Files (*.txt);;All Files (*.*)"));;

            if (strDataFilename.length() > 1)
            {
                //File was selected - start streaming it out
                gpStreamFileHandle = new QFile(strDataFilename);

                if (!gpStreamFileHandle->open(QIODevice::ReadOnly))
                {
                    //Unable to open file
                    QString strMessage = tr("Error during file streaming: Access to selected file is denied: ").append(strDataFilename);
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
    else if (qaAction->text() == "Font")
    {
        //Change font
        bool bTmpBool;
        QFont fTmpFont = QFontDialog::getFont(&bTmpBool, ui->text_TermEditData->font(), this);
        if (bTmpBool == true)
        {
            ui->text_TermEditData->setFont(fTmpFont);
        }
    }
    else if (qaAction->text() == "Run")
    {
        //Runs an application
        if (gbTermBusy == false)
        {
            //Not currently busy
            QString strFilename;
            strFilename = QFileDialog::getOpenFileName(this, "Open File", "", "SmartBasic Applications (*.uwc);;All Files (*.*)");

            if (strFilename.length() > 1)
            {
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
            }
        }
    }
    else if (qaAction->text() == "Automation")
    {
        //Show automation window
        guaAutomationForm->show();
    }
    else if (qaAction->text() == "Batch")
    {
        //Start a Batch file script
        if (gbTermBusy == false)
        {
            //Not currently busy
            QString strDataFilename = QFileDialog::getOpenFileName(this, tr("Open Batch File"), "", tr("Text Files (*.txt);;All Files (*.*)"));
            if (strDataFilename.length() > 1)
            {
                //File selected
                gpStreamFileHandle = new QFile(strDataFilename);

                if (!gpStreamFileHandle->open(QIODevice::ReadOnly))
                {
                    //Unable to open file
                    QString strMessage = tr("Error during batch streaming: Access to selected file is denied: ").append(strDataFilename);
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

                //Reads out each block
                QByteArray baFileData = gpStreamFileHandle->readLine().replace("\n", "").replace("\r", "");
                gspSerialPort.write(baFileData);
                gintQueuedTXBytes += baFileData.size();
                MainWindow::DoLineEnd();
                gpMainLog->WriteLogData(QString(baFileData).append("\n"));

                //Start a timeout timer
                gtmrBatchTimeoutTimer.start(BatchTimeout);
            }
        }
    }
    else if (qaAction->text() == "Clear Display")
    {
        //Clear display
        ui->text_TermEditData->ClearDatIn();
    }
    else if (qaAction->text() == "Clear RX/TX count")
    {
        //Clear counts
        gintRXBytes = 0;
        gintTXBytes = 0;
        ui->label_TermRx->setText(QString::number(gintRXBytes));
        ui->label_TermTx->setText(QString::number(gintTXBytes));
    }
    else if (qaAction->text() == "Copy")
    {
        //Copy selected data
        QApplication::clipboard()->setText(ui->text_TermEditData->textCursor().selection().toPlainText());
    }
    else if (qaAction->text() == "Copy All")
    {
        //Copy all data
        QApplication::clipboard()->setText(ui->text_TermEditData->toPlainText());
    }
    else if (qaAction->text() == "Select All")
    {
        //Select all text
        ui->text_TermEditData->selectAll();
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::balloontriggered
    (
    QAction* qaAction
    )
{
    //Runs when a balloon menu item is selected
    if (qaAction->text() == "Show UwTerminalX")
    {
        //Make active window
        this->raise();
        this->activateWindow();
    }
    else if (qaAction->text() == "Exit")
    {
        //Exit
        QApplication::quit();
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::EnterPressed
    (
    )
{
    //Enter pressed in line mode
    if (gspSerialPort.isOpen() == true && gbTermBusy == false && gbLoopbackMode == false)
    {
        QByteArray baTmpBA = ui->text_TermEditData->GetDatOut()->replace("\n", "").replace("\r", "").toUtf8();
        gspSerialPort.write(baTmpBA);
        gintQueuedTXBytes += baTmpBA.size();
        /*if (ui->check_Echo->isChecked() == true)
        {
            baTmpBA = ui->text_TermEditData->toPlainText().toUtf8();
            if (ui->check_ShowCLRF->isChecked() == true)
            {
                //Escape \t, \r and \n
                baTmpBA.replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n");
            }
            baTmpBA.replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f");
            gbaDisplayBuffer.append(baTmpBA.append("\n"));

            if (!gtmrTextUpdateTimer.isActive())
            {
                gtmrTextUpdateTimer.start();
            }
        }*/
        //gpMainLog->WriteLogData(ui->text_TermEditData->GetDatOut().append("\n"));
        //ui->text_TermEditData->setPlainText("");
        MainWindow::DoLineEnd();
    }
    else if (gspSerialPort.isOpen() == true && gbLoopbackMode == true)
    {
        //Loopback is enabled
        gbaDisplayBuffer.append("[Cannot send: Loopback mode is enabled.]");
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
        ui->text_TermEditData->ClearDatOut();
    }
}

//=============================================================================
//=============================================================================
unsigned char
MainWindow::CompileApp
    (
    unsigned char chMode
    )
{
    //Runs when an application is to be compiled
    gchTermMode = chMode;
    gstrTermFilename = QFileDialog::getOpenFileName(this, (chMode == 6 || chMode == 7 ? tr("Open File") : (chMode == MODE_LOAD || chMode == MODE_LOAD_RUN ? tr("Open SmartBasic Application") : tr("Open SmartBasic Source"))), "", (chMode == 6 || chMode == 7 ? tr("All Files (*.*)") : (chMode == MODE_LOAD || chMode == MODE_LOAD_RUN ? tr("SmartBasic Applications (*.uwc);;All Files (*.*)") : tr("Text/SmartBasic Files (*.txt *.sb);;All Files (*.*)"))));
    if (gstrTermFilename != "")
    {
        if (chMode == MODE_LOAD || chMode == MODE_LOAD_RUN)
        {
            //Loading a compiled application
            gbTermBusy = true;
            MainWindow::LoadFile(false);

            //Download to the device
            gchTermMode2 = MODE_COMPILE;
            QByteArray baTmpBA = QString("AT+DEL \"").append(gstrDownloadFilename).append("\" +").toUtf8();
            gspSerialPort.write(baTmpBA);
            gintQueuedTXBytes += baTmpBA.size();
            MainWindow::DoLineEnd();
            gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));

            //Start the timeout timer
            gtmrDownloadTimeoutTimer.start();
        }
        else if (chMode == 6 || chMode == 7)
        {
            //Download any file to device
            gbTermBusy = true;
            MainWindow::LoadFile(false);

            //Download to the device
            gchTermMode2 = MODE_COMPILE;
            QByteArray baTmpBA = QString("AT+DEL \"").append(gstrDownloadFilename).append("\" +").toUtf8();
            gspSerialPort.write(baTmpBA);
            gintQueuedTXBytes += baTmpBA.size();
            MainWindow::DoLineEnd();
            gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));

            //Start the timeout timer
            gtmrDownloadTimeoutTimer.start();
        }
#ifdef OnlineXComp
        else if (chMode == MODE_SERVER_COMPILE || chMode == MODE_SERVER_COMPILE_LOAD || chMode == MODE_SERVER_COMPILE_LOAD_RUN)
        {
            //Check if device is supported for Online XCompile
            gbTermBusy = true;
            gchTermMode2 = 0;
            gchTermBusyLines = 0;
            gstrTermBusyData = tr("");
            gspSerialPort.write("at i 0");
            gintQueuedTXBytes += 6;
            MainWindow::DoLineEnd();
            gpMainLog->WriteLogData("at i 0\n");
            gspSerialPort.write("at i 13");
            gintQueuedTXBytes += 7;
            MainWindow::DoLineEnd();
            gpMainLog->WriteLogData("at i 13\n");
        }
#endif
        else
        {
            //A file was selected, get the version number
            gbTermBusy = true;
            gchTermMode2 = 0;
            gchTermBusyLines = 0;
            gstrTermBusyData = tr("");
            gspSerialPort.write("at i 0");
            gintQueuedTXBytes += 6;
            MainWindow::DoLineEnd();
            gpMainLog->WriteLogData("at i 0\n");
            gspSerialPort.write("at i 13");
            gintQueuedTXBytes += 7;
            MainWindow::DoLineEnd();
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
MainWindow::UpdateImages
    (
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
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::KeyPressed
    (
    int intKeyValue
    )
{
    //Key pressed, send it out
    if (gbTermBusy == false && gbLoopbackMode == false)
    {
        if (ui->check_Echo->isChecked() == true)
        {
            //Echo mode on
            if (intKeyValue == Qt::Key_Enter || intKeyValue == Qt::Key_Return)
            {
                gbaDisplayBuffer.append("\n");
            }
            else
            {
                /*if (ui->check_ShowCLRF->isChecked() == true)
                {
                    //Escape \t, \r and \n in addition to normal escaping
                    gbaDisplayBuffer.append(QString(QChar(intKeyValue)).toUtf8().replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n").replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f"));
                }
                else
                {
                    //Normal escaping
                    gbaDisplayBuffer.append(QString(QChar(intKeyValue)).toUtf8().replace('\0', "\\00").replace("\x01", "\\01").replace("\x02", "\\02").replace("\x03", "\\03").replace("\x04", "\\04").replace("\x05", "\\05").replace("\x06", "\\06").replace("\x07", "\\07").replace("\x08", "\\08").replace("\x0b", "\\0B").replace("\x0c", "\\0C").replace("\x0e", "\\0E").replace("\x0f", "\\0F").replace("\x10", "\\10").replace("\x11", "\\11").replace("\x12", "\\12").replace("\x13", "\\13").replace("\x14", "\\14").replace("\x15", "\\15").replace("\x16", "\\16").replace("\x17", "\\17").replace("\x18", "\\18").replace("\x19", "\\19").replace("\x1a", "\\1a").replace("\x1b", "\\1b").replace("\x1c", "\\1c").replace("\x1d", "\\1d").replace("\x1e", "\\1e").replace("\x1f", "\\1f"));
                }*/
            }
            if (!gtmrTextUpdateTimer.isActive())
            {
                gtmrTextUpdateTimer.start();
            }
        }

        //Character mode, send right on
        if (intKeyValue == Qt::Key_Enter || intKeyValue == Qt::Key_Return)
        {
            //Return key
            gpMainLog->WriteLogData("\n");
            MainWindow::DoLineEnd();
        }
        else
        {
            //Not return
            QByteArray baTmpBA = QString(QChar(intKeyValue)).toUtf8();
            gspSerialPort.write(baTmpBA);
            gintQueuedTXBytes += baTmpBA.size();
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
            if (!gtmrTextUpdateTimer.isActive())
            {
                gtmrTextUpdateTimer.start();
            }
            gpMainLog->WriteLogData(QString(QChar(intKeyValue)).toUtf8());
        }
    }
    else if (gbLoopbackMode == true)
    {
        //Loopback is enabled
        gbaDisplayBuffer.append("[Cannot send: Loopback mode is enabled.]");
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::DoLineEnd
    (
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
MainWindow::DevRespTimeout
    (
    )
{
    //Runs to indiciate a device timeout when waiting to compile an application
    if (gbTermBusy == true)
    {
        //Update buffer
        gbaDisplayBuffer.append(">TIMEOUT OCCURED!\n");
        gtmrTextUpdateTimer.start();

        //Reset variables
        gbTermBusy = false;
        gchTermBusyLines = 0;
        gstrTermBusyData = tr("");
        ui->btn_Cancel->setEnabled(false);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::process_finished
    (
    int intExitCode,
    QProcess::ExitStatus esExitStatus
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
        if (gchTermMode == MODE_COMPILE_LOAD || gchTermMode == MODE_COMPILE_LOAD_RUN)
        {
            //Load the file
            MainWindow::LoadFile(true);
            gchTermMode2 = MODE_COMPILE;

            //Download to the device
            QByteArray baTmpBA = QString("AT+DEL \"").append(gstrDownloadFilename).append("\" +").toUtf8();
            gspSerialPort.write(baTmpBA);
            gintQueuedTXBytes += baTmpBA.size();
            MainWindow::DoLineEnd();
            gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));
            gtmrDownloadTimeoutTimer.start();
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

//=============================================================================
//=============================================================================
void
MainWindow::SerialStatus
    (
    bool bType
    )
{
    if (gspSerialPort.isOpen() == true)
    {
        if ((gspSerialPort.pinoutSignals() & QSerialPort::ClearToSendSignal) != gbCTSStatus || bType == true)
        {
            //CTS changed
            gbCTSStatus = (gspSerialPort.pinoutSignals() & QSerialPort::ClearToSendSignal);
            ui->image_CTS->setPixmap((gbCTSStatus == true ? *gpGreenCirclePixmap : *gpRedCirclePixmap));
        }
        if ((gspSerialPort.pinoutSignals() & QSerialPort::DataCarrierDetectSignal) != gbDCDStatus || bType == true)
        {
            //DCD changed
            gbDCDStatus = (gspSerialPort.pinoutSignals() & QSerialPort::DataCarrierDetectSignal);
            ui->image_DCD->setPixmap((gbDCDStatus == true ? *gpGreenCirclePixmap : *gpRedCirclePixmap));
        }
        if ((gspSerialPort.pinoutSignals() & QSerialPort::DataSetReadySignal) != gbDSRStatus || bType == true)
        {
            //DSR changed
            gbDSRStatus = (gspSerialPort.pinoutSignals() & QSerialPort::DataSetReadySignal);
            ui->image_DSR->setPixmap((gbDSRStatus == true ? *gpGreenCirclePixmap : *gpRedCirclePixmap));
        }
        if ((gspSerialPort.pinoutSignals() & QSerialPort::RingIndicatorSignal) != gbRIStatus || bType == true)
        {
            //RI changed
            gbRIStatus = (gspSerialPort.pinoutSignals() & QSerialPort::RingIndicatorSignal);
            ui->image_RI->setPixmap((gbRIStatus == true ? *gpGreenCirclePixmap : *gpRedCirclePixmap));
        }
    }
    return;
}

//=============================================================================
//=============================================================================
void
MainWindow::SerialStatusSlot
    (
    )
{
    //Slot function to update serial pinout status
    MainWindow::SerialStatus(0);
}

//=============================================================================
//=============================================================================
void
MainWindow::OpenSerial
    (
    )
{
    //Function to open serial port
    if (gspSerialPort.isOpen() == true)
    {
        //Serial port is already open - close
        while (gspSerialPort.isOpen() == true)
        {
            gspSerialPort.clear();
            gspSerialPort.close();
        }
        gpSignalTimer->stop();

        //Change status message
        ui->statusBar->showMessage("");

        //Notify automation form
        guaAutomationForm->ConnectionChange(false);

        //Update images
        MainWindow::UpdateImages();
    }

    gspSerialPort.setPortName(ui->combo_COM->currentText());
    gspSerialPort.setBaudRate(ui->combo_Baud->currentText().toInt());
    gspSerialPort.setDataBits((QSerialPort::DataBits)ui->combo_Data->currentText().toInt());
    gspSerialPort.setStopBits((QSerialPort::StopBits)ui->combo_Stop->currentText().toInt());

    //Parity
    gspSerialPort.setParity((ui->combo_Parity->currentIndex() == 1 ? QSerialPort::OddParity : (ui->combo_Parity->currentIndex() == 2 ? QSerialPort::EvenParity : QSerialPort::NoParity)));

    //Flow control
    gspSerialPort.setFlowControl((ui->combo_Handshake->currentIndex() == 2 ? QSerialPort::HardwareControl : (ui->combo_Handshake->currentIndex() == 1 ? QSerialPort::SoftwareControl : QSerialPort::NoFlowControl)));

    if (gspSerialPort.open(QIODevice::ReadWrite))
    {
        //Successful
        ui->statusBar->showMessage(QString("[").append(ui->combo_COM->currentText()).append(":").append(ui->combo_Baud->currentText()).append(",").append((ui->combo_Parity->currentIndex() == 0 ? "N" : ui->combo_Parity->currentIndex() == 1 ? "O" : ui->combo_Parity->currentIndex() == 2 ? "E" : "")).append(",").append(ui->combo_Data->currentText()).append(",").append(ui->combo_Stop->currentText()).append(",").append((ui->combo_Handshake->currentIndex() == 0 ? "N" : ui->combo_Handshake->currentIndex() == 1 ? "S" : ui->combo_Handshake->currentIndex() == 2 ? "H" : "")).append("]{").append((ui->radio_LCR->isChecked() ? "cr" : (ui->radio_LLF->isChecked() ? "lf" : (ui->radio_LCRLF->isChecked() ? "cr lf" : (ui->radio_LLFCR->isChecked() ? "lf cr" : ""))))).append("}"));
        ui->label_TermConn->setText(ui->statusBar->currentMessage());

        //Switch to Terminal tab
        ui->selector_Tab->setCurrentIndex(0);

        //Disable read-only mode
        ui->text_TermEditData->setReadOnly(false);

        //DTR
        gspSerialPort.setDataTerminalReady(ui->check_DTR->isChecked());

        //Flow control
        if (ui->combo_Handshake->currentIndex() != 2)
        {
            //Not hardware handshaking - RTS
            ui->check_RTS->setEnabled(true);
            gspSerialPort.setRequestToSend(ui->check_RTS->isChecked());
        }

        //Break
        gspSerialPort.setBreakEnabled(ui->check_Break->isChecked());

        //Enable checkboxes
        ui->check_Break->setEnabled(true);
        ui->check_DTR->setEnabled(true);
        ui->check_Echo->setEnabled(true);
        ui->check_Line->setEnabled(true);

        //Update button text
        ui->btn_TermClose->setText("C&lose");

        //Signal checking
        MainWindow::SerialStatus(1);

        //Enable timer
        gpSignalTimer->start(gpTermSettings->value("SerialSignalCheckInterval", "50").toUInt());

        //Notify automation form
        guaAutomationForm->ConnectionChange(true);

        //Notify scroll edit
        ui->text_TermEditData->SetSerialOpen(true);

        //Set focus to input text edit
        ui->text_TermEditData->setFocus();

        //Allow file drops for uploads
        setAcceptDrops(true);
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
        .append(" and try again.");
        ;
        gpmErrorForm->show();
        gpmErrorForm->SetMessage(&strMessage);
        ui->text_TermEditData->SetSerialOpen(false);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::LoadFile
    (
    bool bToUWC
    )
{
    //Load
    QFile file((bToUWC == true ? (gstrTermFilename.lastIndexOf(".") >= 0 ? gstrTermFilename.left(gstrTermFilename.lastIndexOf(".")).append(".uwc") : gstrTermFilename.append(".uwc")) : gstrTermFilename));
    if (!file.open(QIODevice::ReadOnly))
    {
        //Unable to open file
        QString strMessage = tr("Error during XCompile: Access to selected file is denied: ").append((bToUWC ? (gstrTermFilename.lastIndexOf(".") >= 0 ? gstrTermFilename.left(gstrTermFilename.lastIndexOf(".")).append(".uwc") : gstrTermFilename.append(".uwc")) : gstrTermFilename));
        gpmErrorForm->show();
        gpmErrorForm->SetMessage(&strMessage);
        gbTermBusy = false;
        ui->btn_Cancel->setEnabled(false);
        return;
    }

    //Is this a UWC download?
    gbIsUWCDownload = bToUWC;

    //Create a data stream and hex string holder
    QDataStream in(&file);
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
    file.close();

//TODO: FIX THIS! Get the last slash and check for dot after
    //Calculate the filename
    if (gstrTermFilename.lastIndexOf(".") >= 0)
    {
        //Get up until the first dot
        QRegularExpression reTempRE("^(.*)/(.*)$");
        QRegularExpressionMatch remTempREM = reTempRE.match(gstrTermFilename);
        if (remTempREM.hasMatch() == true)
        {
            //Got a match
            gstrDownloadFilename = remTempREM.captured(2);
            if (gstrDownloadFilename.count(".") > 0)
            {
                //Strip off after the dot
                gstrDownloadFilename = gstrDownloadFilename.left(gstrDownloadFilename.indexOf("."));
            }
        }
    }
    else
    {
        //Use the whole name
        gstrDownloadFilename = gstrTermFilename;
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::RunApplication
    (
    )
{
    //Runs an application
    QByteArray baTmpBA = QString("AT+RUN \"").append(gstrDownloadFilename).append("\"").toUtf8();
    gspSerialPort.write(baTmpBA);
    gintQueuedTXBytes += baTmpBA.size();
    MainWindow::DoLineEnd();

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
MainWindow::on_check_Break_stateChanged
    (
    )
{
    //Break status changed
    gspSerialPort.setBreakEnabled(ui->check_Break->isChecked());
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_RTS_stateChanged
    (
    )
{
    //RTS status changed
    gspSerialPort.setRequestToSend(ui->check_RTS->isChecked());
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_DTR_stateChanged
    (
    )
{
    //DTR status changed
    gspSerialPort.setDataTerminalReady(ui->check_DTR->isChecked());
}

//=============================================================================
//=============================================================================
void
    MainWindow::on_check_Line_stateChanged
    (
    )
{
    //Line mode status changed
    ui->text_TermEditData->SetLineMode(ui->check_Line->isChecked());
}

//=============================================================================
//=============================================================================
void
MainWindow::SerialError
    (
    QSerialPort::SerialPortError speErrorCode
    )
{
    if (speErrorCode == QSerialPort::NoError)
    {
        //No error. Why this is ever emitted is a mystery to me.
        return;
    }
    else if (speErrorCode == QSerialPort::ParityError)
    {
        //Parity error
    }
    else if (speErrorCode == QSerialPort::FramingError)
    {
        //Framing error
    }
    else if (speErrorCode == QSerialPort::ResourceError || speErrorCode == QSerialPort::PermissionError)
    {
        //Resource error or permission error (device unplugged?)
        QString strMessage = tr("Fatal error with serial connection.\nPlease reconnect to the device to continue.");
        gpmErrorForm->show();
        gpmErrorForm->SetMessage(&strMessage);
        ui->text_TermEditData->SetSerialOpen(false);

        //Disable timer
        gpSignalTimer->stop();

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

        //No longer busy
        gbTermBusy = false;
        gchTermMode = 0;
        gchTermMode2 = 0;

        //Disable active checkboxes
        ui->check_Break->setEnabled(false);
        ui->check_DTR->setEnabled(false);
        ui->check_Echo->setEnabled(false);
        ui->check_Line->setEnabled(false);
        ui->check_RTS->setEnabled(false);

        //Disable text entry
        ui->text_TermEditData->setReadOnly(true);

        //Change status message
        ui->statusBar->showMessage("");

        //Change button text
        ui->btn_TermClose->setText("&Open");

        //Update images
        MainWindow::UpdateImages();

        //Notify automation form
        guaAutomationForm->ConnectionChange(false);

        //Show disconnection balloon
        if (gbSysTrayEnabled == true && !this->isActiveWindow() && !gpmErrorForm->isActiveWindow() && !guaAutomationForm->isActiveWindow())
        {
            gpSysTray->showMessage(ui->combo_COM->currentText().append(" Removed"), QString("Connection to device ").append(ui->combo_COM->currentText()).append(" has been lost due to disconnection."), QSystemTrayIcon::Critical);
        }

        if (ui->check_Poll->isChecked())
        {
            //Poll mode is enabled, start timer
            gtmrPollTimer.start();
        }

        //Disallow file drops
        setAcceptDrops(false);
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Duplicate_clicked
    (
    )
{
    //Duplicates instance of UwTerminalX
    QProcess DuplicateProcess;
    DuplicateProcess.startDetached(QCoreApplication::applicationFilePath(), QStringList() << "ACCEPT" << tr("COM=").append(ui->combo_COM->currentText()) << tr("BAUD=").append(ui->combo_Baud->currentText()) << tr("STOP=").append(ui->combo_Stop->currentText()) << tr("DATA=").append(ui->combo_Data->currentText()) << tr("PAR=").append(ui->combo_Parity->currentText()) << tr("FLOW=").append(QString::number(ui->combo_Handshake->currentIndex())) << tr("ENDCHR=").append((ui->radio_LCR->isChecked() == true ? "0" : ui->radio_LLF->isChecked() == true ? "1" : ui->radio_LCRLF->isChecked() == true ? "2" : "3")) << tr("LOCALECHO=").append((ui->check_Echo->isChecked() == true ? "1" : "0")) << tr("LINEMODE=").append((ui->check_Line->isChecked() == true ? "1" : "0")));
}

//=============================================================================
//=============================================================================
void
MainWindow::MessagePass
    (
    QString strDataString,
    bool bEscapeString
    )
{
    //Receive a command from the automation window
    if (gspSerialPort.isOpen() == true && gbTermBusy == false && gbLoopbackMode == false)
    {
        if (bEscapeString == true)
        {
            //Escape string sequences - This could probably be optomised or done a better way
            QRegularExpression reESeq("[\\\\]([0-9A-Fa-f]{2})");
            QRegularExpressionMatchIterator remiESeqMatch = reESeq.globalMatch(strDataString);
            while (remiESeqMatch.hasNext())
            {
                //We have escape sequences
                bool bSuccess;
                QRegularExpressionMatch remThisESeqMatch = remiESeqMatch.next();
                strDataString.replace(QString("\\").append(remThisESeqMatch.captured(1)), QString((char)remThisESeqMatch.captured(1).toInt(&bSuccess, 16)));
            }
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
            gtmrTextUpdateTimer.start();
        }
        gpMainLog->WriteLogData(strDataString);
        MainWindow::DoLineEnd();
    }
    else if (gspSerialPort.isOpen() == true && gbLoopbackMode == true)
    {
        //Loopback is enabled
        gbaDisplayBuffer.append("[Cannot send: Loopback mode is enabled.]\n");
        gtmrTextUpdateTimer.start();
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::LookupErrorCode
    (
    unsigned int intErrorCode
    )
{
    //Looks up an error code and outputs it in the edit (does NOT store it to the log)
    gbaDisplayBuffer.append(gpErrorMessages->value(QString::number(intErrorCode), "Undefined Error Code").toString().append("\n"));
    gtmrTextUpdateTimer.start();
    ui->text_TermEditData->moveCursor(QTextCursor::End);
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Default600_clicked
    (
    )
{
    //Load defaults for BL600/BL620
    ui->combo_Baud->setCurrentIndex(3);
    ui->combo_Parity->setCurrentIndex(0);
    ui->combo_Stop->setCurrentIndex(0);
    ui->combo_Data->setCurrentIndex(1);
    ui->combo_Handshake->setCurrentIndex(2);
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Default620US_clicked
    (
    )
{
    //Load defaults for BL620-US
    ui->combo_Baud->setCurrentIndex(3);
    ui->combo_Parity->setCurrentIndex(0);
    ui->combo_Stop->setCurrentIndex(0);
    ui->combo_Data->setCurrentIndex(1);
    ui->combo_Handshake->setCurrentIndex(0);
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Default900_clicked
    (
    )
{
    //Load defaults for BT900
    ui->combo_Baud->setCurrentIndex(8);
    ui->combo_Parity->setCurrentIndex(0);
    ui->combo_Stop->setCurrentIndex(0);
    ui->combo_Data->setCurrentIndex(1);
    ui->combo_Handshake->setCurrentIndex(2);
}

//=============================================================================
//=============================================================================
void
MainWindow::SerialBytesWritten
    (
    qint64 intByteCount
    )
{
    //Updates the display with the number of bytes written
    gintTXBytes += intByteCount;
    ui->label_TermTx->setText(QString::number(gintTXBytes));
//    ui->label_TermQueue->setText(QString::number(gintQueuedTXBytes));

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
            gbaDisplayBuffer.append(QString("Streamed ").append(QString::number(gintStreamBytesRead)).append(" bytes.\n"));
            gtmrTextUpdateTimer.start();
            gintStreamBytesProgress = gintStreamBytesProgress + StreamProgress;
        }
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_Cancel_clicked
    (
    )
{
    //Cancel current stream or file download
    if (gbTermBusy == true && gchTermMode > 0 && gchTermMode < 8)
    {
        //Cancel download
        gtmrDownloadTimeoutTimer.stop();
        gstrHexData = "";
        gbaDisplayBuffer.append("-- File download cancelled --\n");
        gtmrTextUpdateTimer.start();
        gspSerialPort.write("AT+FCL");
        gintQueuedTXBytes += 6;
        MainWindow::DoLineEnd();
        gpMainLog->WriteLogData("AT+FCL\n");
        if (gpTermSettings->value("DelUWCAfterDownload", "0").toBool() == true && gbIsUWCDownload == true && QFile::exists((gstrTermFilename.lastIndexOf(".") >= 0 ? gstrTermFilename.left(gstrTermFilename.lastIndexOf(".")).append(".uwc") : gstrTermFilename.append(".uwc"))))
        {
            //Remove UWC
            QFile::remove((gstrTermFilename.lastIndexOf(".") >= 0 ? gstrTermFilename.left(gstrTermFilename.lastIndexOf(".")).append(".uwc") : gstrTermFilename.append(".uwc")));
        }
        gchTermMode = 0;
        gchTermMode2 = 0;
        gbTermBusy = false;
    }
    else if (gbTermBusy == true && gbStreamingFile == true)
    {
        //Cancel stream
        FinishStream(true);
    }
    else if (gbTermBusy == true && gbStreamingBatch == true)
    {
        //Cancel batch streaming
        FinishBatch(true);
    }

    //Disable button
    ui->btn_Cancel->setEnabled(false);
}

//=============================================================================
//=============================================================================
void
MainWindow::FinishStream
    (
    bool bType
    )
{
    //Sending a file stream has finished
    if (bType == true)
    {
        //Stream cancelled
        gbaDisplayBuffer.append(QString("Cancelled stream after ").append(QString::number(gintStreamBytesRead)).append(" bytes (").append(QString::number(1+(gtmrStreamTimer.nsecsElapsed()/1000000000))).append(" seconds) [~").append(QString::number((gintStreamBytesRead/(1+gtmrStreamTimer.nsecsElapsed()/1000000000)))).append(" bytes/second].\n"));
    }
    else
    {
        //Stream finished
        gbaDisplayBuffer.append(QString("Finished streaming file, ").append(QString::number(gintStreamBytesRead)).append(" bytes sent in ").append(QString::number(1+(gtmrStreamTimer.nsecsElapsed()/1000000000))).append(" seconds [~").append(QString::number((gintStreamBytesRead/(1+gtmrStreamTimer.nsecsElapsed()/1000000000)))).append(" bytes/second].\n"));
    }

    //Initiate timer for buffer update
    gtmrTextUpdateTimer.start();

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
void MainWindow::FinishBatch
    (
    bool bType
    )
{
    //Sending a file stream has finished
    if (bType == true)
    {
        //Stream cancelled
        gbaDisplayBuffer.append(QString("Cancelled batch (").append(QString::number(1+(gtmrStreamTimer.nsecsElapsed()/1000000000))).append(" seconds)\n"));
    }
    else
    {
        //Stream finished
        gbaDisplayBuffer.append(QString("Finished batch file, ").append(QString::number(1+(gtmrStreamTimer.nsecsElapsed()/1000000000))).append(" seconds\n"));
    }

    //Initiate timer for buffer update
    gtmrTextUpdateTimer.start();

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
MainWindow::UpdateReceiveText
    (
    )
{
    //Updates the receive text buffer
    ui->text_TermEditData->AddDatInText(&gbaDisplayBuffer);
    gbaDisplayBuffer.clear();
}

//=============================================================================
//=============================================================================
void
MainWindow::BatchTimeoutSlot
    (
    )
{
    //A response to a batch command has timed out
    gbaDisplayBuffer.append("TIMEOUT\n");
    gtmrTextUpdateTimer.start();
    gbTermBusy = false;
    gbStreamingBatch = false;
    gchTermMode = 0;
    gpStreamFileHandle->close();
    delete gpStreamFileHandle;
}

//=============================================================================
//=============================================================================
void MainWindow::on_combo_COM_currentIndexChanged
    (
    int index
    )
{
    //Serial port selection has been changed, update text
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
        ui->label_SerialInfo->setText("Invalid");
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::PollUSB
    (
    )
{
    //Polls if USB device exists and reopens it if it does
    QSerialPortInfo SerialInfo(ui->combo_COM->currentText());
    if (SerialInfo.isValid())
    {
        //Exists - cancel timer and reopen port
        gtmrPollTimer.stop();
        OpenSerial();
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::dragEnterEvent
    (
    QDragEnterEvent *event
    )
{
    //A file is being dragged onto the window
    if (event->mimeData()->urls().count() == 1 && gbTermBusy == false && gspSerialPort.isOpen() == true)
    {
        //Nothing is running, serial handle is open and a single file is being dragged - accept action
        event->acceptProposedAction();
    }
}

//=============================================================================
//=============================================================================
void
MainWindow::dropEvent
    (
    QDropEvent *event
    )
{
    //A file has been dragged onto the window - send this file if possible
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
    {
        //No files
        return;
    }
    else if (urls.length() > 1)
    {
        //More than 1 file - ignore
        return;
    }

    QString strFileName = urls.first().toLocalFile();
    if (strFileName.isEmpty())
    {
        //Invalid filename
        return;
    }

    //Send file to device
    gchTermMode = 4;
    gstrTermFilename = strFileName;
    gbTermBusy = true;
    MainWindow::LoadFile(false);

    //Download to the device
    gchTermMode2 = 1;
    QByteArray baTmpBA = QString("AT+DEL \"").append(gstrDownloadFilename).append("\" +").toUtf8();
    gspSerialPort.write(baTmpBA);
    gintQueuedTXBytes += baTmpBA.size();
    MainWindow::DoLineEnd();
    gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));

    //Start the timeout timer
    gtmrDownloadTimeoutTimer.start();

    //Enable cancel button
    ui->btn_Cancel->setEnabled(true);
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_PreXCompRun_stateChanged
(
    int iChecked
)
{
    //User has changed running pre/post XCompiler executable, update GUI
    if (iChecked == 2)
    {
        //Enabled
        ui->check_PreXCompFail->setDisabled(false);
        ui->radio_XCompPost->setDisabled(false);
        ui->radio_XCompPre->setDisabled(false);
        ui->edit_PreXCompFilename->setDisabled(false);
        ui->btn_PreXCompSelect->setDisabled(false);
        ui->label_PreXCompInfo->setDisabled(false);
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
        gpTermSettings->setValue("PrePostXCompRun", "0");
    }
}

//=============================================================================
//=============================================================================
bool
MainWindow::RunPrePostExecutable
(
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
MainWindow::on_btn_PreXCompSelect_clicked
(
)
{
    //Opens an executable selector
    QString strFilename;
    strFilename = QFileDialog::getOpenFileName(this, "Open Executable/batch", "", "Executables/Batch/Bash files (*.exe *.bat *.sh);;All Files (*.*)");

    if (strFilename.length() > 1)
    {
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
MainWindow::on_radio_XCompPre_toggled
(
    bool bChecked
)
{
    //Pre/post XCompiler run time changed to run before XCompiler - update settings
    gpTermSettings->setValue("PrePostXCompMode", "0");
}

//=============================================================================
//=============================================================================
void
MainWindow::on_radio_XCompPost_toggled
(
    bool bChecked
)
{
    //Pre/post XCompiler run time changed to run after XCompiler - update settings
    gpTermSettings->setValue("PrePostXCompMode", "1");
}

//=============================================================================
//=============================================================================
void
MainWindow::on_check_PreXCompFail_stateChanged
(
    int bChecked
)
{
    //Pre/post XCompiler run if XCompiler failed changed - update settings
    gpTermSettings->setValue("PrePostXCompFail", ui->check_PreXCompFail->isChecked());
}

//=============================================================================
//=============================================================================
void
MainWindow::on_edit_PreXCompFilename_editingFinished
(
)
{
    //Pre/post XCompiler executable changed - update settings
    gpTermSettings->setValue("PrePostXCompPath", ui->edit_PreXCompFilename->text());
}

//=============================================================================
//=============================================================================
void
MainWindow::on_btn_GitHub_clicked
(
)
{
    //Open webpage at the UwTerminalX github page)
    QDesktopServices::openUrl(QUrl("https://github.com/LairdCP/UwTerminalX"));
}

#ifdef OnlineXComp
//=============================================================================
//=============================================================================
void
MainWindow::replyFinished
(
QNetworkReply* nrReply
)
{
    //Response received from server regarding online XCompilation
    if (gchTermMode2 == 0)
    {
        //Check if device is supported
        QJsonParseError JsonError;
        QJsonDocument JsonData = QJsonDocument::fromJson(nrReply->readAll(), &JsonError);
        if (JsonError.error == QJsonParseError::NoError)
        {
            //Decoded JSON
            QJsonObject JsonObject = JsonData.object();
            if (nrReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 503)
            {
                //Server responded with error
                QString strMessage = QString("Server responded with error code ").append(JsonObject["Result"].toString()).append("; ").append(JsonObject["Error"].toString());
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
            }
            else if (nrReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200)
            {
                //Server responded with OK
                if (JsonObject["Result"].toString() == "1")
                {
                    //Device supported
                    gstrDeviceID = JsonObject["ID"].toString();
                    gchTermMode2 = 9;

                    //Compile application
                    QNetworkRequest ThisReq(QUrl(QString("http://").append(ServerHost).append("/xcompile.php?JSON=1")));
                    QByteArray baPostData;
                    baPostData.append("-----------------------------17192614014659\r\nContent-Disposition: form-data; name=\"file_XComp\"\r\n\r\n").append(JsonObject["ID"].toString()).append("\r\n");
                    baPostData.append("-----------------------------17192614014659\r\nContent-Disposition: form-data; name=\"file_sB\"; filename=\"test.sb\"\r\nContent-Type: application/octet-stream\r\n\r\n");

                    //Add file data here
                    QFile file(gstrTermFilename);
                    if (!file.open(QIODevice::ReadOnly))
                    {
                        return;
                    }

                    while (!file.atEnd())
                    {
                        baPostData.append(file.readAll());
                    }

                    baPostData.append("\r\n-----------------------------17192614014659--\r\n");
                    ThisReq.setRawHeader("Content-Type", "multipart/form-data; boundary=---------------------------17192614014659");
                    ThisReq.setRawHeader("Content-Length", QString(baPostData.length()).toUtf8());
                    gnmManager->post(ThisReq, baPostData);
                    ui->statusBar->showMessage("Sending smartBASIC application for online compilation...", 500);
                }
                else
                {
                    //Device should be supported but something went wrong...
                    QString strMessage = QString("Unfortunately your device is not supported for online XCompiling.");
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                }
            }
            else
            {
                //Unknown response
                QString strMessage = QString("Server responded with unknown response");
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
            }
        }
        else
        {
            //Error whilst decoding JSON
            QString strMessage = QString("Error: Unable to decode server JSON response, debug: ").append(JsonError.errorString());
            gpmErrorForm->show();
            gpmErrorForm->SetMessage(&strMessage);
        }
    }
    else if (gchTermMode2 == MODE_SERVER_COMPILE)
    {
        //XCompile result
        if (nrReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 503)
        {
            //Error compiling
            QJsonParseError JsonError;
            QJsonDocument JsonData = QJsonDocument::fromJson(nrReply->readAll(), &JsonError);
            if (JsonError.error == QJsonParseError::NoError)
            {
                //Decoded JSON
                QJsonObject JsonObject = JsonData.object();

                //Server responded with error
                if (JsonObject["Result"].toString() == "-9")
                {
                    //Error whilst compiling, show results
                    QString strMessage = QString("Failed to compile ").append(JsonObject["Result"].toString()).append("; ").append(JsonObject["Error"].toString().append("\r\n").append(JsonObject["Description"].toString()));
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                }
                else
                {
                    //Server responded with error
                    QString strMessage = QString("Server responded with error code ").append(JsonObject["Result"].toString()).append("; ").append(JsonObject["Error"].toString());
                    gpmErrorForm->show();
                    gpmErrorForm->SetMessage(&strMessage);
                }
            }
            else
            {
                //Error whilst decoding JSON
                QString strMessage = QString("Unable to decode JSON data from server, debug data: ").append(JsonData.toBinaryData());
                gpmErrorForm->show();
                gpmErrorForm->SetMessage(&strMessage);
            }
        }
        else if (nrReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200)
        {
            //Compiled - save file
            if (!QFile::exists(gstrTermFilename.lastIndexOf(".") >= 0 ? gstrTermFilename.left(gstrTermFilename.lastIndexOf(".")).append(".uwc") : gstrTermFilename.append(".uwc")))
            {
                //Remove file
                QFile::remove(gstrTermFilename.lastIndexOf(".") >= 0 ? gstrTermFilename.left(gstrTermFilename.lastIndexOf(".")).append(".uwc") : gstrTermFilename.append(".uwc"));
            }

            QFile file(gstrTermFilename.lastIndexOf(".") >= 0 ? gstrTermFilename.left(gstrTermFilename.lastIndexOf(".")).append(".uwc") : gstrTermFilename.append(".uwc"));
            if (file.open(QIODevice::WriteOnly))
            {
                file.write(nrReply->readAll());
            }
            file.flush();
            file.close();

            gstrTermFilename = gstrTermFilename.lastIndexOf(".") >= 0 ? gstrTermFilename.left(gstrTermFilename.lastIndexOf(".")).append(".uwc") : gstrTermFilename.append(".uwc");

            if (gchTermMode == MODE_SERVER_COMPILE)
            {
                //Done
                gtmrDownloadTimeoutTimer.stop();
                gstrHexData = "";
                gbaDisplayBuffer.append("-- Online XCompile complete --\n");
                gtmrTextUpdateTimer.start();
                gchTermMode = 0;
                gchTermMode2 = 0;
                gbTermBusy = false;

                //Disable button
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
                MainWindow::LoadFile(false);

                //Download to the device
                gchTermMode2 = MODE_COMPILE;
                QByteArray baTmpBA = QString("AT+DEL \"").append(gstrDownloadFilename).append("\" +").toUtf8();
                gspSerialPort.write(baTmpBA);
                gintQueuedTXBytes += baTmpBA.size();
                MainWindow::DoLineEnd();
                gpMainLog->WriteLogData(QString(baTmpBA).append("\n"));

                //Start the timeout timer
                gtmrDownloadTimeoutTimer.start();
            }
        }
        else
        {
            //Unknown response
            QString strMessage = tr("Unknown response from server.");
            gpmErrorForm->show();
            gpmErrorForm->SetMessage(&strMessage);
        }
    }
}
#endif

/******************************************************************************/
// END OF FILE
/******************************************************************************/
