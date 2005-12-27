/*****************************************************************************
 * Copyright (C) 2004-2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
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

#include "debug.h"


Player::Player(int iId)
{
    m_id = iId;
    m_score = 0;
}


Player::~Player()
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


const Round & Player::getLastRound() const
{
    return m_history.getPreviousTurn().getRound();
}


void Player::endTurn(const Round &iRound, int iTurn)
{
    m_history.playRound(m_id,iTurn,iRound);
}

void Player::removeLastTurn()
{
    m_history.removeLastTurn();
}

const string Player::toString() const
{
    char buff[20];
    string res;

    sprintf(buff,"Player %d\n",m_id);
    res = string(buff);
    res += m_history.toString();
    res += "\n";
    sprintf(buff,"score %d\n",m_score);
    res += string(buff);
    return res;
}

/****************************************************************/
/****************************************************************/

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
