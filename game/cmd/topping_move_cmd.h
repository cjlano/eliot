/*******************************************************************
 * Eliot
 * Copyright (C) 2012 Olivier Teulière
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

#ifndef TOPPING_MOVE_CMD_H_
#define TOPPING_MOVE_CMD_H_

#include "command.h"
#include "move.h"
#include "pldrack.h"
#include "logging.h"

class Player;
class Rack;


/**
 * This class implements the Command design pattern.
 * It encapsulates the logic to keep track of all the moves played by a player
 * in topping mode.
 */
class ToppingMoveCmd: public Command
{
    DEFINE_LOGGER();

    public:
        ToppingMoveCmd(unsigned iPlayerId, const Move &iMove, int iElapsed);

        virtual wstring toString() const;

        // Getters
        unsigned getPlayerId() const { return m_playerId; }
        const Move & getMove() const { return m_move; }
        int getElapsedTime() const { return m_elapsed; }

    protected:
        virtual void doExecute();
        virtual void doUndo();

    private:
        unsigned m_playerId;
        Move m_move;
        int m_elapsed;
};

#endif

