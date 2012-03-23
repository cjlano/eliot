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
#include <typeinfo>

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


void TurnCmd::dropFrom(const Command &iCmd)
{
    // Find the command index
    unsigned idx = findIndex(iCmd);
    ASSERT(idx != m_commands.size(), "Cannot find command to drop");

    LOG_DEBUG("Deleting last turn commands, starting from " << idx);

    while (m_commands.size() > idx)
    {
        if (m_commands.back()->isExecuted())
            m_commands.back()->undo();
        delete m_commands.back();
        m_commands.pop_back();
    }
    if (m_firstNotExecuted > m_commands.size())
        m_firstNotExecuted = m_commands.size();
}


void TurnCmd::dropCommand(const Command &iCmd)
{
    ASSERT(iCmd.isInsertable(), "Only insertable commands can be dropped");
    ASSERT(iCmd.isAutoExecutable(), "Non auto-executable commands cannot be dropped");

    // Find the command index
    unsigned idx = findIndex(iCmd);
    ASSERT(idx != m_commands.size(), "Cannot find command to drop");

    LOG_DEBUG("Dropping single command");

    // Save the execution status
    unsigned tmpIdx = m_firstNotExecuted;

    // Undo commands after the interesting one (included)
    while (tmpIdx > idx)
    {
        --tmpIdx;
        m_commands[tmpIdx]->undo();
    }
    ASSERT(!iCmd.isExecuted(), "Logic error");

    // Drop the command
    delete m_commands[idx];
    m_commands.erase(m_commands.begin() + idx);

    // We have deleted one command, so the index should be decreased
    --m_firstNotExecuted;

    // Re-execute the commands
    while (tmpIdx != m_firstNotExecuted)
    {
        m_commands[tmpIdx]->execute();
        ++tmpIdx;
    }
}


void TurnCmd::insertCommand(Command *iCmd)
{
    ASSERT(iCmd->isInsertable(), "Only insertable commands can be inserted");
    ASSERT(iCmd->isAutoExecutable(), "Non auto-executable commands cannot be inserted");

    // Find the insertion index
    unsigned idx = findIndexFirstNaec();

    LOG_DEBUG("Inserting command");

    // Save the execution status
    unsigned tmpIdx = m_firstNotExecuted;

    // Undo commands after the interesting one (included)
    while (tmpIdx > idx)
    {
        --tmpIdx;
        m_commands[tmpIdx]->undo();
    }

    // Insert the command (possibly at the end, if there is no NAEC)
    if (idx == m_commands.size())
        m_commands.push_back(iCmd);
    else
        m_commands.insert(m_commands.begin() + idx, iCmd);

    // We have inserted one command, so the index should be increased
    ++m_firstNotExecuted;

    // Re-execute the commands
    while (tmpIdx != m_firstNotExecuted)
    {
        m_commands[tmpIdx]->execute();
        ++tmpIdx;
    }
}


void TurnCmd::replaceCommand(const Command &iOldCmd,
                             Command *iNewCmd)
{
    ASSERT(string(typeid(iOldCmd).name()) == string(typeid(*iNewCmd).name()),
           "The commands should be of the same type (" +
           string(typeid(iOldCmd).name()) + " vs. " +
           string(typeid(*iNewCmd).name()));

    unsigned idx = findIndex(iOldCmd);
    ASSERT(idx != m_commands.size(), "Cannot find command");

    // Save the execution status
    unsigned tmpIdx = m_firstNotExecuted;

    // Undo commands after the interesting one (included)
    while (tmpIdx > idx)
    {
        --tmpIdx;
        m_commands[tmpIdx]->undo();
    }
    ASSERT(!iOldCmd.isExecuted(), "Logic error");

    // Replace the command
    delete m_commands[idx];
    m_commands[idx] = iNewCmd;

    // Re-execute the commands
    while (tmpIdx != m_firstNotExecuted)
    {
        m_commands[tmpIdx]->execute();
        ++tmpIdx;
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
    return findIndexFirstNaec() != m_commands.size();
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


unsigned TurnCmd::findIndex(const Command &iCmd) const
{
    for (unsigned i = 0; i < m_commands.size(); ++i)
    {
        if (m_commands[i] == &iCmd)
            return i;
    }
    return m_commands.size();
}


unsigned TurnCmd::findIndexFirstNaec() const
{
    for (unsigned i = 0; i < m_commands.size(); ++i)
    {
        if (!m_commands[i]->isAutoExecutable())
            return i;
    }
    return m_commands.size();
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

