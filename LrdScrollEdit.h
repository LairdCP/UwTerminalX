/******************************************************************************
** Copyright (C) 2014-2015 Ezurio Ltd
**
** Project: UwTerminalX
**
** Module: LrdScrollEdit.h
**
** Notes:
**
*******************************************************************************/
#ifndef LRDSCROLLEDIT_H
#define LRDSCROLLEDIT_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QPlainTextEdit>
#include <QKeyEvent>
#include <QString>
#include <QTimer>
#include <QScrollBar>
#include <QMimeData>
#include <QTextCursor>

/******************************************************************************/
// Defines
/******************************************************************************/
#define ItemAllow 20 //Number of scrollback items to allow

/******************************************************************************/
// Class definitions
/******************************************************************************/
class LrdScrollEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit LrdScrollEdit
        (
        QWidget *parent = 0
        );
    void SetLineMode
        (
        bool bNewLineMode
        );
    void insertFromMimeData
        (
        const QMimeData *mdSrc
        );
    void UpdateDisplay
        (
        );
    void AddDatInText
        (
        QByteArray *baDat
        );
    void AddDatOutText
        (
        QString strDat
        );
    void ClearDatIn
        (
        );
    void ClearDatOut
        (
        );
    QString *GetDatOut
        (
        );
    void UpdateCursor
        (
        );
    void SetSerialOpen
        (
        bool SerialOpen
        );

protected:
    bool eventFilter
        (
        QObject *target,
        QEvent *event
        );

signals:
    void EnterPressed
        (
        );
    void KeyPressed
        (
        QChar KeyValue
        );
    void
    FileDropped
        (
        QString strFilename
        );

private slots:

private:
    QString mstrItemArray[ItemAllow+1]; //Item text
    unsigned char mchItems; //Number of items
    unsigned char mchPosition; //Current position
    bool mbLineMode; //True enables line mode
    bool mbSerialOpen; //True if serial port is open
    QString mstrDatIn; //Incoming data (previous commands/received data)
    QString mstrDatOut; //Outgoing data (user typed keyboard data)
    unsigned int muintCurPos; //Current cursor position

public:
    bool mbLocalEcho; //True if local echo is enabled
};

#endif // LRDSCROLLEDIT_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
