/******************************************************************************
** Copyright (C) 2023 Laird Connectivity
**
** Project: UwTerminalX
**
** Module: InputDialog.cpp
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
#include "InputDialog.h"
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
InputDialog::InputDialog(QWidget *parent) : QDialog(parent)
{
    QFormLayout *dialogLayout = new QFormLayout(this);

    // Add Command
    QLabel *commandLabel = new QLabel("Command:", this);
    QLineEdit *commandLine = new QLineEdit(this);
    dialogLayout->addRow(commandLabel, commandLine);

    fields << commandLine;

    // Add description
    QLabel *descriptionLabel = new QLabel("Description:", this);
    QLineEdit *descriptionLine = new QLineEdit(this);
    dialogLayout->addRow(descriptionLabel, descriptionLine);

    fields << descriptionLine;

    // Add buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal,
        this);
    dialogLayout->addWidget(buttonBox);
    bool conn = connect(buttonBox, &QDialogButtonBox::accepted, this, &InputDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected, this, &InputDialog::reject);
    Q_ASSERT(conn);

    // Set layout
    setLayout(dialogLayout);

    // Set title
    this->setWindowTitle("Add New Command");
}

//=============================================================================
//=============================================================================
QStringList InputDialog::getStrings(QWidget *parent, bool *ok)
{
    InputDialog *dialog = new InputDialog(parent);

    QStringList list;

    const int ret = dialog->exec();
    if (ok)
    {
        *ok = !!ret;
    }

    if (ret)
    {
        foreach (auto field, dialog->fields)
        {
            list << field->text();
        }
    }

    dialog->deleteLater();

    return list;
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
