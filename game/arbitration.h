/*****************************************************************************
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

#ifndef ARBITRATION_H_
#define ARBITRATION_H_

#include "duplicate.h"
#include "logging.h"


class LimitResults;
class PlayerEventCmd;

/**
 * This class simply extends the Duplicate game,
 * to specialize it for arbitration purposes.
 */
class Arbitration: public Duplicate
{
    DEFINE_LOGGER();
    friend class GameFactory;
public:

    /**
     * Complete (or reset) the rack randomly.
     * @exception EndGameException if it is impossible to complete the rack
     * for some reason...
     */
    void setRackRandom();

    /**
     * Set the rack with the given letters
     * @exception EndGameException if the game is over
     * @exception GameException if any other error occurs
     */
    void setRackManual(const wstring &iLetters);

    void search(LimitResults &oResults);

    Move checkWord(const wstring &iWord, const wstring &iCoords) const;

    void setSolo(unsigned iPlayerId, int iPoints = 0);
    void removeSolo(unsigned iPlayerId);
    int getSolo(unsigned iPlayerId) const;

    void addWarning(unsigned iPlayerId);
    void removeWarning(unsigned iPlayerId);
    bool hasWarning(unsigned iPlayerId) const;

    void addPenalty(unsigned iPlayerId, int iPoints = 0);
    void removePenalty(unsigned iPlayerId);
    int getPenalty(unsigned iPlayerId) const;

    void assignMove(unsigned int iPlayerId, const Move &iMove);
    void finalizeTurn();

private:
    // Private constructor to force using the GameFactory class
    Arbitration(const GameParams &iParams);

    /// Undo the current rack, and subsequent commands
    void undoCurrentRack();

    /**
     * Return the first player event of the given type (for the given player)
     * in the commands history for the current turn.
     * If none is found, return 0.
     */
    const PlayerEventCmd * getPlayerEvent(unsigned iPlayerId,
                                          int iEventType) const;

};

#endif /* ARBITRATION_H_ */

