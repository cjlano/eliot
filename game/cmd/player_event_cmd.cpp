/*******************************************************************
 * Eliot
 * Copyright (C) 2012 Olivier Teulière
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

#include <sstream>

#include "cmd/player_event_cmd.h"
#include "player.h"
#include "debug.h"


INIT_LOGGER(game, PlayerEventCmd);


PlayerEventCmd::PlayerEventCmd(Player &ioPlayer, EventType iEvent, int iPoints)
    : m_player(ioPlayer), m_eventType(iEvent), m_points(iPoints)
{
    // Solos and warnings are always positive
    ASSERT(iEvent != SOLO || iPoints >= 0, "Negative points not allowed");
    ASSERT(iEvent != WARNING || iPoints >= 0, "Negative points not allowed");
    // Penalties are negative in arbitration mode, but positive in topping mode
    // End game points are positive for one player and negative for all the other players
}


void PlayerEventCmd::doExecute()
{
    if (m_eventType == WARNING)
        m_player.accessHistory().addWarning();
    else if (m_eventType == PENALTY)
        m_player.accessHistory().addPenaltyPoints(m_points);
    else if (m_eventType == SOLO)
        m_player.accessHistory().addSoloPoints(m_points);
    else if (m_eventType == END_GAME)
        m_player.accessHistory().addEndGamePoints(m_points);
    else
    {
        ASSERT(false, "Missing case");
    }
}


void PlayerEventCmd::doUndo()
{
    if (m_eventType == WARNING)
        m_player.accessHistory().removeWarning();
    else if (m_eventType == PENALTY)
        m_player.accessHistory().addPenaltyPoints(- m_points);
    else if (m_eventType == SOLO)
        m_player.accessHistory().addSoloPoints(- m_points);
    else if (m_eventType == END_GAME)
        m_player.accessHistory().addEndGamePoints(- m_points);
    else
    {
        ASSERT(false, "Missing case");
    }
}


wstring PlayerEventCmd::toString() const
{
    wostringstream oss;
    oss << L"PlayerEventCmd (player " << m_player.getId() << L"): ";
    if (m_eventType == WARNING)
        oss << L"Warning";
    else if (m_eventType == PENALTY)
        oss << L"Penalty (" << m_points << L" points)";
    else if (m_eventType == SOLO)
        oss << L"Solo (" << m_points << L" points)";
    else if (m_eventType == END_GAME)
        oss << L"EndGame (" << m_points << L" points)";
    else
    {
        ASSERT(false, "Missing case");
    }
    return oss.str();
}

