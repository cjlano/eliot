/*****************************************************************************
 * Eliot
 * Copyright (C) 2004-2007 Olivier Teulière & Antoine Fraboulet
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
 *          Antoine Fraboulet <antoine.fraboulet @@ free.fr>
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

#include "tile.h"
#include "rack.h"
#include "pldrack.h"
#include "round.h"
#include "results.h"
#include "board.h"
#include "player.h"
#include "turn.h"
#include "history.h"
#include "encoding.h"

#include "debug.h"


Player::Player(unsigned int iId)
    : m_id(iId), m_score(0)
{
}


const PlayedRack & Player::getCurrentRack() const
{
    return m_history.getCurrentRack();
}


void Player::setCurrentRack(const PlayedRack &iPld)
{
    m_history.setCurrentRack(iPld);
}


const PlayedRack & Player::getLastRack() const
{
    return m_history.getPreviousTurn().getPlayedRack();
}


const Move & Player::getLastMove() const
{
    return m_history.getPreviousTurn().getMove();
}


void Player::endTurn(const Move &iMove, unsigned int iTurn)
{
    addPoints(iMove.getScore());
    m_history.playMove(m_id, iTurn, iMove);
}

void Player::removeLastTurn()
{
    // Remove points of the last turn
    addPoints(- m_history.getPreviousTurn().getMove().getScore());
    m_history.removeLastTurn();
}

wstring Player::toString() const
{
    wstring res;

    wchar_t buff[6];
    _swprintf(buff, 5, L"Player %d\n", m_id);
    res = wstring(buff);
    res += m_history.toString() + L"\n";
    _swprintf(buff, 5, L"score %d\n", m_score);
    res += wstring(buff);
    return res;
}

/****************************************************************/
/****************************************************************/

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
