/******************************************************************************
** Copyright (C) 2023 Laird Connectivity
**
** Project: UwTerminalX
**
** Module: PredefinedCommand.h
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

#ifndef PREDEFINEDCOMMAND_H
#define PREDEFINEDCOMMAND_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QString>
#include <QUuid>
#include <QJsonObject>
#include <QJsonValue>

/******************************************************************************/
// Forward declaration of Class, Struct & Unions
/******************************************************************************/

/******************************************************************************/
// Class definitions
/******************************************************************************/
class PredefinedCommand
{
public:
    explicit PredefinedCommand(QString command = "", QString description = "");
    explicit PredefinedCommand(QUuid uuid, QString command = "", QString description = "");

    QString getCommand();
    bool setCommand(QString command);
    QString getDescription();
    bool setDescription(QString description);
    const QUuid getUuid() const;
    static PredefinedCommand *fromJson(const QJsonObject &json);
    QJsonObject toJson() const;
    ~PredefinedCommand();

private:
    QString mstrCommand;
    QString mstrDescription;
    QUuid uuid;
};

#endif // PREDEFINEDCOMMAND_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
