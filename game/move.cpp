/*****************************************************************************
 * Eliot
 * Copyright (C) 2007 Olivier Teulière
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

#include <algorithm>
#include <wctype.h>
#include <sstream>

#include "move.h"


Move::Move(const Round &iRound)
    : m_score(0), m_round(iRound)
{
    m_type = VALID_ROUND;
    m_score = m_round.getPoints();
}


Move::Move(const wstring &iWord, const wstring &iCoord)
    : m_word(iWord), m_coord(iCoord)
{
    m_type = INVALID_WORD;
    m_score = 0;
}


Move::Move(const wstring &iLetters)
    : m_score(0), m_letters(iLetters)
{
    // Make the letters uppercase
    std::transform(m_letters.begin(), m_letters.end(),
                   m_letters.begin(), towupper);

    if (m_letters.empty())
        m_type = PASS;
    else
        m_type = CHANGE_LETTERS;
}


const Round & Move::getRound() const
{
    if (m_type != VALID_ROUND)
    {
        // FIXME: throw an exception object instead
        throw 1;
    }
    return m_round;
}


const wstring & Move::getBadWord() const
{
    if (m_type != INVALID_WORD)
    {
        // FIXME: throw an exception object instead
        throw 1;
    }
    return m_word;
}


const wstring & Move::getBadCoord() const
{
    if (m_type != INVALID_WORD)
    {
        // FIXME: throw an exception object instead
        throw 1;
    }
    return m_coord;
}


const wstring & Move::getChangedLetters() const
{
    if (m_type != CHANGE_LETTERS &&
        m_type != PASS)
    {
        // FIXME: throw an exception object instead
        throw 1;
    }
    return m_letters;
}


wstring Move::toString() const
{
    wstringstream wss;
    if (m_type == PASS)
        wss << "PASS";
    else if (m_type == CHANGE_LETTERS)
        wss << "CHANGE=" << m_letters;
    else if (m_type == INVALID_WORD)
        wss << "INVALID: word=" << m_round.toString() << "  coords=" << m_coord;
    else if (m_type == VALID_ROUND)
        wss << "VALID: word=" << m_round.toString();
    wss << "  score=" << m_score;
    return wss.str();
}

