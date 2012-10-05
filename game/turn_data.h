/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2007 Antoine Fraboulet & Olivier Teulière
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

#ifndef TURN_DATA_H_
#define TURN_DATA_H_

#include <string>
#include "pldrack.h"
#include "move.h"
#include "logging.h"

using std::wstring;


/**
 * A TurnData is the information about one 'move' done by a player.
 * It consists of the player who played, the rack, and the actual move.
 *
 * This class has no logic, it is merely there to aggregate corresponding
 * data.
 */
class TurnData
{
    DEFINE_LOGGER();
public:
    TurnData();
    TurnData(unsigned int iPlayerId,
             const PlayedRack& iPldRack, const Move& iMove);

    void setPlayer(unsigned int iPlayerId)         { m_playerId = iPlayerId; }
    void setPlayedRack(const PlayedRack& iPldRack) { m_pldrack = iPldRack; }
    void setMove(const Move& iMove)             { m_move = iMove; }
    // Setters for events (warnings, penalties, solos, end game primes)
    void addWarning(unsigned iNb = 1) { m_warningsNb += iNb; }
    void addPenaltyPoints(int iPoints) { m_penaltyPoints += iPoints; }
    void addSoloPoints(int iPoints) { m_soloPoints += iPoints; }
    void addEndGamePoints(int iPoints) { m_endGamePoints += iPoints; }

    unsigned int      getPlayer()     const { return m_playerId; }
    const PlayedRack& getPlayedRack() const { return m_pldrack; }
    const Move&       getMove()       const { return m_move; }
    // Getters for events (warnings, penalties, solos, end game primes)
    unsigned getWarningsNb() const { return m_warningsNb; }
    int getPenaltyPoints() const { return m_penaltyPoints; }
    int getSoloPoints() const { return m_soloPoints; }
    int getEndGamePoints() const { return m_endGamePoints; }

    wstring toString() const;

private:
    unsigned int m_playerId;
    PlayedRack m_pldrack;
    Move m_move;
    int m_warningsNb;
    int m_penaltyPoints;
    int m_soloPoints;
    int m_endGamePoints;
};

#endif

