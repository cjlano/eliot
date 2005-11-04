/*****************************************************************************
 * Copyright (C) 2005 Eliot
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

#ifndef _FREEGAME_H_
#define _FREEGAME_H_

#include "game.h"
#include "tile.h"

using std::string;
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
    friend class GameFactory;
public:
    virtual GameMode getMode() const { return kFREEGAME; }
    virtual string getModeAsString() const { return "Free game"; }

    /*************************
     * Game handling
     *************************/
    virtual int start();
    virtual int setRackRandom(int, bool, set_rack_mode);
    virtual int play(const string &iCoord, const string &iWord);
    virtual int endTurn();
    int pass(const string &iToChange, int n);

private:
    // Private constructor and destructor to force using the GameFactory class
    FreeGame(const Dictionary &iDic);
    virtual ~FreeGame();

    void freegameAI(int n);
    void end();
    int helperPass(const vector<Tile> &iToChange, int n);
};

#endif /* _FREEGAME_H_ */
