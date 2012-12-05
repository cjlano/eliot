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

#ifndef PLAYER_EVENT_CMD_H_
#define PLAYER_EVENT_CMD_H_

#include "command.h"
#include "logging.h"

class Player;


/**
 * This class implements the Command design pattern.
 * It encapsulates the logic to add points to a player score.
 */
class PlayerEventCmd: public Command
{
    DEFINE_LOGGER();

public:
    enum EventType
    {
        SOLO,
        WARNING,
        PENALTY,
        END_GAME,
    };

    PlayerEventCmd(Player &ioPlayer, EventType iEvent, int iPoints = 0);

    virtual bool isInsertable() const { return true; }
    virtual wstring toString() const;

    // Getters
    const Player & getPlayer() const { return m_player; }
    EventType getEventType() const { return m_eventType; }
    int getPoints() const { return m_points; }

protected:
    virtual void doExecute();
    virtual void doUndo();

private:
    Player &m_player;
    EventType m_eventType;
    int m_points;
};

#endif

