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
#include <sstream>

#include "turn_cmd.h"
#include "command.h"
#include "player.h"
#include "debug.h"


INIT_LOGGER(game, TurnCmd);


TurnCmd::TurnCmd()
    : m_firstNotExecuted(0)
{
}


TurnCmd::~TurnCmd()
{
    BOOST_FOREACH(Command *cmd, m_commands)
    {
        delete cmd;
    }
}


void TurnCmd::addAndExecute(Command *iCmd)
{
    ASSERT(isFullyExecuted(), "Adding a command to a partially executed turn");
    m_commands.push_back(iCmd);
    iCmd->execute();
    ++m_firstNotExecuted;
}


void TurnCmd::execute()
{
    for (unsigned i = m_firstNotExecuted; i < m_commands.size(); ++i)
    {
        Command *cmd = m_commands[i];
        ASSERT(!cmd->isExecuted(), "Bug with m_firstNotExecuted");
        cmd->execute();
    }
    m_firstNotExecuted = m_commands.size();
}


void TurnCmd::undo()
{
    // Undo commands in the reverse order of execution
    unsigned firstToUndo = m_firstNotExecuted - 1;
    for (unsigned i = 0; i < m_firstNotExecuted; ++i)
    {
        Command *cmd = m_commands[firstToUndo - i];
        ASSERT(cmd->isExecuted(), "Bug with m_firstNotExecuted");
        cmd->undo();
    }
    m_firstNotExecuted = 0;
}


void TurnCmd::partialExecute()
{
    for (unsigned i = m_firstNotExecuted; i < m_commands.size(); ++i)
    {
        Command *cmd = m_commands[i];
        if (!cmd->isAutoExecutable())
            break;
        ASSERT(!cmd->isExecuted(), "Bug with m_firstNotExecuted");
        cmd->execute();
        ++m_firstNotExecuted;
    }
    ASSERT(isPartiallyExecuted(), "Bug in partialExecute()");
}


void TurnCmd::partialUndo()
{
    // Lazy implementation :)
    undo();
    partialExecute();
}


void TurnCmd::dropNonExecutedCommands()
{
    while (m_commands.size() > m_firstNotExecuted)
    {
        delete m_commands.back();
        m_commands.pop_back();
    }
}


bool TurnCmd::isFullyExecuted() const
{
    return m_firstNotExecuted == m_commands.size();
}


bool TurnCmd::isPartiallyExecuted() const
{
    if (isFullyExecuted())
        return true;
    return !m_commands[m_firstNotExecuted]->isAutoExecutable();
}


bool TurnCmd::isNotAtAllExecuted() const
{
    return m_firstNotExecuted == 0;
}


bool TurnCmd::hasNonAutoExecCmd() const
{
    BOOST_FOREACH(Command *cmd, m_commands)
    {
        if (!cmd->isAutoExecutable())
            return true;
    }
    return false;
}


bool TurnCmd::isHumanIndependent() const
{
    BOOST_FOREACH(Command *cmd, m_commands)
    {
        if (!cmd->isHumanIndependent())
            return false;
    }
    return true;
}


wstring TurnCmd::toString() const
{
    wostringstream oss;
    oss << L"TurnCmd:";
    BOOST_FOREACH(Command *cmd, m_commands)
    {
        oss << endl << L"  " << (cmd->isExecuted() ? "> " : "  " )
            << (cmd->isAutoExecutable() ? "* " : "  ") << cmd->toString();
    }
    return oss.str();
}

