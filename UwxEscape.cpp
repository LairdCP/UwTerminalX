/******************************************************************************
** Copyright (C) 2016 Laird
**
** Project: UwTerminalX
**
** Module: UwxEscape.h
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
#include "UwxEscape.h"

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
//Set pattern to match
QRegularExpression UwxEscape::reESeq = QRegularExpression("[\\\\]([0-9A-Fa-f]{2})");

//=============================================================================
//=============================================================================
void
UwxEscape::EscapeCharacters(
    QString *strData
    )
{
    //Escapes character sequences
    QRegularExpressionMatchIterator remiESeqMatch = UwxEscape::reESeq.globalMatch(strData);
    while (remiESeqMatch.hasNext())
    {
        //We have escape sequences
        QRegularExpressionMatch remThisESeqMatch = remiESeqMatch.next();
        strData->replace(QString("\\").append(remThisESeqMatch.captured(1)), QString((char)remThisESeqMatch.captured(1).toInt(NULL, 16)));
    }

    //Replace newline and tab characters
    strData->replace("\\r", "\r").replace("\\n", "\n").replace("\\t", "\t");
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
