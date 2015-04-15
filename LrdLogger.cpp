/******************************************************************************
** Copyright (C) 2014-2015 Ezurio Ltd
**
** Project: UwTerminalX
**
** Module: LrdLogger.cpp
**
** Notes:
**
*******************************************************************************/

/******************************************************************************/
// Include Files
/******************************************************************************/
#include "LrdLogger.h"

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
LrdLogger::LrdLogger(QWidget *parent) : QWidget(parent)
{
    //Initial values
    mbLogOpen = false;
}

//=============================================================================
//=============================================================================
LrdLogger::~LrdLogger
    (
    )
{
    if (mbLogOpen == true)
    {
        //Log is open, flush the buffer, close it and delete it's handle
        mpStreamOut->flush();
        mpLogFile->flush();
        mbLogOpen = false;
        delete mpStreamOut;
        delete mpLogFile;
    }
}

//=============================================================================
//=============================================================================
unsigned char
LrdLogger::OpenLogFile
    (
    QString strFilename
    )
{
    //Opens the log file specified
    bool bNewFile = QFile::exists(strFilename);
    if (mbLogOpen == false)
    {
        //Open log file
        mpLogFile = new QFile(strFilename);
        if (!mpLogFile->open(QIODevice::Append | QIODevice::Text))
        {
            //Unable to open file
            return LOG_ERR_ACCESS;
        }
        mpStreamOut = new QTextStream(mpLogFile);
        mpStreamOut->setCodec("UTF-8");
        if (bNewFile == false)
        {
            //Create UTF8 header
            mpStreamOut->setGenerateByteOrderMark(true);
        }
        else
        {
            //Add a newline
            *mpStreamOut << "\r\n";
        }
        mbLogOpen = true;
        return LOG_OK;
    }
    else
    {
        //Log already open
        return LOG_ERR_OPEN_ALREADY;
    }
}

//=============================================================================
//=============================================================================
void
LrdLogger::CloseLogFile
    (
    )
{
    //Closes the log file
    if (mbLogOpen == true)
    {
        mpStreamOut->flush();
        mpLogFile->flush();
        mbLogOpen = false;
        delete mpStreamOut;
        delete mpLogFile;
    }
}

//=============================================================================
//=============================================================================
unsigned char
LrdLogger::WriteLogData
    (
    QString strData
    )
{
    //Writes a line to the log file
    *mpStreamOut << strData.toUtf8();
#ifdef FLUSHDATAONWRITE
    mpStreamOut->flush();
#endif
    return LOG_OK;
}

//=============================================================================
//=============================================================================
unsigned char
LrdLogger::WriteRawLogData
    (
    QByteArray baData
    )
{
    //Writes raw data to the log file
    *mpStreamOut << baData;
#ifdef FLUSHDATAONWRITE
    mpStreamOut->flush();
#endif
    return LOG_OK;
}

//=============================================================================
//=============================================================================
unsigned short
LrdLogger::GetLogSize
    (
    )
{
    //Returns the size of the log
    return mpLogFile->size();
}

//=============================================================================
//=============================================================================
void
LrdLogger::ClearLog
    (
    )
{
    //Clears out the log
    if (mbLogOpen == true)
    {
        mpLogFile->resize(0);
        mpStreamOut->setGenerateByteOrderMark(true);
    }
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
