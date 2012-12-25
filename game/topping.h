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

private:
    /// Private constructor and destructor to force using the GameFactory class
    Topping(const GameParams &iParams);

    /// Record a player move
    void recordPlayerMove(const Move &iMove, Player &ioPlayer);

    void endTurn();

    /// Finish the game
    void endGame();

};

#endif

