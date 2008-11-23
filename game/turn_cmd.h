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

#ifndef _TURN_CMD_H
#define _TURN_CMD_H

#include <vector>

#include "command.h"

using namespace std;


/**
 * This class implements both the Command and Composite design patterns.
 * It encapsulates commands, while still behaving like one.
 */
class TurnCmd: public Command
{
    public:
        TurnCmd();
        virtual ~TurnCmd();

        /**
         * Add the given command and execute it.
         * The TurnCmd object takes ownership of the given Command.
         */
        void addAndExecute(Command *iCmd);

        bool isEmpty() const { return m_commands.empty(); }

    protected:
        virtual void doExecute();
        virtual void doUndo();

    private:
        vector<Command *> m_commands;
};

#endif

