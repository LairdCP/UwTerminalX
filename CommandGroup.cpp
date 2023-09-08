/******************************************************************************
** Copyright (C) 2023 Laird Connectivity
**
** Project: UwTerminalX
**
** Module: CommandGroup.cpp
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
#include "CommandGroup.h"

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
CommandGroup::CommandGroup(QString name)
{
    this->mstrName = name;
    this->mlPredefinedCommands = new QList<PredefinedCommand>();
    this->uuid = QUuid::createUuid();
}

//=============================================================================
//=============================================================================
qsizetype CommandGroup::count()
{
    return this->mlPredefinedCommands->count();
}

//=============================================================================
//=============================================================================
QString CommandGroup::getName()
{
    return this->mstrName;
}

//=============================================================================
//=============================================================================
void CommandGroup::addCommand(PredefinedCommand &command)
{
    this->mlPredefinedCommands->append(command);
}

//=============================================================================
//=============================================================================
void CommandGroup::removeCommand(int index)
{
    this->mlPredefinedCommands->removeAt(index);
}

//=============================================================================
//=============================================================================
PredefinedCommand *CommandGroup::getPredefinedCommandAt(int index)
{
    if (index < 0 || index > this->count())
    {
        return nullptr;
    }

    return &(*mlPredefinedCommands)[index];
}

//=============================================================================
//=============================================================================
void CommandGroup::clear()
{
    this->mlPredefinedCommands->clear();
}

//=============================================================================
//=============================================================================
const QUuid CommandGroup::getUuid() const
{
    return this->uuid;
}

//=============================================================================
//=============================================================================
CommandGroup::~CommandGroup()
{
    delete this->mlPredefinedCommands;
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
