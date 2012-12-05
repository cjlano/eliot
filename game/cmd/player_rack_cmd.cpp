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

#include "cmd/player_rack_cmd.h"
#include "player.h"


INIT_LOGGER(game, PlayerRackCmd);


PlayerRackCmd::PlayerRackCmd(Player &ioPlayer, const PlayedRack &iNewRack)
    : m_player(ioPlayer), m_newRack(iNewRack)
{
}


void PlayerRackCmd::doExecute()
{
    // Get what was the rack for the current turn
    m_oldRack = m_player.getCurrentRack();
    // Update the rack of the player
    m_player.setCurrentRack(m_newRack);
}


void PlayerRackCmd::doUndo()
{
    // Restore the rack of the player
    m_player.setCurrentRack(m_oldRack);
}


wstring PlayerRackCmd::toString() const
{
    wostringstream oss;
    oss << L"PlayerRackCmd (player " << m_player.getId() << L"): "
        << m_newRack.toString();
    return oss.str();
}

