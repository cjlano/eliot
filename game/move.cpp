/*****************************************************************************
 * Eliot
 * Copyright (C) 2007-2012 Olivier Teulière
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

#include <boost/foreach.hpp>

#include <wctype.h>
#include <sstream>

#include "move.h"
#include "rack.h"
#include "pldrack.h"
#include "encoding.h"
#include "debug.h"


INIT_LOGGER(game, Move);


Move::Move()
    :m_score(0)
{
    m_type = NO_MOVE;
}


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
    m_letters = toUpper(m_letters);

    if (m_letters.empty())
        m_type = PASS;
    else
        m_type = CHANGE_LETTERS;
}


const Round & Move::getRound() const
{
    ASSERT(m_type == VALID_ROUND, "Incorrect move type");
    return m_round;
}


const wstring & Move::getBadWord() const
{
    ASSERT(m_type == INVALID_WORD, "Incorrect move type");
    return m_word;
}


const wstring & Move::getBadCoord() const
{
    ASSERT(m_type == INVALID_WORD, "Incorrect move type");
    return m_coord;
}


const wstring & Move::getChangedLetters() const
{
    ASSERT(m_type == CHANGE_LETTERS || m_type == PASS, "Incorrect move type");
    return m_letters;
}


PlayedRack Move::ComputeRackForMove(const PlayedRack &iOldRack, const Move &iMove)
{
    // Start from the given rack
    // 03 sept 2000: We have to sort the tiles according to the new rules
    // Using a Rack object will indirectly do it for us
    Rack newRack = iOldRack.getRack();

    if (iMove.isValid())
    {
        // Remove the played tiles from the rack
        const Round &round = iMove.getRound();
        for (unsigned int i = 0; i < round.getWordLen(); i++)
        {
            if (round.isPlayedFromRack(i))
            {
                if (round.isJoker(i))
                    newRack.remove(Tile::Joker());
                else
                    newRack.remove(round.getTile(i));
            }
        }
    }
    else if (iMove.isChangeLetters())
    {
        // Remove the changed tiles from the rack
        const wstring & changed = iMove.getChangedLetters();
        BOOST_FOREACH(wchar_t ch, changed)
        {
            newRack.remove(Tile(ch));
        }
    }
    else if (iMove.isNull())
    {
        // Special case: when the player didn't play, we keep the
        // original played rack, to avoid the implicit reordering
        // of the "old" tiles, and to keep the nice '+' in the display.
        // This is only used in FreeGame mode at the moment (for the Moves
        // of the players who didn't played), and this rule is a bit arbitrary:
        // in this mode, the distinction between "old" and "new" tiles
        // is only useful to display the game history...
        return iOldRack;
    }

    PlayedRack newPldRack;
    newPldRack.setOld(newRack);
    return newPldRack;
}


wstring Move::toString() const
{
    wstringstream wss;
    if (m_type == NO_MOVE)
        wss << "NO_MOVE";
    else if (m_type == PASS)
        wss << "PASS";
    else if (m_type == CHANGE_LETTERS)
        wss << "CHANGE=" << m_letters;
    else if (m_type == INVALID_WORD)
        wss << "INVALID: word=" << m_word << "  coords=" << m_coord;
    else if (m_type == VALID_ROUND)
        wss << "VALID: word=" << m_round.toString();
    wss << "  score=" << m_score;
    return wss.str();
}


bool Move::operator==(const Move &iOther) const
{
    return m_type == iOther.m_type
        && m_score == iOther.m_score
        && m_word == iOther.m_word
        && m_coord == iOther.m_coord
        && m_letters == iOther.m_letters
        && m_round == iOther.m_round;
}


