/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2009 Olivier Teulière
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

#include "arbitration.h"
#include "rack.h"
#include "player.h"
#include "settings.h"
#include "encoding.h"
#include "debug.h"


INIT_LOGGER(game, Arbitration);


Arbitration::Arbitration(const GameParams &iParams)
    : Duplicate(iParams), m_results(1000)
{
}


void Arbitration::search()
{
    // Search for the current player
    const Rack &rack = m_players[m_currPlayer]->getCurrentRack().getRack();
    LOG_DEBUG("Performing search for rack " + lfw(rack.toString()));
    // FIXME arbitration begin
    int limit = Settings::Instance().getInt("training.search-limit");
    // FIXME arbitration end
    m_results.setLimit(limit);
    m_results.search(getDic(), getBoard(), rack, getHistory().beforeFirstRound());
    LOG_DEBUG("Found " << m_results.size() << " results");
}


Move Arbitration::checkWord(const wstring &iWord,
                            const wstring &iCoords) const
{
    Round round;
    int res = checkPlayedWord(iCoords, iWord, round);
    if (res == 0)
        return Move(round);
    return Move(iWord, iCoords);
}


void Arbitration::assignMove(unsigned int iPlayerId, const Move &iMove)
{
    ASSERT(iPlayerId < getNPlayers(), "Wrong player number");
    // A move can only be assigned for the last turn
    ASSERT(accessNavigation().isLastTurn(), "This is not the last turn!");

    Player &player = *m_players[iPlayerId];
    if (hasPlayed(iPlayerId))
    {
        LOG_INFO("Re-assigning move for player " << iPlayerId);
        undoPlayerMove(player);
    }
    recordPlayerMove(iMove, player, true);
}


void Arbitration::finalizeTurn()
{
    m_results.clear();

    // FIXME arbitration begin
    tryEndTurn();
    // FIXME arbitration end
}

