/******************************************************************************
** Copyright (C) 2023 Laird Connectivity
**
** Project: UwTerminalX
**
** Module: UwxPredefinedCommands.h
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
** SPDX-License-Identifier: GPL-3.0
**
*******************************************************************************/
#ifndef UWXPREDEFINEDCOMMANDS_H
#define UWXPREDEFINEDCOMMANDS_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QDialog>
#include "UwxPopup.h"
#include <QTableView>
#include <QFileDialog>
#include <QFile>
#include <QStatusBar>
#include <QPainter>
#include <QPushButton>
#include "UwxPredefinedCommandsTab.h"
#include "PredefinedCommandsTableModel.h"

/******************************************************************************/
// Defines
/******************************************************************************/
#define SaveFilesWithUTF8Header        //Define to save files with a UTF8 BOM header

/******************************************************************************/
// Constants
/******************************************************************************/
const quint16 nVerticalHeaderHeight = 30;  //Vertical header height

/******************************************************************************/
// Forward declaration of Class, Struct & Unions
/******************************************************************************/
namespace Ui
{
    class UwxPredefinedCommands;
}

/******************************************************************************/
// Class definitions
/******************************************************************************/
class UwxPredefinedCommands : public QDialog
{
    Q_OBJECT

public:
    explicit
    UwxPredefinedCommands(
        QWidget *parent = 0
        );
    ~UwxPredefinedCommands(
        );
    void addCommand(
        QString command, QString description
        );
    UwxPredefinedCommandsTab *addCommandGroup(
        QString name
        );
    UwxPredefinedCommandsTab *getCurrentPredefinedCommandsTab(
        );
    QTableView *getCurrentTableView(
        );
    PredefinedCommandsTableModel *getCurrentTableModel(
        );
    void
    SetPopupHandle(
        PopupMessage *pmNewHandle
        );
    void
    ConnectionChange(
        bool bEnabled
        );
    void
    TempAlwaysOnTop(
        bool bEnabled
        );
    void
    LoadFile(
        QString strLoadFile
        );

private:
    void
        on_btn_Add_clicked(
            );
    void
        on_btn_Remove_clicked(
            );
    void
        on_btn_Clear_clicked(
            );

private slots:
    void
    on_btn_Load_clicked(
        );
    void
    on_btn_Save_clicked(
        );
    void
        on_btn_Add_Tab_clicked(
            );
    void
        on_btn_Remove_Tab_clicked(
            );
    void
    on_btn_Close_clicked(
        );
    void
    on_check_OnTop_stateChanged(
        int
        );
    void
    sendButtonClicked(
        const QUuid
        );

signals:
    void SendData(
        QByteArray baDataString,
        bool bEscapeString,
        bool bFromScripting
        );

private:
    Ui::UwxPredefinedCommands *ui;
    PopupMessage *mFormAuto; //Holds handle of error message dialogue
    QStatusBar *msbStatusBar; //Pointer to automation status bar
    bool serialPortOpen = false;
};

#endif // UWXPREDEFINEDCOMMANDS_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
