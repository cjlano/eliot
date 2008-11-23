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

#ifndef _COMMAND_H
#define _COMMAND_H


/**
 * This abstract class is the parent of all classes implementing the Command
 * design pattern.
 */
class Command
{
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

    protected:
        virtual void doExecute() = 0;
        virtual void doUndo() = 0;

    private:
        bool m_executed;
};

#endif

