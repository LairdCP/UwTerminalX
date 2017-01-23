/******************************************************************************
** Copyright (C) 2015-2017 Laird
**
** Project: UwTerminalX
**
** Module: LrdScrollEdit.h
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
#ifndef LRDSCROLLEDIT_H
#define LRDSCROLLEDIT_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QPlainTextEdit>
#include <QKeyEvent>
#include <QString>
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
    explicit
    LrdScrollEdit(
        QWidget *parent = 0
        );
    void
    SetLineMode(
        bool bNewLineMode
        );
    void
    insertFromMimeData(
        const QMimeData *mdSrc
        );
    void
    UpdateDisplay(
        );
    void
    AddDatInText(
        QByteArray *baDat
        );
    void
    AddDatOutText(
        QString strDat
        );
    void
    ClearDatIn(
        );
    void
    ClearDatOut(
        );
    QString
    *GetDatOut(
        );
    void
    UpdateCursor(
        );
    void
    SetSerialOpen(
        bool SerialOpen
        );

protected:
    bool
    eventFilter(
        QObject *target,
        QEvent *event
        );

signals:
    void
    EnterPressed(
        );
    void
    KeyPressed(
        QChar KeyValue
        );
    void
    FileDropped(
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
    int mintCurPos; //Current cursor position
    unsigned int mbPrevTextSize; //Holds a count of the previous text size

public:
    bool mbLocalEcho; //True if local echo is enabled
    bool mbContextMenuOpen; //True when context menu is open
    bool mbLineSeparator; //True if line separator is enabled
};

#endif // LRDSCROLLEDIT_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
