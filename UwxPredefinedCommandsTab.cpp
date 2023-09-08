/******************************************************************************
** Copyright (C) 2023 Laird Connectivity
**
** Project: UwTerminalX
**
** Module: UwxPredefinedCommandsTab.cpp
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

/******************************************************************************/
// Include Files
/******************************************************************************/
#include "UwxPredefinedCommandsTab.h"
#include "ui_UwxPredefinedCommandsTab.h"

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
UwxPredefinedCommandsTab::UwxPredefinedCommandsTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UwxPredefinedCommandsTab)
{
    ui->setupUi(this);

    // Set icon style for the clear button (not a separate image)
    ui->btn_Clear->setIcon(this->style()->standardIcon(QStyle::SP_DialogDiscardButton));
}

//=============================================================================
//=============================================================================
UwxPredefinedCommandsTab::~UwxPredefinedCommandsTab()
{
    delete ui;
}

//=============================================================================
//=============================================================================
QTableView *UwxPredefinedCommandsTab::getTableView()
{
    return ui->tableView;
}

//=============================================================================
//=============================================================================
QPushButton *UwxPredefinedCommandsTab::getAddButton()
{
    return ui->btn_Add;
}

//=============================================================================
//=============================================================================
QPushButton *UwxPredefinedCommandsTab::getRemoveButton()
{
    return ui->btn_Remove;
}

//=============================================================================
//=============================================================================
QPushButton *UwxPredefinedCommandsTab::getClearButton()
{
    return ui->btn_Clear;
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
