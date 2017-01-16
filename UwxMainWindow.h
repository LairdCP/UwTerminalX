/******************************************************************************
** Copyright (C) 2015-2016 Laird
**
** Project: UwTerminalX
**
** Module: UwxMainWindow.h
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
#ifndef UWXMAINWINDOW_H
#define UWXMAINWINDOW_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
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
#include <QHostInfo>
//Need cmath for std::ceil function
#include <cmath>
#if TARGET_OS_MAC
#include "QStandardPaths"
#endif
#include "LrdScrollEdit.h"
#include "UwxPopup.h"
#include "LrdLogger.h"
#if SKIPAUTOMATIONFORM != 1
#include "UwxAutomation.h"
#endif
#if SKIPERRORCODEFORM != 1
#include "UwxErrorCode.h"
#endif
#if SKIPSCRIPTINGFORM != 1
#include "UwxScripting.h"
#endif
#include "UwxEscape.h"

/******************************************************************************/
// Defines
/******************************************************************************/
#define ServerHost                        "uwterminalx.no-ip.org" //Hostname/IP of online xcompile server
#ifndef QT_NO_SSL
    #define UseSSL //By default enable SSL if Qt supports it (requires OpenSSL runtime libraries). Comment this line out to build without SSL support or if you get errors when communicating with the server
#endif
//Defines for various file download functions
#define MODE_COMPILE                      1
#define MODE_COMPILE_LOAD                 2
#define MODE_COMPILE_LOAD_RUN             3
#define MODE_LOAD                         4
#define MODE_LOAD_RUN                     5
#define MODE_SERVER_COMPILE               9
#define MODE_SERVER_COMPILE_LOAD          10
#define MODE_SERVER_COMPILE_LOAD_RUN      11
#define MODE_CHECK_ERROR_CODE_VERSIONS    14
#define MODE_CHECK_UWTERMINALX_VERSIONS   15
#define MODE_UPDATE_ERROR_CODE            16
#define MODE_CHECK_FIRMWARE_VERSIONS      17
#define MODE_CHECK_FIRMWARE_SUPPORT       18
//Defines for version and functions
#define UwVersion                         "1.08f" //Version string
//
#define FileReadBlock                     512     //Number of bytes to read per block when streaming files
#define StreamProgress                    10000   //Number of bytes between streaming progress updates
#define BatchTimeout                      4000    //Time (in ms) to wait for getting a response from a batch command for
#define PrePostXCompTimeout               15000   //Time (in ms) to allow a pre/post XCompilation process to execute for
#define ModuleTimeout                     4000    //Time (in ms) that a download stage command/process times out (module)
#define MaxDevNameSize                    8       //Size (in characters) to allow for a module device name (characters past this point will be chopped off)
#define AutoBaudTimeout                   1200    //Time (in ms) to wait before checking the next baud rate when automatically detecting the module's baud rate
//Defines for default config values
#define DefaultLogFile                    "UwTerminalX.log"
#define DefaultLogMode                    0
#define DefaultLogEnable                  0
#define DefaultCompilerDir                "compilers/"
#define DefaultCompilerSubDirs            0
#define DefaultDelUWCAfterDownload        0
#define DefaultSysTrayIcon                1
#define DefaultSerialSignalCheckInterval  50
#define DefaultPrePostXCompRun            0
#define DefaultPrePostXCompFail           0
#define DefaultPrePostXCompMode           0
#define DefaultPrePostXCompPath           ""
#define DefaultOnlineXComp                1
#define DefaultTextUpdateInterval         80
#define DefaultSkipDownloadDisplay        1
#define DefaultSSLEnable                  1
#define DefaultShowFileSize               1
#define DefaultConfirmClear               1
#define DefaultShiftEnterLineSeparator    1
//Define the protocol
#ifndef UseSSL
    //HTTP
    #define WebProtocol "http"
