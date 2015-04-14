/******************************************************************************
** Copyright (C) 2014-2015 Ezurio Ltd
**
** Project: UwTerminalX
**
** Module: LrdScrollEdit.cpp
**
** Notes:
**
*******************************************************************************/

/******************************************************************************/
// Include Files
/******************************************************************************/
#include "LrdScrollEdit.h"

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
LrdScrollEdit::LrdScrollEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    //Enable an event filter
    installEventFilter(this);
    mchItems = 0; //Number of items
    mchPosition = 0; //Current position
    mbLineMode = true; //True enables line mode
//    connect(PasteTimer, SIGNAL(timeout()), this, SLOT(SendPasteData()));
}

//=============================================================================
//=============================================================================
bool
LrdScrollEdit::eventFilter
    (
    QObject *target,
    QEvent *event
    )
{
    if (event->type() == QEvent::KeyPress)
    {
        //Key has been pressed...
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Up && !(keyEvent->modifiers() & Qt::ShiftModifier))
        {
            //Up pressed without holding shift
            if (mchPosition > 0)
            {
                mchPosition = mchPosition-1;
            }
            this->setPlainText(mstrItemArray[mchPosition]);
            this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Down && !(keyEvent->modifiers() & Qt::ShiftModifier))
        {
            //Down pressed without holding shift
            if (mchPosition < mchItems)
            {
                mchPosition = mchPosition+1;
            }
            this->setPlainText(mstrItemArray[mchPosition]);
            this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
            return true;
        }
        else if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && !(keyEvent->modifiers() & Qt::ShiftModifier) && !(keyEvent->modifiers() & Qt::ControlModifier))
        {
            //Enter pressed
//            if (this->toPlainText() != ItemArray[Items])
//            {
                //Previous entry is not the same as this entry
                if (mchItems > (ItemAllow-1))
                {
                    //Shift out last array item
                    unsigned char i = 1;
                    while (i < ItemAllow)
                    {
                        mstrItemArray[(i-1)] = mstrItemArray[i];
                        ++i;
                    }
                    mchItems--;
                }
                mstrItemArray[mchItems] = this->toPlainText();
                mchItems++;
                mchPosition = mchItems;
//            }

            //Send message to main window
            emit EnterPressed();
            return true;
        }
        else
        {
            if (mbLineMode == true)
            {
                //Line mode - pass on
                return QObject::eventFilter(target, event);
            }
            else
            {
                //Character mode`
                if (!(keyEvent->modifiers() & Qt::ControlModifier))
                {
                    //Control key not held down
                    if ((keyEvent->key() > 31 && keyEvent->key() < 128) || keyEvent->key() == 9 || keyEvent->key() == 10 || keyEvent->key() == 13)
                    {
                        //Not a special character
                        if (!(keyEvent->modifiers() & Qt::ShiftModifier) && keyEvent->key() > 64 && keyEvent->key() < 91)
                        {
                            //Shift isn't down, lowercase it
                            emit KeyPressed(keyEvent->key()+32);
                        }
                        else
                        {
                            emit KeyPressed(keyEvent->key());
                        }
                    }
                    return true;
                }
                else
                {
                    //Control key held down
                    if (keyEvent->key() == Qt::Key_V)
                    {
                        //Paste command
                        QTimer::singleShot(20, this, SLOT(SendPasteData()));
                    }
                }
            }
        }
    }
    return QObject::eventFilter(target, event);
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::SetLineMode
    (
    bool bNewLineMode
    )
{
    //Enables or disables line mode
    mbLineMode = bNewLineMode;
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::SendPasteData
    (
    )
{
    //Private slot to send data
    emit EnterPressed();
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
