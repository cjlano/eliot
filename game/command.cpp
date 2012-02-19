/*****************************************************************************
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

#include "command.h"
#include "debug.h"


INIT_LOGGER(game, Command);


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

