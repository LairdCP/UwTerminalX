/******************************************************************************
** Copyright (C) 2023 Laird Connectivity
**
** Project: UwTerminalX
**
** Module: InputDialog.h
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

#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QDialog>

/******************************************************************************/
// Forward declaration of Class, Struct & Unions
/******************************************************************************/
class QLineEdit;
class QLabel;

/******************************************************************************/
// Class definitions
/******************************************************************************/
class InputDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InputDialog(QWidget *parent = nullptr);

    static QStringList getStrings(QWidget *parent, bool *ok = nullptr);
private:
    QList<QLineEdit*> fields;
};

#endif // INPUTDIALOG_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
