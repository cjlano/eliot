/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: duplicate.h,v 1.6 2005/03/27 21:45:04 ipkiss Exp $
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/

#ifndef _DUPLICATE_H_
#define _DUPLICATE_H_

#include "game.h"

using std::string;


/**
 * This class handles the logic specific to a duplicate game.
 * The trick in this mode is that the players will not necessarily play they
 * word always in the same order, so we need to implement a "synchronization":
 *   - when a human player wants to play a word, he plays it, and its score
 *     and rack are updated. He cannot change his word afterwards.
 *   - if there is still a human player who has not played for the current
 *     turn, we wait for him
 *   - if all the human players have played, it's the turn to the AI players
 *     (currently handled in a loop, but we could imagine that they are running
 *     in their own thread).
 *   - once all the players have played, we can really end the turn:
 *     the best word is played on the board, the history of the game is
 *     updated, and the new rack is chosen.
 *
 * AI players play after human ones, because with the current implementation
 * of the interfaces it is too easy for a player to see the rack of other
 * players, and in particular a human player could take advantage of that to
 * have more clues about the best word.
 * TODO: better isolation of the players...
 */
class Duplicate: public Game
{
    friend class GameFactory;
public:
    virtual GameMode getMode() const { return kDUPLICATE; }
    virtual string getModeAsString() const { return "Duplicate"; }

    /*************************
     * Game handling
     *************************/
    virtual int start();
    virtual int setRackRandom(int, bool, set_rack_mode);
    virtual int play(const string &iCoord, const string &iWord);
    virtual int endTurn();
    int setPlayer(int);
    // Switch to the previous human player who has not played yet
    void prevHumanPlayer();
    // Switch to the next human player who has not played yet
    void nextHumanPlayer();

private:
    // Private constructor and destructor to force using the GameFactory class
    Duplicate(const Dictionary &iDic);
    virtual ~Duplicate();

    void playRound(const Round &iRound, int n);
    int  endTurnForReal();
    void end();
    void duplicateAI(int n);

    // m_hasPlayed[p] is true iff player p has played for this turn
    map<int, bool> m_hasPlayed;
};

#endif /* _DUPLICATE_H_ */
