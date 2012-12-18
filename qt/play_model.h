/*****************************************************************************
 * Eliot
 * Copyright (C) 2009-2012 Olivier Teulière
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

#ifndef PLAY_MODEL_H_
#define PLAY_MODEL_H_

#include <string>
#include <QObject>

#include "coord.h"
#include "logging.h"

using std::wstring;


/**
 * Encapsulate a move being played (possibly incomplete).
 * There is usually only one instance of this class.
 *
 * A PlayModel contains the word typed by the user and the coordinates of the word.
 * Signals are emitted every time one of these values changes: this allows various
 * useful things, such as:
 *  - keeping the arrow on the board synchronized with the typed coordinates
 *  - keeping the word preview on the board synchronized with the typed letters,
 *    and with the remaining letters in the rack
 */
class PlayModel: public QObject
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    /**
     * Remove the word and the coordinates.
     * This may emit both the wordChanged() and the coordChanged() signals.
     */
    void clear();

    void setCoord(const Coord &iCoord);
    const Coord & getCoord() const { return m_currCoord; }

    void setWord(const wstring &iWord);
    const wstring & getWord() const { return m_currWord; }

signals:
    void coordChanged(const Coord &iNewCoord, const Coord &iOldCoord);
    void wordChanged(const wstring &iNewWord, const wstring &iOldWord);

private:
    Coord m_currCoord;
    Coord m_prevCoord;

    wstring m_currWord;
    wstring m_prevWord;
};

#endif

