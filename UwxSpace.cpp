#include "UwxSpace.h"
#include "ui_UwxSpace.h"

//=============================================================================
//=============================================================================
UwxSpace::UwxSpace(QWidget *parent) : QDialog(parent), ui(new Ui::UwxSpace)
{
    ui->setupUi(this);
}

//=============================================================================
//=============================================================================
UwxSpace::~UwxSpace(
    )
{
    delete ui;
}

//=============================================================================
//=============================================================================
void
UwxSpace::on_btn_Close_clicked(
    )
{
    //Close button clicked, close popup.
    this->close();
}

//=============================================================================
//=============================================================================
void
UwxSpace::SetupBars(
    unsigned long ulTotal,
    unsigned long ulFree,
    unsigned long ulDeleted
    )
{
    //Calculate the used space
    unsigned long ulUsed = ulTotal - ulFree - ulDeleted;

    //Set the progress bars up
    ui->bar_Used->setValue(ulUsed*100/ulTotal);
    ui->bar_Deleted->setValue(ulDeleted*100/ulTotal);
    ui->bar_Free->setValue(ulFree*100/ulTotal);
}

//=============================================================================
//=============================================================================
void
UwxSpace::SetupMode(
    bool bMode
    )
{
    if (bMode == true)
    {
        //FAT
        ui->label_Type->setText("FAT segment");
    }
    else
    {
        //Data
        ui->label_Type->setText("Data segment");
    }
}
