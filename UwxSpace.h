#ifndef UWXSPACE_H
#define UWXSPACE_H

#include <QDialog>

namespace Ui
{
    class UwxSpace;
}

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