#endif
//Defines for array indexes
#define FilenameIndexApplication          0
#define FilenameIndexOthers               1
//Defines for right click menu options
#define MenuActionXCompile                1
#define MenuActionXCompileLoad            2
#define MenuActionXCompileLoadRun         3
#define MenuActionLoad                    4
#define MenuActionLoadRun                 5
#define MenuActionErrorHex                6
#define MenuActionErrorInt                7
#define MenuActionLoopback                8
#define MenuActionLoad2                   9
#define MenuActionEraseFile               10
#define MenuActionDir                     11
#define MenuActionRun                     12
#define MenuActionDebug                   13
#define MenuActionDataFile                14
#define MenuActionEraseFile2              15
#define MenuActionClearFilesystem         16
#define MenuActionMultiDataFile           17
#define MenuActionStreamFile              18
#define MenuActionFont                    19
#define MenuActionRun2                    20
#define MenuActionAutomation              21
#define MenuActionScripting               22
#define MenuActionBatch                   23
#define MenuActionClearModule             24
#define MenuActionClearDisplay            25
#define MenuActionClearRxTx               26
#define MenuActionCopy                    27
#define MenuActionCopyAll                 28
#define MenuActionPaste                   29
#define MenuActionSelectAll               30
//Defines for balloon (notification area) icon options
#define BalloonActionShow                 1
#define BalloonActionExit                 2
//Defines for speed test menu
#define SpeedMenuActionRecv               1
#define SpeedMenuActionSend               2
#define SpeedMenuActionSendRecv           3
#define SpeedMenuActionSendRecv5Delay     4
#define SpeedMenuActionSendRecv10Delay    5
#define SpeedMenuActionSendRecv15Delay    6
#define SpeedModeInactive                 0b00
#define SpeedModeRecv                     0b01
#define SpeedModeSend                     0b10
#define SpeedModeSendRecv                 0b11
//Defines for the selector tab
#define TabTerminal                       0
#define TabConfig                         1
#define TabSpeedTest                      2
#define TabUpdate                         3
#define TabAbout                          4
#define TabLogs                           5
#define TabEditor                         6
//Defines for speed testing
#define SpeedTestChunkSize                512  //Maximum number of bytes to send per chunk when speed testing
#define SpeedTestMinBufSize               128  //Minimum buffer size when speed testing, when there are less than this number of bytes in the output buffer it will be topped up
#define SpeedTestStatUpdateTime           500  //Time (in ms) between status updates for speed test mode

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
    explicit MainWindow(
        QWidget *parent = 0
        );
    ~MainWindow(
        );

public slots:
    void
    SerialRead(
        );
    void
    MenuSelected(
        QAction* qaAction
        );
    void
    balloontriggered(
        QAction* qaAction
        );
    void
    DevRespTimeout(
        );
#if defined(_WIN32) || defined(WIN32)
    //_WIN32 isn't defined when MOC is ran but WIN32 is.
    void
    process_finished(
        int intExitCode,
        QProcess::ExitStatus esExitStatus
        );
#endif
    void
    SerialStatusSlot(
        );
    void
    SerialError(
        QSerialPort::SerialPortError speErrorCode
        );
    void
    EnterPressed(
        );
    void
    KeyPressed(
        QChar chrKeyValue
        );
    void
    SerialBytesWritten(
        qint64 intByteCount
        );
    void
    SerialPortClosing(
        );
    void
    MessagePass(
        QString strDataString,
        bool bEscapeString,
        bool bFromScripting
        );
    void
    SpeedMenuSelected(
        QAction* qaAction
        );

private slots:
    void
    on_btn_Accept_clicked(
        );
    void
    on_selector_Tab_currentChanged(
        int intIndex
        );
    void
    on_btn_Connect_clicked(
        );
    void
    on_btn_TermClose_clicked(
        );
    void
    on_btn_Refresh_clicked(
        );
    void
    on_btn_TermClear_clicked(
        );
    void
    on_btn_Duplicate_clicked(
        );
    void
    on_text_TermEditData_customContextMenuRequested(
        const QPoint &pos
        );
    void
    on_check_Break_stateChanged(
        );
    void
    on_check_RTS_stateChanged(
        );
    void
    on_check_DTR_stateChanged(
        );
    void
    on_check_Line_stateChanged(
        );
    void
    closeEvent(
        QCloseEvent *closeEvent
        );
    void
    on_btn_Cancel_clicked(
        );
    void
    UpdateReceiveText(
        );
    void
    BatchTimeoutSlot(
        );
    void
    on_combo_COM_currentIndexChanged(
        int intIndex
        );
    void
    replyFinished(
        QNetworkReply* nrReply
        );
#ifdef UseSSL
    void
    sslErrors(
        QNetworkReply*,
        QList<QSslError>
        );
