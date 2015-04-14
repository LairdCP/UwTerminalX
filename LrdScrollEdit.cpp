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
#include <QScrollBar>
#include <QMimeData>
#include <QTextCursor>

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
    mbSerialOpen = false;
    DatIn = "";
    DatOut = "";
    CurPos = 0;
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
    //Line mode
    if (event->type() == QEvent::KeyPress)
    {
        //Key has been pressed...
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (mbLineMode == true)
        {
            //Line mode
            if (keyEvent->key() == Qt::Key_Up && !(keyEvent->modifiers() & Qt::ShiftModifier))
            {
                //Up pressed without holding shift
                if (mchPosition > 0)
                {
                    mchPosition = mchPosition-1;
                }
                DatOut = mstrItemArray[mchPosition];
                this->UpdateDisplay();
                //this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                return true;
            }
            else if (keyEvent->key() == Qt::Key_Down && !(keyEvent->modifiers() & Qt::ShiftModifier))
            {
                //Down pressed without holding shift
                if (mchPosition < mchItems)
                {
                    mchPosition = mchPosition+1;
                }
                //this->setPlainText(mstrItemArray[mchPosition]);
                //this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                DatOut = mstrItemArray[mchPosition];
                this->UpdateDisplay();
                return true;
            }
            else if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && !(keyEvent->modifiers() & Qt::ShiftModifier) && !(keyEvent->modifiers() & Qt::ControlModifier))
            {
                //Enter pressed
                if (mbSerialOpen == true)
                {
                    if (DatOut != mstrItemArray[mchItems])
                    {
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
                        //mstrItemArray[mchItems] = this->toPlainText();
                        mstrItemArray[mchItems] = DatOut;
                        mchItems++;
                        mchPosition = mchItems;
                    }

                    //Send message to main window
                    emit EnterPressed();
                }
                return true;
            }
            else if (keyEvent->key() == Qt::Key_Backspace)
            {
                if ((keyEvent->modifiers() & Qt::ControlModifier))
                {
                    //delete word
                    if (DatOut.indexOf(" ") != -1)
                    {
                        DatOut = DatOut.left(DatOut.lastIndexOf(" ")+1);
                    }
                    else
                    {
                        DatOut = "";
                    }
                }
                else
                {
                    //delete character
                    DatOut = DatOut.left(DatOut.length()-1);
                    if (CurPos > DatOut.length())
                    {
                        --CurPos;
                    }
                }
                this->UpdateDisplay();
                return true;
            }
            else if (keyEvent->key() == Qt::Key_Left)
            {
                if (keyEvent->modifiers() & Qt::ShiftModifier)
                {
                    //
                }
                else if (CurPos > 0)
                {
                    //
                    --CurPos;
                    this->UpdateCursor();
                }
                return true;
            }
            else if (keyEvent->key() == Qt::Key_Right)
            {
                if (keyEvent->modifiers() & Qt::ShiftModifier)
                {
                    //
                }
                else if (CurPos < DatOut.length())
                {
                    //
                    ++CurPos;
                    this->UpdateCursor();
                }
                return true;
            }
            else if (keyEvent->key() != Qt::Key_Escape && keyEvent->key() != Qt::Key_Tab && keyEvent->key() != Qt::Key_Backtab && keyEvent->key() != Qt::Key_Backspace && keyEvent->key() != Qt::Key_Insert && keyEvent->key() != Qt::Key_Delete && keyEvent->key() != Qt::Key_Pause && keyEvent->key() != Qt::Key_Print && keyEvent->key() != Qt::Key_SysReq && keyEvent->key() != Qt::Key_Clear && keyEvent->key() != Qt::Key_Home && keyEvent->key() != Qt::Key_End && keyEvent->key() != Qt::Key_Shift && keyEvent->key() != Qt::Key_Control && keyEvent->key() != Qt::Key_Meta && keyEvent->key() != Qt::Key_Alt && keyEvent->key() != Qt::Key_AltGr && keyEvent->key() != Qt::Key_CapsLock && keyEvent->key() != Qt::Key_NumLock && keyEvent->key() != Qt::Key_ScrollLock && !(keyEvent->modifiers() & Qt::ControlModifier))
            {
                //Add character
                if (!(keyEvent->modifiers() & Qt::ShiftModifier) && keyEvent->key() > 64 && keyEvent->key() < 91)
                {
                    DatOut += (keyEvent->key()+32);
                }
                else
                {
                    DatOut += keyEvent->key();
                }
            }
        }
        else
        {
            //Character mode
            if (mbSerialOpen == true)
            {
                if (!(keyEvent->modifiers() & Qt::ControlModifier))
                {
                    //Control key not held down
//                    if ((keyEvent->key() > 31 && keyEvent->key() < 128) || keyEvent->key() == 9 || keyEvent->key() == 10 || keyEvent->key() == 13)
//                  {
                    if (keyEvent->key() != Qt::Key_Escape && keyEvent->key() != Qt::Key_Tab && keyEvent->key() != Qt::Key_Backtab && keyEvent->key() != Qt::Key_Backspace && keyEvent->key() != Qt::Key_Insert && keyEvent->key() != Qt::Key_Delete && keyEvent->key() != Qt::Key_Pause && keyEvent->key() != Qt::Key_Print && keyEvent->key() != Qt::Key_SysReq && keyEvent->key() != Qt::Key_Clear && keyEvent->key() != Qt::Key_Home && keyEvent->key() != Qt::Key_End && keyEvent->key() != Qt::Key_Shift && keyEvent->key() != Qt::Key_Control && keyEvent->key() != Qt::Key_Meta && keyEvent->key() != Qt::Key_Alt && keyEvent->key() != Qt::Key_AltGr && keyEvent->key() != Qt::Key_CapsLock && keyEvent->key() != Qt::Key_NumLock && keyEvent->key() != Qt::Key_ScrollLock)
                    {
                        //Not a special character
                        if (!(keyEvent->modifiers() & Qt::ShiftModifier) && keyEvent->key() > 64 && keyEvent->key() < 91)
                        {
                            //Shift isn't down, lowercase it
                            //DatOut += keyEvent->key()+32;
                            emit KeyPressed(keyEvent->key()+32);
                        }
                        else
                        {
                            //DatOut += keyEvent->key();
                            emit KeyPressed(keyEvent->key());
                        }
                        this->UpdateDisplay();
                    }
                    return true;
                }
            }
            else
            {
                return true;
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
void LrdScrollEdit::AddText(QString Dat)
{
    DatIn += Dat;
    if (DatIn.length() == Dat.length() && (Dat[0] == '\r' || Dat[0] == '\n'))
    {
        DatIn.remove(0, 1);
    }
    this->UpdateDisplay();
}

//=============================================================================
//=============================================================================
void LrdScrollEdit::ClearText()
{
    DatIn.clear();
    this->UpdateDisplay();
}

//=============================================================================
//=============================================================================
void LrdScrollEdit::ClearText2()
{
    DatOut.clear();
    this->UpdateDisplay();
}



/* NEW FUNCTIONS BELOW */



//=============================================================================
//=============================================================================
QString LrdScrollEdit::GetText()
{
    return DatOut;
}

//=============================================================================
//=============================================================================
void LrdScrollEdit::insertFromMimeData(const QMimeData *src)
{
    if (mbLineMode == true && src->hasText() == true)
    {
        DatOut += src->text();
        this->UpdateDisplay();
    }
}

//=============================================================================
//=============================================================================
void LrdScrollEdit::UpdateDisplay()
{
    //Updates the receive text buffer, faster
    unsigned int Pos;
    if (this->verticalScrollBar()->sliderPosition() == this->verticalScrollBar()->maximum())
    {
        //Scroll to bottom
        Pos = 65535;
    }
    else
    {
        //Stay here
        Pos = this->verticalScrollBar()->sliderPosition();
    }
    this->setUpdatesEnabled(false);
    if (DatIn.length() > 0)
    {
        this->setPlainText(QString(DatIn).append("\n").append(DatOut));
    }
    else
    {
        this->setPlainText(DatOut);
    }
    this->setUpdatesEnabled(true);
    if (Pos == 65535)
    {
        //Bottom
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
        this->moveCursor(QTextCursor::End);
    }
    else
    {
        //Maintain
        this->verticalScrollBar()->setValue(Pos);
    }

    if (CurPos == DatOut.length()-1)
    {
        ++CurPos;
    }
    this->UpdateCursor();
}

//=============================================================================
//=============================================================================
void LrdScrollEdit::UpdateCursor()
{
    //Updates the text control's cursor position
    QTextCursor ab = this->textCursor();
    if (DatIn.length() > 0)
    {
        ab.setPosition(DatIn.length()+1+CurPos);
    }
    else
    {
        ab.setPosition(CurPos);
    }
    this->setTextCursor(ab);
}

//=============================================================================
//=============================================================================
void LrdScrollEdit::SetSerialOpen(bool SerialOpen)
{
    mbSerialOpen = SerialOpen;
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
