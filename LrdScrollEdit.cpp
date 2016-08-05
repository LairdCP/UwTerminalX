/******************************************************************************
** Copyright (C) 2015-2016 Laird
**
** Project: UwTerminalX
**
** Module: LrdScrollEdit.cpp
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
#include "LrdScrollEdit.h"

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
LrdScrollEdit::LrdScrollEdit(QWidget *parent) : QPlainTextEdit(parent){
    //Enable an event filter
    installEventFilter(this);
    installEventFilter(this->verticalScrollBar());
    mchItems = 0; //Number of items is 0
    mchPosition = 0; //Current position is 0
    mbLineMode = true; //Line mode is on by default
    mbSerialOpen = false; //Serial port is not open by default
    mbLocalEcho = true; //Local echo mode on by default
    mstrDatIn = ""; //Data in is an empty string
    mstrDatOut = ""; //Data out is empty string
    muintCurPos = 0; //Current cursor position is 0
    mbContextMenuOpen = false; //Context menu not currently open
}

//=============================================================================
//=============================================================================
bool
LrdScrollEdit::eventFilter(
    QObject *target,
    QEvent *event
    )
{
    if (target == this->verticalScrollBar())
    {
        //Vertical scroll bar event
        if (event->type() == QEvent::MouseButtonRelease)
        {
            //Button was released, update display buffer
            this->UpdateDisplay();
        }
    }
    else if (event->type() == QEvent::KeyPress)
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
                return true;
            }
            else if (keyEvent->key() == Qt::Key_Down && !(keyEvent->modifiers() & Qt::ShiftModifier))
            {
                //Down pressed without holding shift
                if (mchPosition < mchItems)
                {
                    mchPosition = mchPosition+1;
                }
                mstrDatOut = mstrItemArray[mchPosition];
                this->UpdateDisplay();
                return true;
            }
            else if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && !(keyEvent->modifiers() & Qt::ControlModifier) && (!(keyEvent->modifiers() & Qt::ShiftModifier) || mbLineSeparator == false))
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
            else if (keyEvent->key() == Qt::Key_Delete)
            {
                return true;
            }
            else if (keyEvent->key() != Qt::Key_Escape && keyEvent->key() != Qt::Key_Tab && keyEvent->key() != Qt::Key_Backtab && keyEvent->key() != Qt::Key_Backspace && keyEvent->key() != Qt::Key_Insert && keyEvent->key() != Qt::Key_Pause && keyEvent->key() != Qt::Key_Print && keyEvent->key() != Qt::Key_SysReq && keyEvent->key() != Qt::Key_Clear && keyEvent->key() != Qt::Key_Home && keyEvent->key() != Qt::Key_End && keyEvent->key() != Qt::Key_Shift && keyEvent->key() != Qt::Key_Control && keyEvent->key() != Qt::Key_Meta && keyEvent->key() != Qt::Key_Alt && keyEvent->key() != Qt::Key_AltGr && keyEvent->key() != Qt::Key_CapsLock && keyEvent->key() != Qt::Key_NumLock && keyEvent->key() != Qt::Key_ScrollLock && !(keyEvent->modifiers() & Qt::ControlModifier))
            {
                //Add character
                this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                mstrDatOut += keyEvent->text();
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
                        emit KeyPressed(*keyEvent->text().unicode());
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

        if (mbLocalEcho == false)
        {
            //Return true now if local echo is off
            return true;
        }
    }

    return QObject::eventFilter(target, event);
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::SetLineMode(
    bool bNewLineMode
    )
{
    //Enables or disables line mode
    mbLineMode = bNewLineMode;
    this->UpdateDisplay();
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::AddDatInText(
    QByteArray *baDat
    )
{
    //Adds data to the DatIn buffer
    mstrDatIn += QString(*baDat);
    if (mstrDatIn.length() == baDat->length() && (baDat[0] == "\r" || baDat[0] == "\n"))
    {
        mstrDatIn.remove(0, 1);
    }
    this->UpdateDisplay();
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::AddDatOutText(
    QString strDat
    )
{
    //Adds data to the DatOut buffer
    if (mbLineMode == true)
    {
        //Line mode
        mstrDatOut += strDat;
        this->UpdateDisplay();
    }
    else
    {
        //Character mode
        QChar qcTmpQC;
        foreach (qcTmpQC, strDat)
        {
            emit KeyPressed(qcTmpQC);
        }
    }
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::ClearDatIn(
    )
{
    //Clears the DatIn buffer
    mstrDatIn.clear();
    QTextCursor tcTmpTC(this->document());
    tcTmpTC.setPosition(0);
    this->setTextCursor(tcTmpTC);
    this->UpdateDisplay();
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::ClearDatOut(
    )
{
    //Clears the DatOut buffer
    mstrDatOut.clear();
    this->UpdateDisplay();
}

//=============================================================================
//=============================================================================
QString *
LrdScrollEdit::GetDatOut(
    )
{
    //Returns the DatOut buffer
    return &mstrDatOut;
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::insertFromMimeData(
    const QMimeData *mdSrc
    )
{
    if (mdSrc->hasUrls() == true)
    {
        //A file has been dropped
        QList<QUrl> urls = mdSrc->urls();
        if (urls.isEmpty())
        {
            //No files
            return;
        }
        else if (urls.length() > 1)
        {
            //More than 1 file - ignore
            return;
        }

        if (urls.first().toLocalFile().isEmpty())
        {
            //Invalid filename
            return;
        }

        //Send back to main application
        emit FileDropped(urls.first().toLocalFile());
    }
    else if (mdSrc->hasText() == true)
    {
        //Text has been pasted
        if (mbLineMode == true)
        {
            //Line mode
            mstrDatOut += mdSrc->text();
            this->UpdateDisplay();
        }
        else
        {
            //Character mode
            QString strTmpStr = mdSrc->text();
            QChar qcTmpQC;
            foreach (qcTmpQC, strTmpStr)
            {
                emit KeyPressed(qcTmpQC);
            }
        }
    }
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::UpdateDisplay(
    )
{
    //Updates the receive text buffer, faster
    if (this->verticalScrollBar()->isSliderDown() != true && mbContextMenuOpen == false)
    {
        //Variables for text selection storage
        unsigned int uiAnchor = 0;
        unsigned int uiPosition = 0;
        QTextCursor tcTmpCur;
        bool bShiftStart = false;
        bool bShiftEnd = false;
        unsigned int uiCurrentSize;
        if (this->textCursor().anchor() != this->textCursor().position())
        {
            //Text is selected
            uiAnchor = this->textCursor().anchor();
            uiPosition = this->textCursor().position();
            tcTmpCur = this->textCursor();
            if (uiAnchor > mbPrevTextSize)
            {
                //Start of selected text is in the output buffer
                bShiftStart = true;
                uiCurrentSize = this->toPlainText().size();
            }
            if (uiPosition > mbPrevTextSize)
            {
                //End of selected text is in the output buffer
                bShiftEnd = true;
                uiCurrentSize = this->toPlainText().size();
            }
        }

        //Slider not held down, update
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
        this->setPlainText(QString(mstrDatIn).append((mbLocalEcho == true && mbLineMode == true ? mstrDatOut : "")));
        this->setUpdatesEnabled(true);

        //Update previous text size variable
        mbPrevTextSize = mstrDatIn.size();

        if (muintCurPos == mstrDatOut.length()-1)
        {
            ++muintCurPos;
        }
        this->UpdateCursor();

        if (uiAnchor != 0 || uiPosition != 0)
        {
            //Reselect previous text
            if (bShiftStart == true)
            {
                //Adjust start position
                uiAnchor = uiAnchor + (this->toPlainText().size() - uiCurrentSize);
            }
            if (bShiftEnd == true)
            {
                //Adjust end position
                uiPosition = uiPosition + (this->toPlainText().size() - uiCurrentSize);
            }
            tcTmpCur.setPosition(uiAnchor);
            tcTmpCur.setPosition(uiPosition, QTextCursor::KeepAnchor);
            this->setTextCursor(tcTmpCur);
        }

        //Go back to previous position
        if (Pos == 65535)
        {
            //Bottom
            this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
            if (uiAnchor == 0 && uiPosition == 0)
            {
                this->moveCursor(QTextCursor::End);
            }
        }
        else
        {
            //Maintain
            this->verticalScrollBar()->setValue(Pos);
        }
    }
}

//=============================================================================
//=============================================================================
void
LrdScrollEdit::UpdateCursor(
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
LrdScrollEdit::SetSerialOpen(
    bool SerialOpen
    )
{
    //Updates the serial open variable
    mbSerialOpen = SerialOpen;
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
