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

#ifndef TURN_CMD_H_
#define TURN_CMD_H_

#include <vector>

#include "logging.h"

using namespace std;

class Command;


/**
 * This class encapsulates commands, and almost behaves as one itself.
 * The main difference with a normal command (and thus with the composite pattern)
 * is that a TurnCmd provides partial execution: some of the commands it contains
 * (the first ones) can be executed, whereas others not.
 * The ones which can be executed are the ones flagged "auto-executable".
 */
class TurnCmd
{
    DEFINE_LOGGER();

    public:
        TurnCmd();
        virtual ~TurnCmd();

        /**
         * Add the given command and execute it.
         * The TurnCmd object takes ownership of the given Command.
         */
        void addAndExecute(Command *iCmd);

        bool isEmpty() const { return m_commands.empty(); }

        const vector<Command *> & getCommands() const { return m_commands; }

        virtual bool isHumanIndependent() const;

        virtual wstring toString() const;

        void execute();
        void undo();

    private:
        vector<Command *> m_commands;
};

#endif

