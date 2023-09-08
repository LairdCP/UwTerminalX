/******************************************************************************
** Copyright (C) 2023 Laird Connectivity
**
** Project: UwTerminalX
**
** Module: PredefinedCommandsTableModel.h
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

#ifndef PREDEFINEDCOMMANDSTABLEMODEL_H
#define PREDEFINEDCOMMANDSTABLEMODEL_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QAbstractTableModel>
#include <QObject>
#include "CommandGroup.h"

/******************************************************************************/
// Constants
/******************************************************************************/
const quint16 nColumns = 3;            //Number of table colunns

/******************************************************************************/
// Forward declaration of Class, Struct & Unions
/******************************************************************************/

/******************************************************************************/
// Class definitions
/******************************************************************************/
class PredefinedCommandsTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit PredefinedCommandsTableModel(QObject *parent = nullptr);
    explicit PredefinedCommandsTableModel(CommandGroup *commandGroup, QObject *parent = nullptr);
    ~PredefinedCommandsTableModel();

    int rowCount(const QModelIndex & /*parent = QModelIndex()*/) const;
    int columnCount(const QModelIndex & /*parent = QModelIndex()*/) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    CommandGroup *getCommandGroup();
    PredefinedCommand *getPredefinedCommandAt(int index);
    void addCommand(PredefinedCommand &command);
    void removeCommand(QUuid uuid);
    void clear();

private:
    CommandGroup *mcgCommandGroup;
};

#endif // PREDEFINEDCOMMANDSTABLEMODEL_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
