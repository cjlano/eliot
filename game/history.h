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
#include "turn.h"

class History
{
 public:
    History();
    ~History();

    /// get the size of the history
    int              getSize() const;

    /// Get the (possibly incomplete) rack
    const PlayedRack getCurrentRack() const;

    /// Set the current rack
    void             setCurrentRack(const PlayedRack &iPld);

    /// Get the previous turn
    const Turn       getPreviousTurn() const;

    /// Update the "history" with the given round and, complete the turn.
    /// a new turn is created with the remaining letters in the rack
    void playRound(int player, int turn, const Round& round);

    /// Remove last turn
    void removeLastTurn();

    std::string toString() const;

 private:
    vector < Turn* > history;
};

#endif


/// Local Variables:
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
