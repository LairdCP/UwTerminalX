/******************************************************************************
** Copyright (C) 2015 Laird
**
** Project: UwTerminalX
**
** Module: UwxPopup.h
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
#ifndef UWXSPACE_H
#define UWXSPACE_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QDialog>

/******************************************************************************/
// Forward declaration of Class, Struct & Unions
/******************************************************************************/
namespace Ui
{
    class UwxSpace;
}

/******************************************************************************/
// Class definitions
/******************************************************************************/
class UwxSpace : public QDialog
{
    Q_OBJECT

public:
    explicit UwxSpace(
        QWidget *parent = 0
        );

    ~UwxSpace(
        );

public slots:
    void
    SetupBars(
        unsigned long ulTotal,
        unsigned long ulFree,
        unsigned long ulDeleted
        );

    void
    SetupMode(
        bool bMode
        );

private slots:
    void
    on_btn_Close_clicked(
        );

private:
    Ui::UwxSpace *ui;
};

#endif // UWXSPACE_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/