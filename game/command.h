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

#ifndef COMMAND_H_
#define COMMAND_H_

#include <string>

#include "logging.h"

using std::wstring;


/**
 * This abstract class is the parent of all classes implementing the Command
 * design pattern.
 */
class Command
{
    DEFINE_LOGGER();

    public:
        Command();
        virtual ~Command() {}

        /**
         * Execute the command. This can later be undone using the undo()
         * method.
         *
         * You are not allowed to call this method twice before calling
         * undo() first.
         */
        void execute();

        /**
         * Undo the previous call to execute().
         *
         * You are not allowed to undo a command which is not executed yet.
         */
        void undo();

        /**
         * Return true if the command has been executed (i.e. if it is
         * allowed to call undo()), false otherwise.
         */
        bool isExecuted() const { return m_executed; }

        /**
         * Mark the command as human independent, which means that it will
         * be automatically executed if the commands history is cleared
         * just before this command.
         * It is mostly useful in FreeGame mode, where a turn played by an AI
         * player should be re-executed, because the user cannot control it.
         */
        void setHumanIndependent(bool humanIndependent) { m_humanIndependent = humanIndependent; }
        /// Return true if the command is human independent
        virtual bool isHumanIndependent() const { return m_humanIndependent; }

        /**
         * Description of the command, for debugging purposes
         */
        virtual wstring toString() const = 0;

    protected:
        virtual void doExecute() = 0;
        virtual void doUndo() = 0;

    private:
        bool m_executed;
        bool m_humanIndependent;
};

#endif

