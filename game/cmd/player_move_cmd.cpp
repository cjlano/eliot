/*******************************************************************
 * Eliot
 * Copyright (C) 2008-2012 Olivier Teulière
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

#include "cmd/player_move_cmd.h"
#include "player.h"


INIT_LOGGER(game, PlayerMoveCmd);


PlayerMoveCmd::PlayerMoveCmd(Player &ioPlayer, const Move &iMove, bool iAutoExec)
    : m_player(ioPlayer), m_move(iMove)
{
    setAutoExecutable(iAutoExec || iMove.isNull());
    setHumanIndependent(!ioPlayer.isHuman());
}


void PlayerMoveCmd::doExecute()
{
    // Get what was the rack for the current turn
    m_originalRack = m_player.getCurrentRack();

    // Compute the new rack
    const PlayedRack &newRack = Move::ComputeRackForMove(m_originalRack, m_move);

    // Update the history and rack of the player
    m_player.accessHistory().playMove(m_move, newRack);
}


void PlayerMoveCmd::doUndo()
{
    // Remove the last history item
    m_player.accessHistory().removeLastTurn();
    // TODO: restore rack?
}


wstring PlayerMoveCmd::toString() const
{
    wostringstream oss;
    oss << L"PlayerMoveCmd (player " << m_player.getId() << L"): "
        << m_move.toString();
    return oss.str();
}

