/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2008 Olivier Teulière
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

#ifndef FREEGAME_H_
#define FREEGAME_H_

#include "game.h"
#include "logging.h"

class Player;

using std::string;
using std::wstring;
using std::vector;


/**
 * This class handles the logic specific to a "free" game.
 *
 * The algorithm is simple: players play at their turn, and they can either
 * play a word or change letters (changing letters implies passing its turn).
 *
 * When a player has no more letters (end of the game), the points of the
 * letters left in the racks of his opponents are added to his score, and
 * removed from the score of the latters.
 */
class FreeGame: public Game
{
    DEFINE_LOGGER();
    friend class GameFactory;
public:

    /*************************
     * Game handling
     *************************/
    /**
     * Start the game.
     */
    virtual void start();

    virtual bool isFinished() const;

    /**
     * See description of Game::play() for the possible return values.
     * Note that if the "freegame-reject-invalid" setting is set to false
     * the method always returns 0 (the player will have 0 for this turn)
     */
    virtual int play(const wstring &iCoord, const wstring &iWord);

    /**
     * Pass the turn, changing the letters listed in iToChange.
     * If you simply want to pass the turn without changing any letter,
     * provide an empty string.
     *
     * Possible return values:
     *  0: everything went fine
     *  1: changing letters is not allowed if there are less than 7 tiles
     *     left in the bag
     *  2: the rack of the current player does not contain all the
     *     listed letters
     *  3: the game is already finished
     *  4: some letters are invalid for the current dictionary
     */
    int pass(const wstring &iToChange);

private:
    // Private constructor to force using the GameFactory class
    FreeGame(const GameParams &iParams);

    /// Make the AI player whose ID is p play its turn
    void playAI(unsigned int p);

    /// Record a player move
    void recordPlayerMove(const Move &iMove, Player &ioPlayer, bool isForHuman);

    /// Finish the current turn
    int endTurn();

    /// Finish the game
    void endGame();

    /**
     * Check whether it is legal to change the letters of iToChange.
     * The return codes are the same as the ones on the pass() method
     */
    int checkPass(const Player &iPlayer, const wstring &iToChange) const;
};

#endif /* _FREEGAME_H_ */

