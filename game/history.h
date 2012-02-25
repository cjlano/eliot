/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2007 Antoine Fraboulet & Olivier Teulière
 * Authors: Antoine Fraboulet <antoine.fraboulet @@ free.fr>
 *          Olivier Teulière <ipkiss @@ gmail.com>
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

#ifndef HISTORY_H_
#define HISTORY_H_

#include <string>
#include <vector>

#include "logging.h"

using std::wstring;
using std::vector;

class Move;
class Turn;
class PlayedRack;
class Rack;

/**
 * History stores all the turns that have been played
 * This class is used many times in the game
 *  - one for the complete game
 *  - one for each player
 *
 * The top of the history is an empty Turn until it has been filled
 * and the game is up to a new turn. So a History object is never empty.
 * However, the getSize() method only returns the number of complete
 * turns, and can therefore return 0.
 *
 * getCurrentRack() can/should be used to store the current played rack.
 * setCurrentRack must be called whenever the current played rack is
 * modified.
 *
 * History owns the turns that it stores. Do not delete a turn referenced
 * by History
 */
class History
{
    DEFINE_LOGGER();
public:
    History();
    ~History();

    /// Get the size of the history (without the current incomplete turn)
    unsigned int getSize() const;

    /// Get the (possibly incomplete) rack
    const PlayedRack& getCurrentRack() const;

    /// Set the current rack
    void setCurrentRack(const PlayedRack &iPld);

    /// Get the previous (complete) turn
    const Turn& getPreviousTurn() const;

    /// Get turn 'n' (starting at 0)
    const Turn& getTurn(unsigned int) const;

    /**
     * Return true if the history doesn't contain at least one move
     * corresponding to a valid round, false otherwise.
     * Said differently, this method checks whether a word was already played
     * on the board.
     */
    bool beforeFirstRound() const;

    /**
     * Update the history with the given move and complete the turn.
     * A new turn is created with the unplayed letters in the rack
     * 03 sept 2000: We have to sort the tiles according to the new rules
     */
    void playMove(unsigned int player, const Move &iMove,
                  const Rack &iNewRack);

    /// Remove last turn
    void removeLastTurn();

    /// String handling
    wstring toString() const;

 private:
    vector<Turn*> m_history;
};

#endif

