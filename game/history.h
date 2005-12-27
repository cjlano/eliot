/* Eliot                                                                     */
/* Copyright (C) 1999  Antoine Fraboulet                                     */
/*                                                                           */
/* This file is part of Eliot.                                               */
/*                                                                           */
/* Eliot is free software; you can redistribute it and/or modify             */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* Eliot is distributed in the hope that it will be useful,                  */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

/**
 *  \file   history.h
 *  \brief  Game history system
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#ifndef _HISTORY_H
#define _HISTORY_H

#include <vector>
#include "pldrack.h"
#include "round.h"
#include "turn.h"

/**
 * History stores all the turns that have been played
 * This class is used many times in the game
 *  - one for the complete game
 *  - one for each of the players
 *
 * A History is never void (getSize() can be used as the is the current turn
 * number for the complete game history). 
 *
 * History starts at zero.
 *
 * The top of the history is an empty
 * Turn until it has been filled and game is up to a new round.
 * 
 * getCurrentRack() can/should be used to store the current played rack. 
 * setCurrentRack must be called whenever the current played rack is
 * modified.
 * 
 * History owns the turns that it stores. Do not delete a turn referenced by History
 */

class History
{
 public:
    History();
    virtual ~History();

    /// get the size of the history
    int               getSize() const;

    /// Get the (possibly incomplete) rack
    const PlayedRack& getCurrentRack() const;

    /// Set the current rack
    void              setCurrentRack(const PlayedRack &iPld);

    /// Get the previous turn
    const Turn&       getPreviousTurn() const;

    /// Get turn 'n'
    const Turn&       getTurn(unsigned int) const;

    /// Update the "history" with the given round and complete the turn.
    /// A new turn is created with the remaining letters in the rack
    void playRound(int player, int turn, const Round& round);

    /// Remove last turn
    void removeLastTurn();

    /// String handling
    std::string toString() const;

 private:
    std::vector < Turn* > m_history;
};

#endif


/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
