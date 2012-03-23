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

#ifndef NAVIGATION_H_
#define NAVIGATION_H_

#include <vector>
#include "logging.h"

class TurnCmd;
class Command;

using namespace std;


/**
 * Handle the navigation in the game history.
 *
 * A few assumptions are made for the design:
 *  - each turn in the game is represented by a TurnCmd object
 *  - the current TurnCmd object is always in the "partially executed" state
 *    (see class documentation), except maybe for the last turn
 *  - previous turns are fully executed, and future turns are never executed
 *  - addind a command can only be done if the current turn is the last one
 *    and is fully executed
 *
 * Many assertions are there to help enforce this design.
 */
class Navigation
{
    DEFINE_LOGGER();
    public:
        Navigation();
        ~Navigation();

        void newTurn();
        void addAndExecute(Command *iCmd);

        unsigned int getCurrTurn() const;
        unsigned int getNbTurns() const;
        bool isFirstTurn() const;
        bool isLastTurn() const;

        void firstTurn();
        void prevTurn();
        void nextTurn();
        void lastTurn();
        /**
         * Get rid of the future turns of the game, the current turn
         * becoming the last one.
         */
        void clearFuture();

        /**
         * Remove the commands of the last turn, starting from the given one.
         */
        void dropFrom(const Command &iCmd);

        /**
         * Remove the given command from the current turn
         * (only if it is an insertable and non auto-executable command)
         */
        void dropCommand(const Command &iCmd);

        /**
         * Insert the given command in the current turn, before the first NAEC
         * (only if it is an insertable and non auto-executable command)
         */
        void insertCommand(Command *iCmd);

        /**
         * Replace one existing command (for the current turn) with another one
         * (of the same type).
         */
        void replaceCommand(const Command &iOldCmd,
                            Command *iNewCmd);

        const vector<TurnCmd *> & getTurns() const;
        const TurnCmd & getCurrentTurn() const;

        /**
         * Print the contents of the commands history, to ease debugging
         */
        void print() const;

    private:
        vector<TurnCmd *> m_turnCommands;
        unsigned int m_currTurn;
};

#endif

