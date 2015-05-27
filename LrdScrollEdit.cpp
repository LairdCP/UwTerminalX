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
    mchItems = 0; //Number of items is 0
    mchPosition = 0; //Current position is 0
    mbLineMode = true; //Line mode is on by default
    mbSerialOpen = false; //Serial port is not open by default
    mbLocalEcho = true; //Local echo mode on by default
    mstrDatIn = ""; //Data in is an empty string
    mstrDatOut = ""; //Data out is empty string
    muintCurPos = 0; //Current cursor position is 0
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
                mstrDatOut = mstrItemArray[mchPosition];
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
                mstrDatOut = mstrItemArray[mchPosition];
                this->UpdateDisplay();
                return true;
            }
            else if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && !(keyEvent->modifiers() & Qt::ShiftModifier) && !(keyEvent->modifiers() & Qt::ControlModifier))
            {
                //Enter pressed
                if (mbSerialOpen == true)
                {
                    if (mstrDatOut != mstrItemArray[mchItems])
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
                        mstrItemArray[mchItems] = mstrDatOut;
                        mchItems++;
                        mchPosition = mchItems;
                    }

                    //Send message to main window
                    emit EnterPressed();
                }
                this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                return true;
            }
            else if (keyEvent->key() == Qt::Key_Backspace)
            {
                this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                if ((keyEvent->modifiers() & Qt::ControlModifier))
                {
                    //Delete word
                    if (mstrDatOut.indexOf(" ") != -1)
                    {
                        mstrDatOut = mstrDatOut.left(mstrDatOut.lastIndexOf(" ")+1);
                    }
                    else
                    {
                        mstrDatOut = "";
                    }
                }
                else
                {
                    //Delete character
                    mstrDatOut = mstrDatOut.left(mstrDatOut.length()-1);
                    if (muintCurPos > mstrDatOut.length())
                    {
                        --muintCurPos;
                    }
                }
                this->UpdateDisplay();
                return true;
            }
