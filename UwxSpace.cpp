/******************************************************************************
** Copyright (C) 2015 Laird
**
** Project: UwTerminalX
**
** Module: UwxMainWindow.cpp
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

/******************************************************************************/
// Include Files
/******************************************************************************/
#include "UwxSpace.h"
#include "ui_UwxSpace.h"

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
UwxSpace::UwxSpace(QWidget *parent) : QDialog(parent), ui(new Ui::UwxSpace)
{
    ui->setupUi(this);
}

//=============================================================================
//=============================================================================
UwxSpace::~UwxSpace(
    )
{
    delete ui;
}

//=============================================================================
//=============================================================================
void
UwxSpace::on_btn_Close_clicked(
    )
{
    //Close button clicked, close popup.
    this->close();
}

//=============================================================================
//=============================================================================
void
UwxSpace::SetupBars(
    unsigned long ulTotal,
    unsigned long ulFree,
    unsigned long ulDeleted
    )
{
    //Calculate the used space
    unsigned long ulUsed = ulTotal - ulFree - ulDeleted;

    //Set the progress bars up
    ui->bar_Used->setValue(ulUsed*100/ulTotal);
    ui->bar_Deleted->setValue(ulDeleted*100/ulTotal);
    ui->bar_Free->setValue(ulFree*100/ulTotal);
}

//=============================================================================
//=============================================================================
void
UwxSpace::SetupMode(
    bool bMode
    )
{
    if (bMode == true)
    {
        //FAT
        ui->label_Type->setText("FAT segment");
    }
    else
    {
        //Data
        ui->label_Type->setText("Data segment");
    }
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
