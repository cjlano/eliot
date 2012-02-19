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

        const vector<TurnCmd *> & getTurns() const;

        /**
         * Print the contents of the commands history, to ease debugging
         */
        void print() const;

    private:
        vector<TurnCmd *> m_turnCommands;
        unsigned int m_currTurn;
};

#endif

