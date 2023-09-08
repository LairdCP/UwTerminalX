/******************************************************************************
** Copyright (C) 2023 Laird Connectivity
**
** Project: UwTerminalX
**
** Module: PredefinedCommand.cpp
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
#include "PredefinedCommand.h"

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
PredefinedCommand::PredefinedCommand(QString command, QString description)
{
    this->mstrCommand = command;
    this->mstrDescription = description;
    this->uuid = QUuid::createUuid();
}

//=============================================================================
//=============================================================================
QString PredefinedCommand::getCommand()
{
    return this->mstrCommand;
}

//=============================================================================
//=============================================================================
bool PredefinedCommand::setCommand(QString command)
{
    this->mstrCommand = command;
    return true;
}

//=============================================================================
//=============================================================================
QString PredefinedCommand::getDescription()
{
    return this->mstrDescription;
}

//=============================================================================
//=============================================================================
bool PredefinedCommand::setDescription(QString description)
{
    this->mstrDescription = description;
    return true;
}

//=============================================================================
//=============================================================================
const QUuid PredefinedCommand::getUuid() const
{
    return this->uuid;
}

//=============================================================================
//=============================================================================
PredefinedCommand::~PredefinedCommand()
{

}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
