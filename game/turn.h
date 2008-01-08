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

/**
 *  \file   turn.h
 *  \brief  Game turn (= id + pldrack + move)
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#ifndef _TURN_H
#define _TURN_H

#include <string>
#include "pldrack.h"
#include "move.h"

using std::wstring;


/**
 * A Turn is the information about one 'move' done by a player.
 * It consists of the player who played, the rack, and the actual move.
 * A turn also has an id (XXX: currently never read)
 *
 * This class has no logic, it is merely there to aggregate corresponding
 * data.
 */
class Turn
{
public:
    Turn();
    Turn(unsigned int iNum, unsigned int iPlayerId,
         const PlayedRack& iPldRack, const Move& iMove);

    void setNum(unsigned int iNum)                 { m_num = iNum; }
    void setPlayer(unsigned int iPlayerId)         { m_playerId = iPlayerId; }
    void setPlayedRack(const PlayedRack& iPldRack) { m_pldrack = iPldRack; }
    void setMove(const Move& iMove)             { m_move = iMove; }

    unsigned int      getNum()        const { return m_num; }
    unsigned int      getPlayer()     const { return m_playerId; }
    const PlayedRack& getPlayedRack() const { return m_pldrack; }
    const Move&       getMove()       const { return m_move; }

    wstring toString(bool iShowExtraSigns = false) const;

private:
    unsigned int m_num;
    unsigned int m_playerId;
    PlayedRack   m_pldrack;
    Move         m_move;
};

#endif

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
