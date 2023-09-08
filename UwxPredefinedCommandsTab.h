/******************************************************************************
** Copyright (C) 2023 Laird Connectivity
**
** Project: UwTerminalX
**
** Module: UwxPredefinedCommandsTab.h
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
#ifndef UWXPREDEFINEDCOMMANDSTAB_H
#define UWXPREDEFINEDCOMMANDSTAB_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QWidget>
#include <QTableView>
#include <QPushButton>

/******************************************************************************/
// Defines
/******************************************************************************/

/******************************************************************************/
// Constants
/******************************************************************************/

/******************************************************************************/
// Forward declaration of Class, Struct & Unions
/******************************************************************************/
namespace Ui {
    class UwxPredefinedCommandsTab;
}

/******************************************************************************/
// Class definitions
/******************************************************************************/
class UwxPredefinedCommandsTab : public QWidget
{
    Q_OBJECT

public:
    explicit UwxPredefinedCommandsTab(QWidget *parent = nullptr);
    ~UwxPredefinedCommandsTab();

    QTableView *getTableView();
    QPushButton *getAddButton();
    QPushButton *getRemoveButton();
    QPushButton *getClearButton();

private:
    Ui::UwxPredefinedCommandsTab *ui;
};

#endif // UWXPREDEFINEDCOMMANDSTAB_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
