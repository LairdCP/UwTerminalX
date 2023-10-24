/******************************************************************************
** Copyright (C) 2023 Laird Connectivity
**
** Project: UwTerminalX
**
** Module: UwxPredefinedCommands.cpp
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
#include <QDebug>
#include <QString>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include "UwxPredefinedCommands.h"
#include "ui_UwxPredefinedCommands.h"
#include "UwxPredefinedCommandsTab.h"
#include "InputDialog.h"

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
UwxPredefinedCommands::UwxPredefinedCommands(QWidget *parent) : QDialog(parent), ui(new Ui::UwxPredefinedCommands)
{
    //On dialogue creation
    ui->setupUi(this);

    //Create status bar
    msbStatusBar = new QStatusBar;
    ui->StatusBarLayout->addWidget(msbStatusBar);
    msbStatusBar->showMessage("[Status]");

    addCommandGroup("New Group");

    // Connect to double-click event on group tab name to allow renaming groups
    connect(ui->tabWidget->tabBar(),
            &QTabBar::tabBarDoubleClicked,
            this,
            [=](int index){
                bool ok = true;
                QString newName = QInputDialog::getText(
                    this,
                    tr("Change Group Name"),
                    tr("New Group Name:"),
                    QLineEdit::Normal,
                    "New Group", &ok);

                if (ok)
                {
                    ui->tabWidget->setTabText(index, newName);
                }
            });

    //Set always on top state
    on_check_OnTop_stateChanged(0);
}

//=============================================================================
//=============================================================================
UwxPredefinedCommandsTab *UwxPredefinedCommands::addCommandGroup(QString name)
{
    return addCommandGroup(new CommandGroup(name));
}

//=============================================================================
//=============================================================================
UwxPredefinedCommandsTab *UwxPredefinedCommands::addCommandGroup(CommandGroup *commandGroup)
{
    // Create tab
    UwxPredefinedCommandsTab *tab = new UwxPredefinedCommandsTab();
    PredefinedCommandsTableModel *mpctmTableModel = new PredefinedCommandsTableModel(commandGroup, tab);
    tab->getTableView()->setModel(mpctmTableModel);
    tab->getTableView()->verticalHeader()->setVisible(false);
    tab->getTableView()->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);

    tab->getTableView()->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    tab->getTableView()->horizontalHeader()->resizeSection(1, 300);
    tab->getTableView()->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    tab->getTableView()->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    tab->getTableView()->horizontalHeader()->setStretchLastSection(true);
    tab->getTableView()->horizontalHeader()->setMaximumHeight(nVerticalHeaderHeight);
    tab->getTableView()->horizontalHeader()->setMinimumHeight(nVerticalHeaderHeight);

    // Connect Add button
    connect(tab->getAddButton(),
            &QPushButton::clicked,
            this,
            [=]{
                on_btn_Add_clicked();
            });

    // Connect Remove button
    connect(tab->getRemoveButton(),
            &QPushButton::clicked,
            this,
            [=]{
                on_btn_Remove_clicked();
            });

    // Connect Clear button
    connect(tab->getClearButton(),
            &QPushButton::clicked,
            this,
            [=]{
                on_btn_Clear_clicked();
            });

    // Add tab to the tab widget
    ui->tabWidget->addTab(tab, commandGroup->getName());

    return tab;
}

//=============================================================================
//=============================================================================
void UwxPredefinedCommands::addCommand(QString command, QString description)
{
    // Create the new command and add it to the current table's model
    PredefinedCommandsTableModel *model = getCurrentTableModel();
    PredefinedCommand *newCommand = new PredefinedCommand(command, description);
    model->addCommand(*newCommand);
    model->insertRow(model->rowCount(QModelIndex()));
    addSendButton(newCommand, getCurrentTableView(), model->getCommandGroup()->count() - 1);
}

void UwxPredefinedCommands::addSendButton(PredefinedCommand *newCommand, QTableView *tableView, int index)
{
    // Create a new 'Send' push button, add it to the table, and connect a slot to its 'clicked' signal
    QPushButton *sendButton = new QPushButton("&Send");
    sendButton->setEnabled(this->serialPortOpen);
    tableView->setIndexWidget(tableView->model()->index(index, 0), sendButton);
    connect(sendButton,
            &QPushButton::clicked,
            this,
            [=]{
                sendButtonClicked(newCommand->getUuid());
            });
}

//=============================================================================
//=============================================================================
UwxPredefinedCommands::~UwxPredefinedCommands(
    )
{
    delete msbStatusBar;
    delete ui;
}

//=============================================================================
//=============================================================================
void
UwxPredefinedCommands::SetPopupHandle(
    PopupMessage *pmNewHandle
    )
{
    //Sets the Popup Message handle
    mFormAuto = pmNewHandle;
}

//=============================================================================
//=============================================================================
void
UwxPredefinedCommands::on_btn_Load_clicked(
    )
{
    //Load button clicked
    QString strLoadFile;
    strLoadFile = QFileDialog::getOpenFileName(this, tr("Open Predefined Commands File"), "", tr("JSON Files (*.json);;All Files (*.*)"));
    if (strLoadFile != "")
    {
        //We have a file to load!
        LoadFile(strLoadFile);
    }
}

//=============================================================================
//=============================================================================
void
UwxPredefinedCommands::on_btn_Save_clicked(
    )
{
    //Save button clicked
    QString strSaveFile;
    strSaveFile = QFileDialog::getSaveFileName(this, tr("Save Predefined Commands File"), "", tr("JSON Files (*.json);;All Files (*.*)"));
    if (strSaveFile != "")
    {
        // We have a file to save!
        SaveFile(strSaveFile);
    }
}

//=============================================================================
//=============================================================================
void
    UwxPredefinedCommands::on_btn_Add_clicked(
        )
{
    bool ok;
    QStringList list = InputDialog::getStrings(this, &ok);
    if (ok)
    {
        addCommand(list[0], list[1]);
        getCurrentPredefinedCommandsTab()->getRemoveButton()->setEnabled(true);
    }
}

//=============================================================================
//=============================================================================
void UwxPredefinedCommands::on_btn_Remove_clicked()
{
    QTableView *currentTableView = getCurrentTableView();
    PredefinedCommandsTableModel *currentTableModel = getCurrentTableModel();
    QItemSelectionModel *select = currentTableView->selectionModel();
    if (select->hasSelection())
    {
        QModelIndexList selection = select->selectedIndexes();
        QList<QUuid> *commandsToRemove = new QList<QUuid>;
        for (int i = 0; i < selection.count(); i++)
        {
            QModelIndex index = selection.at(i);
            commandsToRemove->append(currentTableModel->getPredefinedCommandAt(index.row())->getUuid());
        }
        for (int i = 0; i< commandsToRemove->count(); i++)
        {
            currentTableModel->removeCommand(commandsToRemove->at(i));
        }
        getCurrentPredefinedCommandsTab()->getRemoveButton()->setEnabled(currentTableModel->rowCount(QModelIndex()) > 1);
    }
}

//=============================================================================
//=============================================================================
void UwxPredefinedCommands::on_btn_Add_Tab_clicked()
{
    bool ok = true;
    QString newGroupName = QInputDialog::getText(
        this,
        tr("Add New Group"),
        tr("New Group Name:"),
        QLineEdit::Normal,
        "New Group", &ok);

    if (ok)
    {
        addCommandGroup(newGroupName);
        ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
        ui->btn_Remove_Tab->setEnabled(true);
    }
}

//=============================================================================
//=============================================================================
void
    UwxPredefinedCommands::on_btn_Remove_Tab_clicked(
        )
{
    if (ui->tabWidget->count() > 1) {
        PredefinedCommandsTableModel *currentTableModel = getCurrentTableModel();
        if (QMessageBox::question(this,
                                  "Remove Group",
                                  QString("Are you sure you want to remove group \"%1\"?").arg(currentTableModel->getCommandGroup()->getName()),
                                  QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
            ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
            delete currentTableModel;
            ui->btn_Remove_Tab->setEnabled(ui->tabWidget->count() > 1);
        }
    }
}

//=============================================================================
//=============================================================================
void
UwxPredefinedCommands::on_btn_Close_clicked(
    )
{
    //Close button clicked
    this->close();
}

//=============================================================================
//=============================================================================
void
UwxPredefinedCommands::ConnectionChange(
    bool bEnabled
    )
{
    //Enable or disable interactivity of the window
    this->serialPortOpen = bEnabled;

    // Enabled/disable 'Send' buttons based on new connection state
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        UwxPredefinedCommandsTab *currentTab = reinterpret_cast<UwxPredefinedCommandsTab *>(ui->tabWidget->widget(i));
        QTableView *currentView = currentTab->getTableView();
        PredefinedCommandsTableModel *currentModel = reinterpret_cast<PredefinedCommandsTableModel *>(currentView->model());
        for (int j = 0; j < currentModel->rowCount(QModelIndex()); j++)
        {
            QPushButton *currentButton = reinterpret_cast<QPushButton *>(currentView->indexWidget(currentModel->index(j, 0)));
            currentButton->setEnabled(this->serialPortOpen);
        }
    }
}

//=============================================================================
//=============================================================================
void UwxPredefinedCommands::sendButtonClicked(const QUuid commandUuid)
{
    PredefinedCommandsTableModel *currentTableModel = getCurrentTableModel();
    CommandGroup *commandGroup = currentTableModel->getCommandGroup();
    for (int i = 0; i < commandGroup->count(); i++)
    {
        PredefinedCommand *command = commandGroup->getPredefinedCommandAt(i);
        if (command->getUuid() == commandUuid)
        {
            emit SendData(command->getCommand().toUtf8(), ui->check_Unescape->isChecked(), false);
            return;
        }
    }
    qDebug() << "Error! Command not found";
}

//=============================================================================
//=============================================================================
UwxPredefinedCommandsTab *UwxPredefinedCommands::getCurrentPredefinedCommandsTab()
{
    return reinterpret_cast<UwxPredefinedCommandsTab *>(ui->tabWidget->currentWidget());
}

//=============================================================================
//=============================================================================
QTableView *UwxPredefinedCommands::getCurrentTableView()
{
    return getCurrentPredefinedCommandsTab()->getTableView();
}

//=============================================================================
//=============================================================================
PredefinedCommandsTableModel *UwxPredefinedCommands::getCurrentTableModel()
{
    return reinterpret_cast<PredefinedCommandsTableModel *>(getCurrentTableView()->model());
}

//=============================================================================
//=============================================================================
void
UwxPredefinedCommands::on_btn_Clear_clicked(
    )
{
    getCurrentTableModel()->clear();
    getCurrentTableView()->reset();
    getCurrentPredefinedCommandsTab()->getRemoveButton()->setEnabled(false);
}

//=============================================================================
//=============================================================================
void
UwxPredefinedCommands::on_check_OnTop_stateChanged(
    int
    )
{
    //Always on-top state changed
    bool bReShow = this->isVisible();

    if (ui->check_OnTop->isChecked())
    {
        //Always on top
        this->setWindowFlags((Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint));
    }
    else
    {
        //Not always on top
        this->setWindowFlags((Qt::Window | Qt::WindowCloseButtonHint));
    }

    if (bReShow == true)
    {
        //Repaint (show) window
        this->show();
    }
}

//=============================================================================
//=============================================================================
void
UwxPredefinedCommands::TempAlwaysOnTop(
    bool bEnabled
    )
{
    //This is used to temporally disable the always on top setting so that it doesn't overlap file open dialogues
    if (this->isVisible() && ui->check_OnTop->isChecked())
    {
        //Window is visible and always on top is checked
        if (bEnabled == true)
        {
            //Restore setting
            on_check_OnTop_stateChanged(0);
        }
        else
        {
            //Disable
            this->setWindowFlags((Qt::Window | Qt::WindowCloseButtonHint));
            this->show();
        }
    }
}

//=============================================================================
//=============================================================================
void
UwxPredefinedCommands::SaveFile(
    QString strSaveFile
    )
{
    QFile file(strSaveFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        // Unable to open file
        QString strMessage = tr("Error during predefined commands file save: Access to selected file is denied: ").append(strSaveFile);
        mFormAuto->SetMessage(&strMessage);
        mFormAuto->show();
        return;
    }

    QJsonObject json;

    QJsonArray groups;
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {

        UwxPredefinedCommandsTab *tab = reinterpret_cast<UwxPredefinedCommandsTab *>(ui->tabWidget->widget(i));
        QTableView *tableView = tab->getTableView();
        PredefinedCommandsTableModel *model = reinterpret_cast<PredefinedCommandsTableModel *>(tableView->model());
        groups.append(model->getCommandGroup()->toJson());
    }
    json["groups"] = groups;

    file.write(QJsonDocument(json).toJson());
    file.flush();
    file.close();
    msbStatusBar->showMessage(QString(strSaveFile).append(": saved."));
}

//=============================================================================
//=============================================================================
void
UwxPredefinedCommands::LoadFile(
    QString strLoadFile
    )
{
    QFile file(strLoadFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //Unable to open file
        QString strMessage = tr("Error during predefined commands file open: Access to selected file is denied: ").append(strLoadFile);
        mFormAuto->SetMessage(&strMessage);
        mFormAuto->show();
        return;
    }

    QByteArray saveData = file.readAll();
    file.close();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    QJsonObject json = loadDoc.object();

    const QJsonValue groupsValue = json["groups"];
    if (groupsValue.isArray()) {
        const QJsonArray groups = groupsValue.toArray();

        // Clear current commands
        int tabCount = ui->tabWidget->count();
        for (int i = tabCount - 1; i >= 0; i--)
        {
            UwxPredefinedCommandsTab *tab = reinterpret_cast<UwxPredefinedCommandsTab *>(ui->tabWidget->widget(i));
            QTableView *tableView = tab->getTableView();
            PredefinedCommandsTableModel *model = reinterpret_cast<PredefinedCommandsTableModel *>(tableView->model());

            model->clear();
            tableView->reset();
            ui->tabWidget->removeTab(i);
            delete model;
            delete tab;
        }

        for (const QJsonValue &group : groups)
        {
            CommandGroup *newGroup = CommandGroup::fromJson(group.toObject());
            UwxPredefinedCommandsTab *newTab = addCommandGroup(newGroup);
            for (int i = 0; i < newGroup->count(); i++)
            {
                addSendButton(newGroup->getPredefinedCommandAt(i), newTab->getTableView(), i);
            }

            newTab->getRemoveButton()->setEnabled(newGroup->count() > 0);
        }

        if (ui->tabWidget->count() == 0) {
            // If no groups were added, add a default one
            addCommandGroup("New Group");
        }

        ui->btn_Remove_Tab->setEnabled(true);
    }
    msbStatusBar->showMessage(QString(strLoadFile).append(": loaded."));
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
