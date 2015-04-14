/******************************************************************************
** Copyright (C) 2014-2015 Ezurio Ltd
**
** Project: UwTerminalX
**
** Module: UwxPopup.h
**
** Notes:
**
*******************************************************************************/
#ifndef UWXPOPUPMESSAGE_H
#define UWXPOPUPMESSAGE_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QDialog>

/******************************************************************************/
// Forward declaration of Class, Struct & Unions
/******************************************************************************/
namespace Ui
{
    class PopupMessage;
}

/******************************************************************************/
// Class definitions
/******************************************************************************/
class PopupMessage : public QDialog
{
    Q_OBJECT

public:
    explicit PopupMessage
    (
    QWidget *parent = 0
    );
    ~PopupMessage
    (
    );
    void SetMessage(
    QString *strMsg
    );

private slots:
    void on_pushButton_clicked
    (
    );

private:
    Ui::PopupMessage *ui;
};

#endif // UWXPOPUPMESSAGE_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
