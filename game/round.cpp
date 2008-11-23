/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2007 Antoine Fraboulet & Olivier Teulière
 * Authors: Antoine Fraboulet <antoine.fraboulet @@ free.fr>
 *          Olivier Teulière <ipkiss @@ gmail.com>
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

#include <string>
#include <sstream>
#include <wctype.h>
#include "tile.h"
#include "round.h"
#include "encoding.h"


#define FROMBOARD 0x1
#define FROMRACK  0x2
#define JOKER     0x4

#define __UNUSED__ __attribute__((unused))


Round::Round()
{
    init();
}


void Round::init()
{
    m_word.clear();
    m_tileOrigin.clear();
    m_coord.setRow(1);
    m_coord.setCol(1);
    m_coord.setDir(Coord::HORIZONTAL);
    m_points = 0;
    m_bonus  = 0;
}


void Round::setWord(const vector<Tile> &iTiles)
{
    m_word = iTiles;
    // XXX: always from rack?
    m_tileOrigin = vector<char>(iTiles.size(), FROMRACK);
}


void Round::setFromRack(unsigned int iIndex)
{
    m_tileOrigin[iIndex] &= ~FROMBOARD;
    m_tileOrigin[iIndex] |= FROMRACK;
}


void Round::setFromBoard(unsigned int iIndex)
{
    m_tileOrigin[iIndex] &= ~FROMRACK;
    m_tileOrigin[iIndex] |= FROMBOARD;
}


void Round::setJoker(unsigned int iIndex, bool value)
{
    if (value)
        m_tileOrigin[iIndex] |= JOKER;
    else
        m_tileOrigin[iIndex] &= ~JOKER;
}


bool Round::isJoker(unsigned int iIndex) const
{
     return m_tileOrigin[iIndex] & JOKER;
}


const Tile& Round::getTile(unsigned int iIndex) const
{
     return m_word[iIndex];
}


bool Round::isPlayedFromRack(unsigned int iIndex) const
{
     return m_tileOrigin[iIndex] & FROMRACK;
}


void Round::addRightFromBoard(Tile c)
{
    m_word.push_back(c);
    m_tileOrigin.push_back(FROMBOARD);
}


void Round::removeRightToBoard(Tile __UNUSED__ c)
{
    // c is unused.
    m_word.pop_back();
    m_tileOrigin.pop_back();
}


void Round::addRightFromRack(Tile c, bool iJoker)
{
    m_word.push_back(c);
    char origin = FROMRACK;
    if (iJoker)
    {
        origin |= JOKER;
    }
    m_tileOrigin.push_back(origin);
}


void Round::removeRightToRack(Tile __UNUSED__ c, bool __UNUSED__ iJoker)
{
    // c is unused.
    m_word.pop_back();
    m_tileOrigin.pop_back();
}

wstring Round::getWord() const
{
    wchar_t c;
    wstring s;

    for (unsigned int i = 0; i < getWordLen(); i++)
    {
        c = getTile(i).toChar();
        if (isJoker(i))
            c = towlower(c);
        s += c;
    }
    return s;
}

wstring Round::toString() const
{
    wostringstream oss;
    if (!getWord().empty())
    {
        oss << getWord() << L' ' << (getBonus() ? L'*' : L' ')
            << L' ' << getPoints() << L' ' << getCoord().toString();
    }
    return oss.str();
}

