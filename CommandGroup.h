/******************************************************************************
** Copyright (C) 2023 Laird Connectivity
**
** Project: UwTerminalX
**
** Module: CommandGroup.h
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

#ifndef COMMANDGROUP_H
#define COMMANDGROUP_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QString>
#include <QList>
#include "PredefinedCommand.h"

/******************************************************************************/
// Forward declaration of Class, Struct & Unions
/******************************************************************************/

/******************************************************************************/
// Class definitions
/******************************************************************************/
class CommandGroup
{
public:
    explicit CommandGroup(QString name = "");
    ~CommandGroup();

    qsizetype count();
    QString getName();
    void addCommand(PredefinedCommand &command);
    void removeCommand(int index);
    PredefinedCommand *getPredefinedCommandAt(int index);
    void clear();
    const QUuid getUuid() const;

private:
    QString mstrName;
    QList<PredefinedCommand> *mlPredefinedCommands;
    QUuid uuid;
};

#endif // COMMANDGROUP_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
