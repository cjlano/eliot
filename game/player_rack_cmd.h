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

#ifndef _PLAYER_RACK_CMD_H
#define _PLAYER_RACK_CMD_H

#include "command.h"
#include "pldrack.h"

class Player;


/**
 * This class implements the Command design pattern.
 * It encapsulates the logic to update the player rack, usually to complete it.
 */
class PlayerRackCmd: public Command
{
    public:
        PlayerRackCmd(Player &ioPlayer, const PlayedRack &iNewRack);

    protected:
        virtual void doExecute();
        virtual void doUndo();

    private:
        Player &m_player;
        PlayedRack m_oldRack;
        PlayedRack m_newRack;
};

#endif

