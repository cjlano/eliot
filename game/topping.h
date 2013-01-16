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

#ifndef TOPPING_H_
#define TOPPING_H_

#include <string>

#include "game.h"
#include "move.h"
#include "logging.h"

class Player;

using std::string;
using std::wstring;


/**
 * This class handles the logic specific to a topping game.
 * In this mode, the player plays against time, to find the top move
 * (or one of them if there are several moves with the same score)
 * This mode is mostly interesting for (good) duplicate players.
 */
class Topping: public Game
{
    DEFINE_LOGGER();
    friend class GameFactory;
public:

    /*************************
     * Game handling
     *************************/
    virtual void start();

    virtual bool isFinished() const;

    /// See description of Game::play()
    virtual int play(const wstring &iCoord, const wstring &iWord);

    /**
     * Override the default behaviour of addPlayer(), because in topping
     * mode we only want a human player
     */
    virtual void addPlayer(Player *iPlayer);

    /**
     * Word played by the player, with the corresponding elapsed time, in seconds
     */
    void tryWord(const wstring &iWord, const wstring &iCoord, int iElapsed);

    /**
     * Return all the moves tried by the player (including the invalid ones).
     * The moves are returned in chronological order.
     */
    vector<Move> getTriedMoves() const;

    /**
     * Return the best possible move (or one of them, if there are several ones).
     */
    Move getTopMove() const;

    /**
     * Indicate that the player didn't find the top in the allocated time.
     * This will play the top on the board, give a points penalty to the player
     * and start the next turn.
     */
    void turnTimeOut(int iElapsed);

    /**
     * Give an additional penalty to the player (probably because
     * he used a hint)
     */
    void addPenalty(int iPenalty);

private:
    /// Private constructor and destructor to force using the GameFactory class
    Topping(const GameParams &iParams, const Game *iMasterGame);

    /// Record a player move
    void recordPlayerMove(const Move &iMove, Player &ioPlayer, int iElapsed);

    void endTurn();

    /// Finish the game
    void endGame();

    /**
     * Return the score of the top move.
     */
    int getTopScore() const;

};

#endif

