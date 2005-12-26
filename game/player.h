/*****************************************************************************
 * Copyright (C) 2004-2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
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

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <vector>
#include <string>
#include "pldrack.h"
#include "history.h"

class Turn;


/**
 * This class is the parent class for all the players involved in a game.
 * It defines the common methods to update the rack, score, etc...
 */
class Player
{
public:
    Player(int iId);
    virtual ~Player();

    // Pseudo RTTI
    virtual bool isHuman() const = 0;

    /**************************
     * General getters
     **************************/
    // Get the (possibly incomplete) rack of the player
    const PlayedRack & getCurrentRack() const;
    // Get the previous rack
    const PlayedRack & getLastRack() const;
    // Get the previous round (corresponding to the previous rack...)
    const Round & getLastRound() const;

    void setCurrentRack(const PlayedRack &iPld);

    const History& getHistory() const { return m_history; }

    /**************************
     * Acessors for the score of the player
     **************************/
    // Add (or remove, if iPoints is negative) points to the score
    // of the player
    void addPoints(int iPoints) { m_score += iPoints; }
    int  getPoints() const      { return m_score; }

    // Update the player "history", with the given round.
    // A new rack is created with the remaining letters
    void endTurn(const Round &iRound, int iTurn);

    const std::string toString() const;

private:
    /// ID of the player
    int m_id;

    /// Score of the player
    int m_score;

    /// History of the racks and rounds for the player
    History m_history;
};


/**
 * Human player.
 */
class HumanPlayer: public Player
{
public:
    HumanPlayer(int iId): Player(iId) {}
    virtual ~HumanPlayer() {}

    // Pseudo RTTI
    virtual bool isHuman() const { return true; }
};

#endif

