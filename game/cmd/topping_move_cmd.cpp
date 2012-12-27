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

#include "cmd/topping_move_cmd.h"
#include "player.h"


INIT_LOGGER(game, ToppingMoveCmd);


ToppingMoveCmd::ToppingMoveCmd(unsigned iPlayerId, const Move &iMove, int iElapsed)
    : m_playerId(iPlayerId), m_move(iMove), m_elapsed(iElapsed)
{
    setAutoExecutable(true);
    setHumanIndependent(false);
}


void ToppingMoveCmd::doExecute()
{
    // Nothing to do
}


void ToppingMoveCmd::doUndo()
{
    // Nothing to undo
}


wstring ToppingMoveCmd::toString() const
{
    wostringstream oss;
    oss << L"ToppingMoveCmd (player " << m_playerId << L"): "
        << m_move.toString() << "  elapsed=" << m_elapsed;
    return oss.str();
}

