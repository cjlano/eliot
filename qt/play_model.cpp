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

#include "play_model.h"


INIT_LOGGER(qt, PlayModel);


void PlayModel::clear()
{
    setCoord(Coord());
    setWord(L"");
}


void PlayModel::setCoord(const Coord &iCoord)
{
    // Avoid useless work
    if (iCoord == m_currCoord)
        return;

    m_prevCoord = m_currCoord;
    m_currCoord = iCoord;
    emit coordChanged(iCoord, m_prevCoord);
}


void PlayModel::setWord(const wstring &iWord)
{
    // Avoid useless work
    if (iWord == m_currWord)
        return;

    m_prevWord = m_currWord;
    m_currWord = iWord;
    emit wordChanged(iWord, m_prevWord);
}

