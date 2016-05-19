/******************************************************************************
** Copyright (C) 2015-2016 Laird
**
** Project: UwTerminalX
**
** Module: UwxAutomation.cpp
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

/******************************************************************************/
// Include Files
/******************************************************************************/
#include "UwxAutomation.h"
#include "ui_UwxAutomation.h"

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
UwxAutomation::UwxAutomation(QWidget *parent) : QDialog(parent), ui(new Ui::UwxAutomation){
    //On dialogue creation
    this->setWindowFlags((Qt::Dialog | Qt::WindowCloseButtonHint));
    ui->setupUi(this);
    this->hide();

    //Default values
    mchItemPosition = 0;
    mchItemTotal = 20;
    mchItemHighest = 0;

    //Update the position label
    ArrayPositionUpdate();

    //Connect signals
    connect(ui->edit_Line1, SIGNAL(returnPressed()), this, SLOT(EnterPressed()));
    connect(ui->edit_Line2, SIGNAL(returnPressed()), this, SLOT(EnterPressed()));
    connect(ui->edit_Line3, SIGNAL(returnPressed()), this, SLOT(EnterPressed()));
    connect(ui->edit_Line4, SIGNAL(returnPressed()), this, SLOT(EnterPressed()));
    connect(ui->edit_Line5, SIGNAL(returnPressed()), this, SLOT(EnterPressed()));
    connect(ui->edit_Line6, SIGNAL(returnPressed()), this, SLOT(EnterPressed()));
    connect(ui->edit_Line7, SIGNAL(returnPressed()), this, SLOT(EnterPressed()));
    connect(ui->edit_Line8, SIGNAL(returnPressed()), this, SLOT(EnterPressed()));
    connect(ui->edit_Line9, SIGNAL(returnPressed()), this, SLOT(EnterPressed()));

    //Create status bar
    msbStatusBar = new QStatusBar;
    ui->StatusBarLayout->addWidget(msbStatusBar);
    msbStatusBar->showMessage("");

    //Set always on top state
    on_check_OnTop_stateChanged(0);
}

//=============================================================================
//=============================================================================
UwxAutomation::~UwxAutomation(
    )
{
    //On dialogue deletion
    disconnect(ui->edit_Line1, SIGNAL(returnPressed()));
    disconnect(ui->edit_Line2, SIGNAL(returnPressed()));
    disconnect(ui->edit_Line3, SIGNAL(returnPressed()));
    disconnect(ui->edit_Line4, SIGNAL(returnPressed()));
    disconnect(ui->edit_Line5, SIGNAL(returnPressed()));
    disconnect(ui->edit_Line6, SIGNAL(returnPressed()));
    disconnect(ui->edit_Line7, SIGNAL(returnPressed()));
    disconnect(ui->edit_Line8, SIGNAL(returnPressed()));
    disconnect(ui->edit_Line9, SIGNAL(returnPressed()));
    delete msbStatusBar;
    delete ui;
}

//=============================================================================
//=============================================================================
void
UwxAutomation::SetPopupHandle(
    PopupMessage *pmNewHandle
    )
{
    //Sets the Popup Message handle
    mFormAuto = pmNewHandle;
}

