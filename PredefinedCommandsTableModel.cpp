/******************************************************************************
** Copyright (C) 2023 Laird Connectivity
**
** Project: UwTerminalX
**
** Module: PredefinedCommandsTableModel.cpp
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
#include "PredefinedCommandsTableModel.h"
#include <QSize>

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
PredefinedCommandsTableModel::PredefinedCommandsTableModel(QObject *parent)
    : PredefinedCommandsTableModel(new CommandGroup(""), parent)
{

}

//=============================================================================
//=============================================================================
PredefinedCommandsTableModel::PredefinedCommandsTableModel(CommandGroup *commandGroup, QObject *parent)
    : QAbstractTableModel(parent)
{
    mcgCommandGroup = commandGroup;
}

//=============================================================================
//=============================================================================
PredefinedCommandsTableModel::~PredefinedCommandsTableModel()
{
    delete mcgCommandGroup;
}

//=============================================================================
//=============================================================================
int PredefinedCommandsTableModel::rowCount(const QModelIndex & /*parent = QModelIndex()*/) const
{
    return this->mcgCommandGroup->count();
}

//=============================================================================
//=============================================================================
int PredefinedCommandsTableModel::columnCount(const QModelIndex & /*parent = QModelIndex()*/) const
{
    return nColumns;
}

//=============================================================================
//=============================================================================
QVariant PredefinedCommandsTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    PredefinedCommand *predefinedCommand = this->mcgCommandGroup->getPredefinedCommandAt(index.row());
    if (predefinedCommand == nullptr)
        return QVariant();

    switch(index.column())
    {
    case 1:
        return QVariant(predefinedCommand->getCommand());
    case 2:
        return QVariant(predefinedCommand->getDescription());
    default:
        return QVariant();
    }
}

//=============================================================================
//=============================================================================
QVariant PredefinedCommandsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::SizeHintRole)
        return QSize(50, 50);
    if (orientation == Qt::Vertical)
        return QVariant();

    switch (section)
    {
    case 0:
        return QVariant("Send");
    case 1:
        return QVariant("Command");
    case 2:
        return QVariant("Description");
    default:
        return QVariant();
    }
}

//=============================================================================
//=============================================================================
Qt::ItemFlags PredefinedCommandsTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (index.column() == 1 || index.column() == 2)
        flags |= Qt::ItemIsEditable;
    return flags;
}

//=============================================================================
//=============================================================================
bool PredefinedCommandsTableModel::setData(const QModelIndex &index, const QVariant &value, int /* role */)
{
    if (index.column() < 1 || index.column() > 2)
        return false;

    PredefinedCommand *predefinedCommand = this->mcgCommandGroup->getPredefinedCommandAt(index.row());
    if (predefinedCommand == nullptr)
        return false;

    bool ok;
    if (index.column() == 1)
    {
        ok = predefinedCommand->setCommand(value.toString());
    } else {
        ok = predefinedCommand->setDescription(value.toString());
    }

    return ok;
}

//=============================================================================
//=============================================================================
CommandGroup *PredefinedCommandsTableModel::getCommandGroup()
{
    return this->mcgCommandGroup;
}

//=============================================================================
//=============================================================================
PredefinedCommand *PredefinedCommandsTableModel::getPredefinedCommandAt(int index)
{
    return this->mcgCommandGroup->getPredefinedCommandAt(index);
}

//=============================================================================
//=============================================================================
void PredefinedCommandsTableModel::addCommand(PredefinedCommand &command)
{
    beginInsertRows(QModelIndex(), this->mcgCommandGroup->count(), this->mcgCommandGroup->count());
    this->mcgCommandGroup->addCommand(command);
    endInsertRows();
    emit dataChanged(createIndex(0,0), createIndex(this->mcgCommandGroup->count(), nColumns), {Qt::DisplayRole});
}

//=============================================================================
//=============================================================================
void PredefinedCommandsTableModel::removeCommand(QUuid uuid)
{
    for (int i = 0; i < rowCount(QModelIndex()); i++)
    {
        if (this->mcgCommandGroup->getPredefinedCommandAt(i)->getUuid() == uuid)
        {
            beginRemoveRows(QModelIndex(), i, i);
            this->mcgCommandGroup->removeCommand(i);
            endRemoveRows();
            emit dataChanged(createIndex(0,0), createIndex(this->mcgCommandGroup->count(), nColumns), {Qt::DisplayRole});
            return;
        }
    }
}

//=============================================================================
//=============================================================================
void PredefinedCommandsTableModel::clear()
{
    this->beginResetModel();
    this->mcgCommandGroup->clear();
    this->endResetModel();
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
