/*****************************************************************************
 * Eliot
 * Copyright (C) 2009-2012 Olivier Teulière
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

#include "cmd/master_move_cmd.h"
#include "duplicate.h"

using namespace std;


INIT_LOGGER(game, MasterMoveCmd);


MasterMoveCmd::MasterMoveCmd(Duplicate &ioDuplicate,
                             const Move &iMove)
    : m_duplicateGame(ioDuplicate), m_newMove(iMove)
{
    // In arbitration mode, no move is completely human independent
    // (in this case, the human is the arbitrator).
    // Since every turn must contain a MasterMoveCmd, this little trick
    // ensures that we have no human independent turn (without it, an
    // arbitration game with only computer players would be considered
    // as human independent, which could cause problems with the
    // Navigation::clearFuture() method (all the turns would be replayed
    // prior to deleting the turns, so no turn at all would be deleted).
    setHumanIndependent(!ioDuplicate.isArbitrationGame());
}


void MasterMoveCmd::doExecute()
{
    m_oldMove = m_duplicateGame.getMasterMove();
    m_duplicateGame.innerSetMasterMove(m_newMove);
}


void MasterMoveCmd::doUndo()
{
    m_duplicateGame.innerSetMasterMove(m_oldMove);
}


wstring MasterMoveCmd::toString() const
{
    wostringstream oss;
    oss << L"MasterMoveCmd (move " << m_newMove.toString() << L")";
    return oss.str();
}

