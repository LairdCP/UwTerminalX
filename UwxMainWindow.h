/******************************************************************************
** Copyright (C) 2014-2015 Ezurio Ltd
**
** Project: UwTerminalX
**
** Module: UwxMainWindow.h
**
** Notes:
**
*******************************************************************************/
#ifndef UWXMAINWINDOW_H
#define UWXMAINWINDOW_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMenu>
#include <QClipboard>
#include <QMimeData>
#include <QFileDialog>
#include <QScrollBar>
#include <QProcess>
#include <QTimer>
#include <QRegularExpression>
#include <QStringList>
#include <QMessageBox>
#include <QFile>
#include <QKeyEvent>
#include <QFontDialog>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTextDocumentFragment>
#include <QTime>
#include <QDate>
#include <QElapsedTimer>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QFileInfo>
#include "LrdScrollEdit.h"
#include "UwxPopup.h"
#include "LrdLogger.h"

/******************************************************************************/
// Defines
/******************************************************************************/
#define ServerHost "uwterminalx.no-ip.org" //Hostname/IP of online xcompile server
//#define UseSSL //Define to use https/SSL (requires OpenSSL support in Qt and runtime libraries)
//Defines for various file download functions
#define MODE_COMPILE 1
#define MODE_COMPILE_LOAD 2
#define MODE_COMPILE_LOAD_RUN 3
#define MODE_LOAD 4
#define MODE_LOAD_RUN 5
#define MODE_SERVER_COMPILE 9
#define MODE_SERVER_COMPILE_LOAD 10
#define MODE_SERVER_COMPILE_LOAD_RUN 11
#define MODE_CHECK_ERROR_CODE_VERSIONS 14
#define MODE_CHECK_UWTERMINALX_VERSIONS 15
#define MODE_UPDATE_ERROR_CODE 16
#define MODE_CHECK_FIRMWARE_VERSIONS 17
//Defines for version and functions
#define UwVersion "0.94" //Version string
#define FileReadBlock 512 //Number of bytes to read per block when streaming files
#define StreamProgress 10000 //Number of bytes between streaming progress updates
#define BatchTimeout 4000 //Time (in mS) to wait for getting a response from a batch command for
#define PrePostXCompTimeout 15000 //Time (in mS) to allow a pre/post XCompilation process to execute for
#define ModuleTimeout 4000 //Time (in mS) that a download stage command/process times out (module)
//Defines for default config values
#define DefaultLogFile "UwTerminalX.log"
#define DefaultLogMode 0
#define DefaultLogEnable 0
#define DefaultCompilerDir "compilers/"
#define DefaultCompilerSubDirs 0
#define DefaultDelUWCAfterDownload 0
#define DefaultSysTrayIcon 1
#define DefaultSerialSignalCheckInterval 50
#define DefaultPrePostXCompRun 0
#define DefaultPrePostXCompFail 0
#define DefaultPrePostXCompMode 0
#define DefaultPrePostXCompPath ""
#define DefaultOnlineXComp 1
#define DefaultTextUpdateInterval 80
//Define the protocol
#ifdef UseSSL
    //HTTPS
    #define WebProtocol "https"
#else
    //HTTP
    #define WebProtocol "http"
#endif

/******************************************************************************/
// Forward declaration of Class, Struct & Unions
/******************************************************************************/
namespace Ui
{
    class MainWindow;
}

/******************************************************************************/
// Class definitions
/******************************************************************************/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void
    MessagePass
        (
        QString strDataString,
        bool bEscapeString
        );

public slots:
    void
    readData
        (
        );
    void
    triggered
        (
        QAction* qaAction
        );
    void
    balloontriggered
        (
        QAction* qaAction
        );
    void
    DevRespTimeout
        (
        );
    void
    process_finished
        (
        int intExitCode,
        QProcess::ExitStatus esExitStatus
        );
    void
    SerialStatusSlot
        (
        );
    void
    SerialError
        (
        QSerialPort::SerialPortError speErrorCode
        );
    void
    EnterPressed
        (
        );
    void
    KeyPressed
        (
        QChar chrKeyValue
        );
    void
    SerialBytesWritten
        (
        qint64 intByteCount
        );

    void
    SerialPortClosing
        (
        );

