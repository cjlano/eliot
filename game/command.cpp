/*****************************************************************************
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

#include "command.h"
#include "debug.h"


INIT_LOGGER(game, Command);
INIT_LOGGER(game, UndoCmd);


Command::Command()
    : m_executed(false), m_humanIndependent(true), m_autoExecutable(true)
{
}


void Command::execute()
{
    ASSERT(!m_executed, "Command already executed!");
    doExecute();
    m_executed = true;
}


void Command::undo()
{
    ASSERT(m_executed, "Command already undone!");
    doUndo();
    m_executed = false;
}



UndoCmd::UndoCmd(Command *cmd)
    : m_cmd(cmd)
{
    ASSERT(m_cmd != 0, "Null command given");
}


UndoCmd::~UndoCmd()
{
    delete m_cmd;
}


bool UndoCmd::isAutoExecutable() const
{
    return m_cmd->isAutoExecutable();
}


void UndoCmd::doExecute()
{
    ASSERT(m_cmd->isExecuted(), "The wrapped command is not executed");
    m_cmd->undo();
}


void UndoCmd::doUndo()
{
    ASSERT(!m_cmd->isExecuted(), "The wrapped command is already executed");
    m_cmd->execute();
}


wstring UndoCmd::toString() const
{
    return L"UndoCmd (" + m_cmd->toString() + L")";
}

