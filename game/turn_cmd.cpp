/*******************************************************************
 * Eliot
 * Copyright (C) 2008 Olivier Teulière
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

#include <boost/foreach.hpp>

#include "turn_cmd.h"
#include "player.h"


TurnCmd::~TurnCmd()
{
    BOOST_FOREACH(Command *cmd, m_commands)
    {
        delete cmd;
    }
}


void TurnCmd::addAndExecute(Command *iCmd)
{
    m_commands.push_back(iCmd);
    iCmd->execute();
}


void TurnCmd::doExecute()
{
    BOOST_FOREACH(Command *cmd, m_commands)
    {
        if (!cmd->isExecuted())
            cmd->execute();
    }
}


void TurnCmd::doUndo()
{
    // Undo commands in the reverse order of execution
    vector<Command*>::reverse_iterator it;
    for (it = m_commands.rbegin(); it != m_commands.rend(); ++it)
    {
        (*it)->undo();
    }
}