private slots:
    void
    on_btn_Accept_clicked
        (
        );
    void
    on_selector_Tab_currentChanged
        (
        int intIndex
        );
    void
    on_btn_Connect_clicked
        (
        );
    void
    on_btn_TermClose_clicked
        (
        );
    void
    on_btn_Refresh_clicked
        (
        );
    void
    on_btn_TermClear_clicked
        (
        );
    void
    on_btn_Duplicate_clicked
        (
        );
    void
    on_text_TermEditData_customContextMenuRequested
        (
        const QPoint &pos
        );
    void
    on_check_Break_stateChanged
        (
        );
    void
    on_check_RTS_stateChanged
        (
        );
    void
    on_check_DTR_stateChanged
        (
        );
    void
    on_check_Line_stateChanged
        (
        );
    void
    closeEvent
        (
        QCloseEvent *closeEvent
        );
    void
    on_btn_Cancel_clicked
        (
        );
    void
    UpdateReceiveText
        (
        );
    void
    BatchTimeoutSlot
        (
        );
    void
    on_combo_COM_currentIndexChanged
        (
        int intIndex
        );
    void
    replyFinished
        (
        QNetworkReply* nrReply
        );
#ifdef UseSSL
    void
    sslErrors
        (
        QNetworkReply*,
        QList<QSslError>
        );
#endif
    void
    on_check_PreXCompRun_stateChanged
        (
        int iChecked
        );
    bool
    RunPrePostExecutable
        (
        QString strFilename
        );
    void
    on_btn_PreXCompSelect_clicked
        (
        );
    void
    on_radio_XCompPre_toggled
        (
        bool bChecked
        );
    void
    on_radio_XCompPost_toggled
        (
        bool bChecked
        );
    void
    on_check_PreXCompFail_stateChanged
        (
        int bChecked
        );
    void
    on_edit_PreXCompFilename_editingFinished
        (
        );
    void
    on_btn_GitHub_clicked
        (
        );
    void
    on_check_OnlineXComp_stateChanged
        (
        int bChecked
        );
    QList<QString>
    SplitFilePath
        (
        QString strFilename
        );
    void
    on_btn_BL600Apps_clicked
        (
        );
    void
    on_btn_BL620Apps_clicked
        (
        );
    void
    on_btn_BT900Apps_clicked
        (
        );
    void
    on_btn_ErrorCodeUpdate_clicked
        (
        );
    void
    on_btn_UwTerminalXUpdate_clicked
        (
        );
    void
    on_check_Echo_stateChanged
        (
        int bChecked
        );
    void
    on_btn_ErrorCodeDownload_clicked
        (
        );
    void
    on_combo_PredefinedDevice_currentIndexChanged
        (
        int index
        );
    void
    on_btn_PredefinedAdd_clicked
        (
        );
    void
    on_btn_PredefinedDelete_clicked
        (
        );
    void
    DroppedFile
        (
        QString strFilename
        );
    void
    on_btn_SaveDevice_clicked
        (
        );
    void
    on_btn_OpenLog_clicked
        (
        );
    void
    on_btn_OpenConfig_clicked
        (
        );
    void
    on_btn_ModuleFirmware_clicked
        (
        );
    void
    on_btn_LairdModules_clicked
        (
        );
    void
    ContextMenuClosed
        (
        );
    bool
    event
        (
        QEvent *evtEvent
        );
    void
    on_btn_OpenDeviceFile_clicked
        (
        );
    void
    on_btn_LogFileSelect_clicked
        (
        );
    void
    on_edit_LogFile_editingFinished
        (
        );
    void
    on_check_LogEnable_stateChanged
        (
        int iChecked
        );
    void
    on_check_LogAppend_stateChanged
        (
        int iChecked
        );

