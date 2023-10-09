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
PredefinedCommand::PredefinedCommand(QUuid uuid, QString command, QString description)
{
    this->mstrCommand = command;
    this->mstrDescription = description;
    this->uuid = uuid;
}

//=============================================================================
//=============================================================================
PredefinedCommand::PredefinedCommand(QString command, QString description)
    : PredefinedCommand(QUuid::createUuid(), command, description)
{

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
PredefinedCommand *PredefinedCommand::fromJson(const QJsonObject &json)
{
    PredefinedCommand *result = new PredefinedCommand();

    if (const QJsonValue v = json["uuid"]; v.isString())
        result->uuid = QUuid(v.toString());

    if (const QJsonValue v = json["command"]; v.isString())
        result->mstrCommand = v.toString();

    if (const QJsonValue v = json["description"]; v.isString())
        result->mstrDescription = v.toString();

    return result;
}

//=============================================================================
//=============================================================================
QJsonObject PredefinedCommand::toJson() const
{
    QJsonObject json;

    json["uuid"] = uuid.toString();
    json["command"] = mstrCommand;
    json["description"] = mstrDescription;

    return json;
}

//=============================================================================
//=============================================================================
PredefinedCommand::~PredefinedCommand()
{

}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
