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

#ifndef _DUPLICATE_H_
#define _DUPLICATE_H_

#include "game.h"

class Player;

using std::string;
using std::wstring;


/**
 * This class handles the logic specific to a duplicate game.
 * The trick in this mode is that the players will not necessarily play they
 * word always in the same order, so we need to implement a "synchronization":
 *   - when a human player wants to play a word, he plays it, and its score
 *     and rack are updated. He cannot change his word afterwards.
 *   - if there is still a human player who has not played for the current
 *     turn, we wait for him
 *   - if all the human players have played, it's the turn of the AI players
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
    /**
     * In Duplicate mode, the start() method starts a new turn, and is
     * automatically called when the previous turn is finished.
     *
     * Pre-requisite: all the players must have the same rack when this
     * method is called
     */
    virtual void start();

    /**
     * See description of Game::play() for the possible return values.
     * Note that if the "duplicate-reject-invalid" setting is set to false
     * the method always returns 0 (the player will have 0 for this turn)
     */
    virtual int play(const wstring &iCoord, const wstring &iWord);

    /**
     * Set the current player, given its ID.
     * The given player ID must correspond to a human player, which did not
     * play yet for this turn.
     * Possible return values:
     *  0: everything went fine
     *  1: the player is not human
     */
    int setPlayer(unsigned int p);

    /// Switch to the previous human player who has not played yet
    void prevHumanPlayer();

    /// Switch to the next human player who has not played yet
    void nextHumanPlayer();

    /// Return true if the player has played for the current turn
    // XXX: not very nice API, should be a player property...
    virtual bool hasPlayed(unsigned int player) const;

private:
    // Private constructor to force using the GameFactory class
    Duplicate(const Dictionary &iDic);

    /// Record a player move
    void recordPlayerMove(const Move &iMove, Player &ioPlayer);

    /// Make the AI player whose ID is p play its turn
    void playAI(unsigned int p);

    /**
     * This function does not terminate the turn itself, but performs some
     * checks to know whether or not it should be terminated (with a call to
     * endTurn()).
     *
     * For the turn to be terminated, all the players must have played.
     * Since the AI players play after the human players, we check whether
     * one of the human players has not played yet:
     *   - if so, we have nothing to do (we are waiting for him/her)
     *   - if not (all human players have played), the AI players can play,
     *     and we finish the turn.
     */
    void tryEndTurn();

    /**
     * This function really changes the turn, i.e. the best word is played,
     * the game history is updated, a "solo" bonus is given if needed, and
     * all racks are made equal to the one of the player who played the
     * best move.
     * We suppose here that all the players have finished to play for this
     * turn (this should have been checked by tryEndturn())
     */
    void endTurn();

    /// Finish the game
    void endGame();

    // m_hasPlayed[p] is true iff player p has played for this turn
    map<unsigned int, bool> m_hasPlayed;
};

#endif /* _DUPLICATE_H_ */

