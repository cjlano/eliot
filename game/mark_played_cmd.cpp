/*****************************************************************************
 * Eliot
 * Copyright (C) 2009 Olivier Teulière
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
#include "mark_played_cmd.h"
#include "duplicate.h"

using namespace std;


INIT_LOGGER(game, MarkPlayedCmd);


MarkPlayedCmd::MarkPlayedCmd(Duplicate &ioDuplicate,
                             unsigned int iPlayerId,
                             bool iPlayedFlag)
    : m_duplicateGame(ioDuplicate), m_playerId(iPlayerId),
      m_newPlayedFlag(iPlayedFlag)
{
}


void MarkPlayedCmd::doExecute()
{
    m_oldPlayedFlag = m_duplicateGame.hasPlayed(m_playerId);
    m_duplicateGame.setPlayedFlag(m_playerId, m_newPlayedFlag);
}


void MarkPlayedCmd::doUndo()
{
    m_duplicateGame.setPlayedFlag(m_playerId, m_oldPlayedFlag);
}


wstring MarkPlayedCmd::toString() const
{
    wostringstream oss;
    oss << L"MarkPlayedCmd (player " << m_playerId
        << L" marked " << m_newPlayedFlag << L")";
    return oss.str();
}

