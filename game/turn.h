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

#ifndef TURN_H_
#define TURN_H_

#include <string>
#include "pldrack.h"
#include "move.h"

using std::wstring;


/**
 * A Turn is the information about one 'move' done by a player.
 * It consists of the player who played, the rack, and the actual move.
 *
 * This class has no logic, it is merely there to aggregate corresponding
 * data.
 */
class Turn
{
public:
    Turn();
    Turn(unsigned int iPlayerId,
         const PlayedRack& iPldRack, const Move& iMove);

    void setPlayer(unsigned int iPlayerId)         { m_playerId = iPlayerId; }
    void setPlayedRack(const PlayedRack& iPldRack) { m_pldrack = iPldRack; }
    void setMove(const Move& iMove)             { m_move = iMove; }

    unsigned int      getPlayer()     const { return m_playerId; }
    const PlayedRack& getPlayedRack() const { return m_pldrack; }
    const Move&       getMove()       const { return m_move; }

    wstring toString(bool iShowExtraSigns = false) const;

private:
    unsigned int m_playerId;
    PlayedRack   m_pldrack;
    Move         m_move;
};

#endif

