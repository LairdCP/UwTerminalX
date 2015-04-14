/******************************************************************************
** Copyright (C) 2014-2015 Ezurio Ltd
**
** Project: UwTerminalX
**
** Module: LrdLogger.h
**
** Notes:
**
*******************************************************************************/
#ifndef LRDLOGGER_H
#define LRDLOGGER_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QWidget>
#include <QFile>
#include <QTextStream>

/******************************************************************************/
// Defines
/******************************************************************************/
#define LOG_OK 0                //Success
#define LOG_ERR_OPEN_ALREADY 1  //Log already open
#define LOG_ERR_ACCESS 2        //Access denied to log file

/******************************************************************************/
// Class definitions
/******************************************************************************/
class LrdLogger : public QWidget
{
    Q_OBJECT
public:
    explicit LrdLogger
    (
    QWidget *parent = 0
    );
    ~LrdLogger
    (
    );
    unsigned char OpenLogFile
    (
    QString Filename
    );
    void CloseLogFile
    (
    );
    unsigned char WriteLogData
    (
    QString Data
    );
    unsigned char WriteRawLogData
    (
    QByteArray Data
    );
    unsigned short GetLogSize
    (
    );
    void ClearLog
    (
    );

private:
    bool mbLogOpen; //True when log file is open
    QFile *mpLogFile; //Contains the handle of log file
    QTextStream *mpStreamOut; //Contains the handle to the text steam
};

#endif // LRDLOGGER_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