//=============================================================================
//=============================================================================
void
UwxAutomation::SetMainHandle(
    MainWindow *mwNewHandle
    )
{
    //Sets the main window handle
    mMainAuto = mwNewHandle;
    this->setParent(mMainAuto);
    this->setWindowFlags((Qt::WindowCloseButtonHint | Qt::Window));
    this->setModal(false);
    this->hide();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Load_clicked(
    )
{
    //Load button clicked
    QString strLoadFile;
    strLoadFile = QFileDialog::getOpenFileName(this, tr("Open Automation File"), "", tr("Text Files (*.txt);;All Files (*.*)"));
    if (strLoadFile != "")
    {
        //We have a file to load!
        QFile file(strLoadFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            //Unable to open file
            QString strMessage = tr("Error during automation file open: Access to selected file is denied: ").append(strLoadFile);
            mFormAuto->SetMessage(&strMessage);
            mFormAuto->show();
            return;
        }

        //Clear out the array
        unsigned char i = 0;
        while (i < AutoItemAllow)
        {
            mstrAutoItemArray[i] = "";
            ++i;
        }

        //Read the file data
        i = 0;
        while (!file.atEnd())
        {
            QByteArray baThisLine = file.readLine();
            mstrAutoItemArray[i] = baThisLine.replace("\r", "").replace("\n", "");
            ++i;
            if (i > AutoItemAllow)
            {
                //Maximum lines reached, stop processing
                --i;
                break;
            }
        }

        //Close the file handle
        file.close();

        //Show number of lines loaded
        msbStatusBar->showMessage(QString(strLoadFile).append(": ").append(QString::number(i)).append(" line").append((i == 1 ? " loaded." : "s loaded.")));

        //Update the text boxes
        mchItemPosition = 0;
        LoadTextData();
    }
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Save_clicked(
    )
{
    //Save button clicked
    QString strSaveFile;
    strSaveFile = QFileDialog::getSaveFileName(this, tr("Save Automation File"), "", tr("Text Files (*.txt);;All Files (*.*)"));
    if (strSaveFile != "")
    {
        //We have a file to load!
        QFile file(strSaveFile);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            //Unable to open file
            QString strMessage = tr("Error during automation file save: Access to selected file is denied: ").append(strSaveFile);
            mFormAuto->SetMessage(&strMessage);
            mFormAuto->show();
            return;
        }

#ifdef SaveFilesWithUTF8Header
        //Output UTF-8 BOM header
        QByteArray baBOM;
        baBOM[0] = 0xEF;
        baBOM[1] = 0xBB;
        baBOM[2] = 0xBF;
        file.write(baBOM);
#endif

        //Save the data
        unsigned char i = 0;
        ArrayHighest();
        while (i <= mchItemHighest)
        {
            file.write(mstrAutoItemArray[i].append("\n").toUtf8());
            ++i;
        }

        //Flush the file output and close the file handke
        file.flush();
        file.close();

        //
        msbStatusBar->showMessage(QString(strSaveFile).append(": ").append(QString::number(i)).append(" line").append((i == 1 ? " saved." : "s saved.")));
    }
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Top_clicked(
    )
{
    //Top button clicked
    mchItemPosition = 0;
    LoadTextData();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Up_clicked(
    )
{
    //Up button clicked
    if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
    {
        //Scroll by 10
        if (mchItemPosition > 9)
        {
            mchItemPosition = mchItemPosition-10;
            LoadTextData();
        }
        else if (mchItemPosition > 0)
        {
            mchItemPosition = 0;
            LoadTextData();
        }
    }
    else
    {
        //Scroll by 1
        if (mchItemPosition > 0)
        {
            --mchItemPosition;
            LoadTextData();
        }
    }
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Down_clicked(
    )
{
    //Down button clicked
    if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
    {
        //Scroll by 10
        if (mchItemPosition < AutoItemAllow-20)
        {
            mchItemPosition = mchItemPosition+10;
            LoadTextData();
        }
        else
        {
            mchItemPosition = AutoItemAllow-10;
            LoadTextData();
        }
    }
    else
    {
        //Scroll by 1
        if (mchItemPosition < AutoItemAllow-10)
        {
            ++mchItemPosition;
            LoadTextData();
        }
    }
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Bottom_clicked(
    )
{
    //Bottom button clicked
    mchItemPosition = AutoItemAllow-10;
    LoadTextData();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Close_clicked(
    )
{
    //Close button clicked
    this->close();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Send1_clicked(
    )
{
    //Send button #1 pressed
    mMainAuto->MessagePass(ui->edit_Line1->text(), ui->check_Unescape->isChecked());
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Send2_clicked(
    )
{
    //Send button #2 pressed
    mMainAuto->MessagePass(ui->edit_Line2->text(), ui->check_Unescape->isChecked());
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Send3_clicked(
    )
{
    //Send button #3 pressed
    mMainAuto->MessagePass(ui->edit_Line3->text(), ui->check_Unescape->isChecked());
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Send4_clicked(
    )
{
    //Send button #4 pressed
    mMainAuto->MessagePass(ui->edit_Line4->text(), ui->check_Unescape->isChecked());
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Send5_clicked(
    )
{
    //Send button #5 pressed
    mMainAuto->MessagePass(ui->edit_Line5->text(), ui->check_Unescape->isChecked());
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Send6_clicked(
    )
{
    //Send button #6 pressed
    mMainAuto->MessagePass(ui->edit_Line6->text(), ui->check_Unescape->isChecked());
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Send7_clicked(
    )
{
    //Send button #7 pressed
    mMainAuto->MessagePass(ui->edit_Line7->text(), ui->check_Unescape->isChecked());
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Send8_clicked(
    )
{
    //Send button #8 pressed
    mMainAuto->MessagePass(ui->edit_Line8->text(), ui->check_Unescape->isChecked());
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Send9_clicked(
    )
{
    //Send button #9 pressed
    mMainAuto->MessagePass(ui->edit_Line9->text(), ui->check_Unescape->isChecked());
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Send10_clicked(
    )
{
    //Send button #10 pressed
    mMainAuto->MessagePass(ui->edit_Line10->text(), ui->check_Unescape->isChecked());
}

//=============================================================================
//=============================================================================
void
UwxAutomation::LoadTextData(
    )
{
    //Loads the text boxes with the data in the array
    ui->edit_Line1->setText(mstrAutoItemArray[mchItemPosition]);
    ui->edit_Line2->setText(mstrAutoItemArray[(mchItemPosition+1)]);
    ui->edit_Line3->setText(mstrAutoItemArray[(mchItemPosition+2)]);
    ui->edit_Line4->setText(mstrAutoItemArray[(mchItemPosition+3)]);
    ui->edit_Line5->setText(mstrAutoItemArray[(mchItemPosition+4)]);
    ui->edit_Line6->setText(mstrAutoItemArray[(mchItemPosition+5)]);
    ui->edit_Line7->setText(mstrAutoItemArray[(mchItemPosition+6)]);
    ui->edit_Line8->setText(mstrAutoItemArray[(mchItemPosition+7)]);
    ui->edit_Line9->setText(mstrAutoItemArray[(mchItemPosition+8)]);
    ui->edit_Line10->setText(mstrAutoItemArray[(mchItemPosition+9)]);
    ArrayPositionUpdate();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::ConnectionChange(
    bool bEnabled
    )
{
    //Enable or disable interactivity of the window
    ui->btn_Send1->setEnabled(bEnabled);
    ui->btn_Send2->setEnabled(bEnabled);
    ui->btn_Send3->setEnabled(bEnabled);
    ui->btn_Send4->setEnabled(bEnabled);
    ui->btn_Send5->setEnabled(bEnabled);
    ui->btn_Send6->setEnabled(bEnabled);
    ui->btn_Send7->setEnabled(bEnabled);
    ui->btn_Send8->setEnabled(bEnabled);
    ui->btn_Send9->setEnabled(bEnabled);
    ui->btn_Send10->setEnabled(bEnabled);
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_edit_Line1_editingFinished(
    )
{
    //Update array with new text
    mstrAutoItemArray[mchItemPosition] = ui->edit_Line1->text();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_edit_Line2_editingFinished(
    )
{
    //Update array with new text
    mstrAutoItemArray[(mchItemPosition+1)] = ui->edit_Line2->text();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_edit_Line3_editingFinished(
    )
{
    //Update array with new text
    mstrAutoItemArray[(mchItemPosition+2)] = ui->edit_Line3->text();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_edit_Line4_editingFinished(
    )
{
    //Update array with new text
    mstrAutoItemArray[(mchItemPosition+3)] = ui->edit_Line4->text();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_edit_Line5_editingFinished(
    )
{
    //Update array with new text
    mstrAutoItemArray[(mchItemPosition+4)] = ui->edit_Line5->text();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_edit_Line6_editingFinished(
    )
{
    //Update array with new text
    mstrAutoItemArray[(mchItemPosition+5)] = ui->edit_Line6->text();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_edit_Line7_editingFinished(
    )
{
    //Update array with new text
    mstrAutoItemArray[(mchItemPosition+6)] = ui->edit_Line7->text();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_edit_Line8_editingFinished(
    )
{
    //Update array with new text
    mstrAutoItemArray[(mchItemPosition+7)] = ui->edit_Line8->text();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_edit_Line9_editingFinished(
    )
{
    //Update array with new text
    mstrAutoItemArray[(mchItemPosition+8)] = ui->edit_Line9->text();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_edit_Line10_editingFinished(
    )
{
    //Update array with new text
    mstrAutoItemArray[(mchItemPosition+9)] = ui->edit_Line10->text();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::ArrayHighest(
    )
{
    //Gets the highest position for a valid entry in the array
    unsigned char i = 0;
    mchItemHighest = 0;
    while (i < AutoItemAllow)
    {
        if (mstrAutoItemArray[i] != "" )
        {
            mchItemHighest = i;
        }
        ++i;
    }
}

//=============================================================================
//=============================================================================
void
UwxAutomation::ArrayPositionUpdate(
    )
{
    //Updates the label to show the current array position
    ui->label_Position->setText(QString('(').append(QString::number(mchItemPosition)).append('/').append(QString::number(AutoItemAllow-10)).append(')'));
}

//=============================================================================
//=============================================================================
void
UwxAutomation::EnterPressed(
    )
{
    //Enter has been pressed with a line edit selected
    if (ui->edit_Line1->hasFocus())
    {
        mMainAuto->MessagePass(ui->edit_Line1->text(), ui->check_Unescape->isChecked());
    }
    else if (ui->edit_Line2->hasFocus())
    {
        mMainAuto->MessagePass(ui->edit_Line2->text(), ui->check_Unescape->isChecked());
    }
    else if (ui->edit_Line3->hasFocus())
    {
        mMainAuto->MessagePass(ui->edit_Line3->text(), ui->check_Unescape->isChecked());
    }
    else if (ui->edit_Line4->hasFocus())
    {
        mMainAuto->MessagePass(ui->edit_Line4->text(), ui->check_Unescape->isChecked());
    }
    else if (ui->edit_Line5->hasFocus())
    {
        mMainAuto->MessagePass(ui->edit_Line5->text(), ui->check_Unescape->isChecked());
    }
    else if (ui->edit_Line6->hasFocus())
    {
        mMainAuto->MessagePass(ui->edit_Line6->text(), ui->check_Unescape->isChecked());
    }
    else if (ui->edit_Line7->hasFocus())
    {
        mMainAuto->MessagePass(ui->edit_Line7->text(), ui->check_Unescape->isChecked());
    }
    else if (ui->edit_Line8->hasFocus())
    {
        mMainAuto->MessagePass(ui->edit_Line8->text(), ui->check_Unescape->isChecked());
    }
    else if (ui->edit_Line9->hasFocus())
    {
        mMainAuto->MessagePass(ui->edit_Line9->text(), ui->check_Unescape->isChecked());
    }
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_btn_Clear_clicked(
    )
{
    //Clears the array and all edit boxes and resets back to default
    unsigned char i = 0;
    while (i < AutoItemAllow)
    {
        mstrAutoItemArray[i] = "";
        ++i;
    }
    mchItemPosition = 0;
    mchItemTotal = 20;
    mchItemHighest = 0;
    ui->edit_Line1->setText("");
    ui->edit_Line2->setText("");
    ui->edit_Line3->setText("");
    ui->edit_Line4->setText("");
    ui->edit_Line5->setText("");
    ui->edit_Line6->setText("");
    ui->edit_Line7->setText("");
    ui->edit_Line8->setText("");
    ui->edit_Line9->setText("");
    ui->edit_Line10->setText("");

    //Update array position text
    ArrayPositionUpdate();
}

//=============================================================================
//=============================================================================
void
UwxAutomation::on_check_OnTop_stateChanged(
    int
    )
{
    //Always on-top state changed
    if (ui->check_OnTop->isChecked())
    {
        //Always on top
        this->setWindowFlags((Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint));
    }
    else
    {
        //Not always on top
        this->setWindowFlags((Qt::Dialog | Qt::WindowCloseButtonHint));
    }
    this->show();
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
