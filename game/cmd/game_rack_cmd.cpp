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

#include "cmd/game_rack_cmd.h"
#include "game.h"


INIT_LOGGER(game, GameRackCmd);


GameRackCmd::GameRackCmd(Game &ioGame, const PlayedRack &iNewRack)
    : m_game(ioGame), m_newRack(iNewRack)
{
}


void GameRackCmd::doExecute()
{
    // Get what was the rack for the current turn
    m_oldRack = m_game.getHistory().getCurrentRack();
    // Update the game rack
    m_game.accessHistory().setCurrentRack(m_newRack);
}


void GameRackCmd::doUndo()
{
    // Restore the game rack
    m_game.accessHistory().setCurrentRack(m_oldRack);
}


wstring GameRackCmd::toString() const
{
    return L"GameRackCmd: " + m_newRack.toString();
}

