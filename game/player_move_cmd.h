/*******************************************************************
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

#ifndef PLAYER_MOVE_CMD_H_
#define PLAYER_MOVE_CMD_H_

#include "command.h"
#include "move.h"
#include "pldrack.h"
#include "logging.h"

class Player;
class Rack;


/**
 * This class implements the Command design pattern.
 * It encapsulates the logic to update the player state when a player
 * plays a move:
 *  - score update
 *  - rack update
 *  - player history update
 *
 * The Move validation must have been done before calling execute().
 */
class PlayerMoveCmd: public Command
{
    DEFINE_LOGGER();

    public:
        PlayerMoveCmd(Player &ioPlayer, const Move &iMove,
                      bool iAutoExec = false);

        virtual wstring toString() const;

        // Getters
        const Player & getPlayer() const { return m_player; }
        const Move & getMove() const { return m_move; }

    protected:
        virtual void doExecute();
        virtual void doUndo();

    private:
        Player &m_player;
        Move m_move;
        PlayedRack m_originalRack;
};

#endif

