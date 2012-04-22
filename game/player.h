/*****************************************************************************
 * Eliot
 * Copyright (C) 2004-2007 Olivier Teulière & Antoine Fraboulet
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
 *          Antoine Fraboulet <antoine.fraboulet @@ free.fr>
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

#ifndef PLAYER_H_
#define PLAYER_H_

#include <vector>
#include <string>
#include "pldrack.h"
#include "history.h"
#include "logging.h"

using std::wstring;

class Turn;
class Rack;


/**
 * This class is the parent class for all the players involved in a game.
 * It defines the common methods to update the rack, score, etc...
 */
class Player
{
    DEFINE_LOGGER();
public:
    explicit Player();
    virtual ~Player() {}

    // Pseudo RTTI
    virtual bool isHuman() const = 0;

    /// Get the name of the player
    const wstring & getName() const { return m_name; }
    /// Set the name of the player
    void setName(const wstring &iName) { m_name = iName; }

    /// Accessors for the table number
    unsigned getTableNb() const { return m_tableNb; }
    void setTableNb(unsigned tableNb) { m_tableNb = tableNb; }

    /// ID handling
    unsigned int getId() const { return m_id; }
    void setId(unsigned int iId);

    /**************************
     * General getters
     **************************/
    /// Get the (possibly incomplete) rack of the player
    const PlayedRack & getCurrentRack() const;
    /// Get the previous rack
    const PlayedRack & getLastRack() const;
    /// Get the previous move (corresponding to the previous rack...)
    const Move & getLastMove() const;

    void setCurrentRack(const PlayedRack &iPld);

    const History& getHistory() const { return m_history; }
    History & accessHistory() { return m_history; }

    /// Remove last turn
    void removeLastTurn();

    /// Return the total number of warnings received
    unsigned getWarningsNb() const;

    /**************************
     * Accessors for the score of the player
     **************************/
    // Add (or remove, if iPoints is negative) points to the score
    // of the player
    void addPoints(int iPoints) { m_score += iPoints; }
    int  getPoints() const      { return m_score; }

    wstring toString() const;

private:
    /// ID of the player
    unsigned int m_id;

    /// Score of the player
    int m_score;

    /// Name of the player
    wstring m_name;

    // Table number (optional: set to 0 if not used)
    unsigned m_tableNb;

    /// History of the racks and rounds for the player
    History m_history;
};


/**
 * Human player.
 */
class HumanPlayer: public Player
{
public:
    HumanPlayer(): Player() {}
    virtual ~HumanPlayer() {}

    // Pseudo RTTI
    virtual bool isHuman() const { return true; }
};

#endif

