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

#include "debug.h"


Player::Player(int iId)
{
    m_id = iId;
    m_score = 0;
}


Player::~Player()
{
    for (unsigned int i = 0; i < m_history.size(); i++)
        delete m_history[i];
}


const PlayedRack & Player::getCurrentRack() const
{
    return m_pldrack;
}


void Player::setCurrentRack(const PlayedRack &iPld)
{
    m_pldrack = iPld;
}


const PlayedRack & Player::getLastRack() const
{
    return m_history.back()->getPlayedRack();
}


const Round & Player::getLastRound() const
{
    return m_history.back()->getRound();
}


/*
 * This function increments the number of racks, and fills the new rack
 * with the unplayed tiles from the previous one.
 * 03 sept 2000 : We have to sort the tiles according to the new rules
 */
void Player::endTurn(const Round &iRound, int iTurn)
{
    m_history.push_back(new Turn(iTurn, m_id, m_pldrack, iRound));

    Rack rack;
    m_pldrack.getRack(rack);

    // We remove the played tiles from the rack
    for (int i = 0; i < iRound.getWordLen(); i++)
    {
        if (iRound.isPlayedFromRack(i))
        {
            if (iRound.isJoker(i))
                rack.remove(Tile::Joker());
            else
                rack.remove(iRound.getTile(i));
        }
    }

    // Now reinitialize the current rack with the remaining tiles
    m_pldrack = PlayedRack();
    m_pldrack.setOld(rack);
}