#pragma warning("TODO: Add left/right code to Scroll edit")
/*            else if (keyEvent->key() == Qt::Key_Left)
            {
                if (keyEvent->modifiers() & Qt::ShiftModifier)
                {
                    //
                }
                else if (muintCurPos > 0)
                {
                    //
                    --muintCurPos;
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
                else if (muintCurPos < mstrDatOut.length())
                {
                    //
                    ++muintCurPos;
                    this->UpdateCursor();
                }
                return true;
            }*/
            else if (keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right)
            {
                return true;
            }
            else if (keyEvent->key() != Qt::Key_Escape && keyEvent->key() != Qt::Key_Tab && keyEvent->key() != Qt::Key_Backtab && keyEvent->key() != Qt::Key_Backspace && keyEvent->key() != Qt::Key_Insert && keyEvent->key() != Qt::Key_Delete && keyEvent->key() != Qt::Key_Pause && keyEvent->key() != Qt::Key_Print && keyEvent->key() != Qt::Key_SysReq && keyEvent->key() != Qt::Key_Clear && keyEvent->key() != Qt::Key_Home && keyEvent->key() != Qt::Key_End && keyEvent->key() != Qt::Key_Shift && keyEvent->key() != Qt::Key_Control && keyEvent->key() != Qt::Key_Meta && keyEvent->key() != Qt::Key_Alt && keyEvent->key() != Qt::Key_AltGr && keyEvent->key() != Qt::Key_CapsLock && keyEvent->key() != Qt::Key_NumLock && keyEvent->key() != Qt::Key_ScrollLock && !(keyEvent->modifiers() & Qt::ControlModifier))
            {
                //Add character
                this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                /*if (!(keyEvent->modifiers() & Qt::ShiftModifier) && keyEvent->key() > 64 && keyEvent->key() < 91)
                {
                    mstrDatOut += (keyEvent->key()+32);
                }
                else
                {
                    mstrDatOut += keyEvent->key();
                }*/

                //Possible work-around for caps lock issue
                if (keyEvent->key() > 64 && keyEvent->key() < 91)
                {
                    mstrDatOut += keyEvent->text();
                }
                else
                {
                    mstrDatOut += keyEvent->key();
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
                    this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                    if (keyEvent->key() != Qt::Key_Escape && keyEvent->key() != Qt::Key_Tab && keyEvent->key() != Qt::Key_Backtab && /*keyEvent->key() != Qt::Key_Backspace &&*/ keyEvent->key() != Qt::Key_Insert && keyEvent->key() != Qt::Key_Delete && keyEvent->key() != Qt::Key_Pause && keyEvent->key() != Qt::Key_Print && keyEvent->key() != Qt::Key_SysReq && keyEvent->key() != Qt::Key_Clear && keyEvent->key() != Qt::Key_Home && keyEvent->key() != Qt::Key_End && keyEvent->key() != Qt::Key_Shift && keyEvent->key() != Qt::Key_Control && keyEvent->key() != Qt::Key_Meta && keyEvent->key() != Qt::Key_Alt && keyEvent->key() != Qt::Key_AltGr && keyEvent->key() != Qt::Key_CapsLock && keyEvent->key() != Qt::Key_NumLock && keyEvent->key() != Qt::Key_ScrollLock)
                    {
                        //Not a special character
                        /*if (!(keyEvent->modifiers() & Qt::ShiftModifier) && keyEvent->key() > 64 && keyEvent->key() < 91)
                        {
                            //Shift isn't down, lowercase it
                            emit KeyPressed(keyEvent->key()+32);
                        }
                        else
                        {
                            emit KeyPressed(keyEvent->key());
                        }*/

                        //Possible work-around for caps lock issue
                        if (keyEvent->key() > 64 && keyEvent->key() < 91)
                        {
                            emit KeyPressed(keyEvent->text().toUtf8()[0]);
                        }
                        else
                        {
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

    if (mbLocalEcho == false)
    {
        //Return true now if local echo is off
        return true;
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
LrdScrollEdit::AddDatInText
    (
    QByteArray *bDat
    )
{
    mstrDatIn += QString(*bDat);
    if (mstrDatIn.length() == bDat->length() && (bDat[0] == "\r" || bDat[0] == "\n"))
    {
        mstrDatIn.remove(0, 1);
    }
    this->UpdateDisplay();
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::ClearDatIn
    (
    )
{
    mstrDatIn.clear();
    this->UpdateDisplay();
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::ClearDatOut
    (
    )
{
    mstrDatOut.clear();
    this->UpdateDisplay();
}

//=============================================================================
//=============================================================================
QString *
LrdScrollEdit::GetDatOut
    (
    )
{
    return &mstrDatOut;
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::insertFromMimeData
    (
    const QMimeData *mdSrc
    )
{
    if (mbLineMode == true && mdSrc->hasText() == true)
    {
        mstrDatOut += mdSrc->text();
        this->UpdateDisplay();
    }
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::UpdateDisplay
    (
    )
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
    if (mstrDatIn.length() > 0)
    {
        this->setPlainText(QString(mstrDatIn).append((mbLocalEcho == true ? "\n" : "")).append((mbLocalEcho == true ? mstrDatOut : "")));
    }
    else
    {
        this->setPlainText(mstrDatOut);
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

    if (muintCurPos == mstrDatOut.length()-1)
    {
        ++muintCurPos;
    }
    this->UpdateCursor();
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::UpdateCursor
    (
    )
{
    //Updates the text control's cursor position
#pragma warning("TODO: Add update cursor code.")
    /*
    QTextCursor tcTmpCur = this->textCursor();
    if (mstrDatIn.length() > 0)
    {
        tcTmpCur.setPosition(mstrDatIn.length()+1+muintCurPos);
    }
    else
    {
        tcTmpCur.setPosition(muintCurPos);
    }
    this->setTextCursor(tcTmpCur);*/
    this->moveCursor(QTextCursor::End);
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::SetSerialOpen
    (
    bool SerialOpen
    )
{
    mbSerialOpen = SerialOpen;
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