private:
    Ui::MainWindow *ui;
    void
    RefreshSerialDevices
        (
        );
    unsigned char
    CompileApp
        (
        unsigned char chMode
        );
    void
    UpdateImages
        (
        );
    void
    DoLineEnd
        (
        );
    void
    SerialStatus
        (
        bool bType
        );
    void
    OpenSerial
        (
        );
    void
    LoadFile
        (
        bool bToUWC
        );
    void
    RunApplication
        (
        );
    void
    LookupErrorCode
        (
        unsigned int intErrorCode
        );
    void
    FinishStream
        (
        bool bType
        );
    void
    FinishBatch
        (
        bool bType
        );

    //Private variables
    bool gbTermBusy; //True when compiling or loading a program or streaming a file (busy)
    bool gbStreamingFile; //True when a file is being streamed
    QSerialPort gspSerialPort; //Contains the handle for the serial port
    quint32 gintRXBytes; //Number of RX bytes
    quint32 gintTXBytes; //Number of TX bytes
    quint32 gintQueuedTXBytes; //Number of TX bytes that have been queued in buffer (not necesserially sent)
    unsigned char gchTermBusyLines; //Number of commands recieved (for sending applications)
    unsigned char gchTermMode; //What function is being ran when compiling
    unsigned char gchTermMode2; //Current sub-mode of download
    QString gstrTermFilename; //Holds the filename of the file to load
    QString gstrTermBusyData; //Holds the recieved data for checking
    QProcess gprocCompileProcess; //Holds the data for the process to compile
    QImage gimEmptyCircleImage; //Holder for empty circle image
    QImage gimRedCircleImage; //Holder for red circle image
    QImage gimGreenCircleImage; //Holder for green circle image
    QImage gimUw16Image; //Holder for UwTerminal 16x16 icon
    QImage gimUw32Image; //Holder for UwTerminal 32x32 icon
    QPixmap *gpEmptyCirclePixmap; //Pixmap holder for empty circle image
    QPixmap *gpRedCirclePixmap; //Pixmap holder for red circle image
    QPixmap *gpGreenCirclePixmap; //Pixmap holder for green circle image
    QPixmap *gpUw32Pixmap; //Pixmap holder for UwTerminal 32x32 icon
    QPixmap *gpUw16Pixmap; //Pixmap holder for UwTerminal 16x16 icon
    QString gstrHexData; //Holds the hex data to be sent to the device
    QString gstrDownloadFilename; //Holds the inter-function download filename
    QTimer *gpSignalTimer; //Handle for a timer to update COM port signals
    QTimer gtmrDownloadTimeoutTimer; //Timer for module timeout indication
    LrdLogger *gpMainLog; //Handle to the main log file (if enabled/used)
    bool gbMainLogEnabled; //True if opened successfully (and enabled)
    QMenu *gpMenu; //Main menu
    QMenu *gpSMenu1; //Submenu 1
    QMenu *gpSMenu2; //Submenu 2
    QMenu *gpSMenu3; //Submenu 3
    QMenu *gpBalloonMenu; //Balloon menu
    bool gbLoopbackMode; //True if loopback mode is enabled
    bool gbSysTrayEnabled; //True if system tray is enabled
    QSystemTrayIcon *gpSysTray; //Handle for system tray object
    bool gbIsUWCDownload; //Set to true when a UWC is being downloaded
    bool gbCTSStatus; //True when CTS is asserted
    bool gbDCDStatus; //True when DCD is asserted
    bool gbDSRStatus; //True when DSR is asserted
    bool gbRIStatus; //True when RI is asserted
    QFile *gpStreamFileHandle; //Handle for the file to stream data from
    quint32 gintStreamBytesSize; //The size of the file to stream in bytes
    quint32 gintStreamBytesRead; //The number of bytes read from the stream
    quint32 gintStreamBytesProgress; //The number of bytes when the next progress output should be made
    QByteArray gbaDisplayBuffer; //Buffer of data to display
    QElapsedTimer gtmrStreamTimer; //Counts how long a stream takes to send
    QTimer gtmrTextUpdateTimer; //Timer for slower updating of display buffer (but less display freezing)
    bool gbStreamingBatch; //True if batch file is being streamed
    QTimer gtmrBatchTimeoutTimer; //Timer for a batch command timeout
    QByteArray gbaBatchReceive; //Storage for batch data coming in
    QSettings *gpTermSettings; //Handle to settings
    QSettings *gpErrorMessages; //Handle to error codes
    QSettings *gpPredefinedDevice; //Handle to predefined devices
    QNetworkAccessManager *gnmManager; //Network access manager
    QString gstrDeviceID; //What the server compiler ID is
    bool gbFileOpened; //True when a file on the module has been opened
    QString gstrLastFilename; //Holds the last filename of the last selected file
#ifdef UseSSL
    QSslCertificate *sslcLairdSSL = NULL; //Holds the Laird SSL certificate
#endif

protected:
    void dragEnterEvent
        (
        QDragEnterEvent *dragEvent
        );
    void dropEvent
        (
        QDropEvent *dropEvent
        );
};

#endif // UWXMAINWINDOW_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
