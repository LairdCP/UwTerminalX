/******************************************************************************
** Copyright (C) 2014-2015 Ezurio Ltd
**
** Project: UwTerminalX
**
** Module: UwxPopup.h
**
** Notes:
**
*******************************************************************************/
#ifndef UWXAUTOMATION_H
#define UWXAUTOMATION_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QDialog>
#include "UwxPopup.h"
#include "UwxMainWindow.h"
#include <QFileDialog>
#include <QFile>

/******************************************************************************/
// Defines
/******************************************************************************/
#define AutoItemAllow 40        //Number of items in the list to allow
#define SaveFilesWithUTF8Header //Define to save files with a UTF8 BOM header

/******************************************************************************/
// Forward declaration of Class, Struct & Unions
/******************************************************************************/
namespace Ui
{
    class UwxAutomation;
}

/******************************************************************************/
// Class definitions
/******************************************************************************/
class UwxAutomation : public QDialog
{
    Q_OBJECT

public:
    explicit UwxAutomation
    (
    QWidget *parent = 0
    );
    ~UwxAutomation();
    void SetPopupHandle
    (
    PopupMessage *pmNewHandle
    );
    void SetMainHandle
    (
    MainWindow *mwNewHandle
    );
    void ConnectionChange
    (
    bool bEnabled
    );

private slots:
    void on_btn_Load_clicked
    (
    );
    void on_btn_Save_clicked
    (
    );
    void on_btn_Top_clicked
    (
    );
    void on_btn_Up_clicked
    (
    );
    void on_btn_Down_clicked
    (
    );
    void on_btn_Bottom_clicked
    (
    );
    void on_btn_Close_clicked
    (
    );
    void on_btn_Send1_clicked
    (
    );
    void on_btn_Send2_clicked
    (
    );
    void on_btn_Send3_clicked
    (
    );
    void on_btn_Send4_clicked
    (
    );
    void on_btn_Send5_clicked
    (
    );
    void on_btn_Send6_clicked
    (
    );
    void on_btn_Send7_clicked
    (
    );
    void on_btn_Send8_clicked
    (
    );
    void on_btn_Send9_clicked
    (
    );
    void on_btn_Send10_clicked
    (
    );
    void LoadTextData
    (
    );
    void on_edit_Line1_editingFinished
    (
    );
    void on_edit_Line2_editingFinished
    (
    );
    void on_edit_Line3_editingFinished
    (
    );
    void on_edit_Line4_editingFinished
    (
    );
    void on_edit_Line5_editingFinished
    (
    );
    void on_edit_Line6_editingFinished
    (
    );
    void on_edit_Line7_editingFinished
    (
    );
    void on_edit_Line8_editingFinished
    (
    );
    void on_edit_Line9_editingFinished
    (
    );
    void on_edit_Line10_editingFinished
    (
    );
    void ArrayHighest
    (
    );
    void ArrayPositionUpdate
    (
    );
    void EnterPressed
    (
    );
    void on_btn_Clear_clicked
    (
    );
    void on_check_OnTop_stateChanged
    (
    int arg1
    );

private:
    Ui::UwxAutomation *ui;
    PopupMessage *mFormAuto; //Holds handle of error message dialogue
    MainWindow *mMainAuto; //Holds handle of main window
    QString mstrAutoItemArray[(AutoItemAllow+1)]; //Holds the text items
    unsigned char mchItemPosition; //Current position of the array for the text boxes
    unsigned char mchItemTotal; //Total number of array entries
    unsigned char mchItemHighest; //Highest number in the array with an entry that has data
};

#endif // UWXAUTOMATION_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
