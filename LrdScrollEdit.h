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
    int KeyValue
    );

private slots:
    void SendPasteData();

private:
    QString mstrItemArray[ItemAllow+1]; //Item text
    unsigned char mchItems; //Number of items
    unsigned char mchPosition; //Current position
    bool mbLineMode; //True enables line mode
    QTimer PasteTimer; //Timer for when something is pasted in character mode

};

#endif // LRDSCROLLEDIT_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