#endif
    void
    on_check_PreXCompRun_stateChanged(
        int intChecked
        );
    bool
    RunPrePostExecutable(
        QString strFilename
        );
    void
    on_btn_PreXCompSelect_clicked(
        );
    void
    on_radio_XCompPre_toggled(
        bool bChecked
        );
    void
    on_radio_XCompPost_toggled(
        bool bChecked
        );
    void
    on_check_PreXCompFail_stateChanged(
        int bChecked
        );
    void
    on_edit_PreXCompFilename_editingFinished(
        );
    void
    on_btn_GitHub_clicked(
        );
    void
    on_check_OnlineXComp_stateChanged(
        int bChecked
        );
    QList<QString>
    SplitFilePath(
        QString strFilename
        );
    void
    on_btn_ErrorCodeUpdate_clicked(
        );
    void
    on_btn_UwTerminalXUpdate_clicked(
        );
    void
    on_check_Echo_stateChanged(
        int bChecked
        );
    void
    on_btn_ErrorCodeDownload_clicked(
        );
    void
    on_combo_PredefinedDevice_currentIndexChanged(
        int intIndex
        );
    void
    on_btn_PredefinedAdd_clicked(
        );
    void
    on_btn_PredefinedDelete_clicked(
        );
    void
    DroppedFile(
        QString strFilename
        );
    void
    on_btn_SaveDevice_clicked(
        );
    void
    on_btn_ModuleFirmware_clicked(
        );
    void
    ContextMenuClosed(
        );
    bool
    event(
        QEvent *evtEvent
        );
    void
    on_btn_LogFileSelect_clicked(
        );
    void
    on_edit_LogFile_editingFinished(
        );
    void
    on_check_LogEnable_stateChanged(
        int intChecked
        );
    void
    on_check_LogAppend_stateChanged(
        int intChecked
        );
    void
    on_btn_Help_clicked(
        );
    void
    on_combo_LogDirectory_currentIndexChanged(
        int
        );
    void
    on_btn_LogRefresh_clicked(
        );
    void
    on_btn_Licenses_clicked(
        );
    void
    on_btn_OnlineXComp_Supported_clicked(
        );
    void
    on_check_SkipDL_stateChanged(
        int
        );
    bool
    LookupDNSName(
        );
    void
    on_btn_WebBrowse_clicked(
        );
    void
    on_btn_EditViewFolder_clicked(
        );
    void
    on_combo_EditFile_currentIndexChanged(
        int
        );
    void
    on_btn_EditSave_clicked(
        );
    void
    on_btn_EditLoad_clicked(
        );
#ifndef __APPLE__
    void
    on_btn_EditExternal_clicked(
        );
#endif
    void
    on_btn_LogViewExternal_clicked(
        );
    void
    on_btn_LogViewFolder_clicked(
        );
    void
    on_text_EditData_textChanged(
        );
    void
    on_combo_LogFile_currentIndexChanged(
        int
        );
    void
    on_btn_ReloadLog_clicked(
        );
#ifdef UseSSL
    void
    on_check_EnableSSL_stateChanged(
        int
        );
#endif
    void
    on_check_ShowFileSize_stateChanged(
        int
        );
    void
    on_btn_DetectBaud_clicked(
        );
    void
    DetectBaudTimeout(
        );
    void
    on_check_ConfirmClear_stateChanged(
        int
        );
    void
    on_check_LineSeparator_stateChanged(
        int
        );
#if SKIPERRORCODEFORM != 1
    void
    on_btn_Error_clicked(
        );
#endif
#if SKIPSCRIPTINGFORM != 1
    void
    ScriptStartRequest(
        );
    void
    ScriptFinished(
        );
#endif
    void
    on_check_SpeedRTS_stateChanged(
        int
        );
    void
    on_check_SpeedDTR_stateChanged(
        int
        );
    void
    on_btn_SpeedClear_clicked(
        );
    void
    on_btn_SpeedClose_clicked(
        );
    void
    on_btn_SpeedStart_clicked(
        );
    void
    on_btn_SpeedStop_clicked(
        );
    void
    OutputSpeedTestStats(
        );
    void
    on_combo_SpeedDataType_currentIndexChanged(
        int
        );
    void
    on_btn_SpeedCopy_clicked(
        );
    void
    UpdateSpeedTestValues(
        );
    void
    SpeedTestStartTimer(
        );
    void
    SpeedTestStopTimer(
        );
    void
    UpdateDisplayText(
        );
    void
    on_combo_SpeedDataDisplay_currentIndexChanged(
        int
        );

