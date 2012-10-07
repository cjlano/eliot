/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2012 Antoine Fraboulet & Olivier Teulière
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
#include <algorithm> // For std::transform
#include <wctype.h>
#include "tile.h"
#include "round.h"
#include "encoding.h"
#include "debug.h"


INIT_LOGGER(game, Round);


Round::Round()
    : m_coord(1, 1, Coord::HORIZONTAL), m_points(0), m_bonus(false)
{
}


void Round::setWord(const vector<Tile> &iTiles)
{
    m_word = iTiles;
    // XXX: always from rack?
    m_rackOrigin = vector<bool>(iTiles.size(), true);
}


void Round::setTile(unsigned int iIndex, const Tile &iTile)
{
    ASSERT(iIndex < m_word.size(), "Invalid index");
    m_word[iIndex] = iTile;
}


void Round::setFromRack(unsigned int iIndex)
{
    ASSERT(iIndex < m_word.size(), "Invalid index");
    m_rackOrigin[iIndex] = true;
}


void Round::setFromBoard(unsigned int iIndex)
{
    ASSERT(iIndex < m_word.size(), "Invalid index");
    m_rackOrigin[iIndex] = false;
}


bool Round::isJoker(unsigned int iIndex) const
{
    ASSERT(iIndex < m_word.size(), "Invalid index");
     return m_word[iIndex].isJoker();
}


const Tile& Round::getTile(unsigned int iIndex) const
{
    ASSERT(iIndex < m_word.size(), "Invalid index");
     return m_word[iIndex];
}


bool Round::isPlayedFromRack(unsigned int iIndex) const
{
    ASSERT(iIndex < m_word.size(), "Invalid index");
     return m_rackOrigin[iIndex];
}


void Round::addRightFromBoard(const Tile &iTile)
{
    // The call to toUpper() is necessary to avoid that a joker
    // on the board appears as a joker in the Round
    m_word.push_back(iTile.toUpper());
    m_rackOrigin.push_back(false);
}


void Round::addRightFromRack(const Tile &iTile, bool iJoker)
{
    if (iJoker)
        m_word.push_back(iTile.toLower());
    else
        m_word.push_back(iTile);
    m_rackOrigin.push_back(true);
}


void Round::removeRight()
{
    ASSERT(!m_word.empty() && !m_rackOrigin.empty(),
           "Trying to remove tiles that were never added");
    m_word.pop_back();
    m_rackOrigin.pop_back();
}


wstring Round::getWord() const
{
    wstring s;

    for (unsigned int i = 0; i < getWordLen(); i++)
    {
        wstring chr = getTile(i).getDisplayStr();
        if (isJoker(i))
            std::transform(chr.begin(), chr.end(), chr.begin(), towlower);
        s += chr;
    }
    return s;
}


unsigned Round::countJokersFromRack() const
{
    unsigned count = 0;
    for (unsigned int i = 0; i < getWordLen(); i++)
    {
        if (isJoker(i) && isPlayedFromRack(i))
            ++count;
    }
    return count;
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