private:
    Ui::MainWindow *ui;
    void
    RefreshSerialDevices(
        );
    unsigned char
    CompileApp(
        unsigned char chMode
        );
    void
    UpdateImages(
        );
    void
    DoLineEnd(
        );
    void
    SerialStatus(
        bool bType
        );
    void
    OpenDevice(
        );
    void
    LoadFile(
        bool bToUWC
        );
    void
    RunApplication(
        );
    void
    LookupErrorCode(
        unsigned int intErrorCode
        );
    void
    FinishStream(
        bool bType
        );
    void
    FinishBatch(
        bool bType
        );
    QString
    AtiToXCompName(
        QString strAtiResp
        );
    void
    LoadSettings(
        );
    void
    UpdateSettings(
        int intMajor,
        int intMinor,
        QChar qcDelta
        );
    QString
    CleanFilesize(
        QString strFilename
        );
    QString
    RemoveZeros(
        QString strData
        );
    void
    SendSpeedTestData(
        int intMaxLength
        );
    void
    SpeedTestBytesWritten(
        qint64 intByteCount
        );
    void
    SpeedTestReceive(
        );
    void
    OutputSpeedTestAvgStats(
        unsigned long nsec
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
#ifdef _WIN32
    QProcess gprocCompileProcess; //Holds the data for the process to compile
#endif
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
    QMenu *gpSpeedMenu; //Speed testing menu
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
    QNetworkReply *gnmrReply; //Network reply
    QString gstrDeviceID; //What the server compiler ID is
    bool gbFileOpened; //True when a file on the module has been opened
    QString gstrLastFilename[2]; //Holds the filenames of the last selected files
    QString gstrResolvedServer; //Holds the resolved hostname of the XCompile server
    bool gbEditFileModified; //True if the file in the editor pane has been modified, otherwise false
    int giEditFileType; //Type of file currently open in the editor
    bool gbErrorsLoaded; //True if error csv file has been loaded
    QTimer gtmrBaudTimer; //Timer for automatic baud rate detection timeout
    bool gbAutoBaud; //True if automatic baud rate detection is in progress
#ifdef UseSSL
    QString WebProtocol; //Holds HTTP or HTTPS depending on options selected
    QSslCertificate *sslcLairdSSL = NULL; //Holds the Laird SSL certificate
    QSslCertificate *sslcLairdSSLNew = NULL; //Holds the (newer) Laird SSL certificate
#endif
    PopupMessage *gpmErrorForm; //Error message form
#if SKIPAUTOMATIONFORM != 1
    UwxAutomation *guaAutomationForm; //Automation form
#endif
#if SKIPERRORCODEFORM != 1
    UwxErrorCode *gecErrorCodeForm; //Error code lookup form
#endif
#if SKIPSCRIPTINGFORM != 1
    UwxScripting *gusScriptingForm; //Scripting form
    bool gbScriptingRunning; //True if a script is running
#endif
    bool gbSpeedTestRunning; //True if speed test is running
    unsigned char gchSpeedTestMode; //What mode the speed test is (inactive, receive, send or send & receive)
    QElapsedTimer gtmrSpeedTimer; //Used for timing how long a speed test has been running
    QByteArray gbaSpeedDisplayBuffer; //Buffer of data to display for speed test mode
    QByteArray gbaSpeedMatchData; //Expected data to match in speed test mode
    QByteArray gbaSpeedReceivedData; //Received data from device in speed test mode
    QTimer gtmrSpeedTestStats; //Timer that runs every 250ms to update stats for speed test
    QTimer gtmrSpeedTestStats10s; //Timer that runs every 10 seconds to output 10s stats for speed test
    QTimer gtmrSpeedUpdateTimer; //Timer for slower updating of speed test buffer (but less display freezing)
    QTimer *gtmrSpeedTestDelayTimer; //Timer used for delay before sending data in speed test mode
    quint32 gintSpeedBytesReceived; //Number of bytes received from device in speed test mode
    quint32 gintSpeedBytesReceived10s; //Number of bytes received from device in the past 10 seconds in speed test mode
    quint32 gintSpeedBytesSent; //Number of bytes sent to the device in speed test mode
    quint32 gintSpeedBytesSent10s; //Number of bytes sent to the device in the past 10 seconds in speed test mode
    quint32 gintSpeedBufferCount; //Number of bytes waiting to be sent to the device (waiting in the buffer) in speed test mode
    quint32 gintSpeedTestMatchDataLength; //Length of MatchData
    quint32 gintSpeedTestReceiveIndex; //Current index for RecData
    quint32 gintSpeedTestStatErrors; //Number of errors in packets recieved in speed test mode
    quint32 gintSpeedTestStatSuccess; //Number of successful packets received in speed test mode
    quint32 gintSpeedTestStatPacketsSent; //Numbers of packets sent in speed test mode
    quint32 gintSpeedTestStatPacketsReceived; //Number of packets received in speed test mode
    quint8 gintSpeedTestDataBits; //Number of data bits (per byte) for speed testing
    quint8 gintSpeedTestStartStopParityBits; //Number of bits for start/stop/parity (per byte) for speed testing
    quint8 gintSpeedTestBytesBits; //Holds the current speed test combo selection option

protected:
    void dragEnterEvent(
        QDragEnterEvent *dragEvent
        );
    void dropEvent(
        QDropEvent *dropEvent
        );
};

#endif // UWXMAINWINDOW_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
