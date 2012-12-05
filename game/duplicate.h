/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2012 Olivier Teulière
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

#ifndef DUPLICATE_H_
#define DUPLICATE_H_

#include "game.h"
#include "move.h"
#include "logging.h"

class Player;
class PlayedRack;
class Move;
class PlayerEventCmd;

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
    DEFINE_LOGGER();
    friend class GameFactory;
    friend class MarkPlayedCmd;
    friend class MasterMoveCmd;
public:

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

    virtual bool isFinished() const;

    /**
     * See description of Game::play() for the possible return values.
     * Note that if the "duplicate-reject-invalid" setting is set to false
     * the method always returns 0 (the player will have 0 for this turn)
     */
    virtual int play(const wstring &iCoord, const wstring &iWord);

    /**
     * Set the current player, given its ID.
     * The given player ID must correspond to a human player, who did not
     * play yet for this turn.
     * @param p: ID of the player
     * @exception GameException: Thrown if the player is not human or if
     *      he has already played
     */
    void setPlayer(unsigned int p);

    /**
     * Set the master move.
     * Two move types are possible: VALID_ROUND and NO_MOVE.
     * Setting a master move to NO_MOVE means reseting it.
     */
    void setMasterMove(const Move &iMove);

    const Move &getMasterMove() const { return m_masterMove; }

    /// Return true if the player has played for the current turn
    virtual bool hasPlayed(unsigned int iPlayerId) const;

protected:
    // Protected constructor to force using the GameFactory class
    Duplicate(const GameParams &iParams);

    /// Record a player move
    void recordPlayerMove(Player &ioPlayer, const Move &iMove);

    /// Helper function to set the game rack and the players rack at the same time
    void setGameAndPlayersRack(const PlayedRack &iRack);

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

    /// Finish the game
    void endGame();

    /**
     * Return the first player event of the given type (for the given player)
     * in the commands history for the current turn.
     * If none is found, return 0.
     */
    const PlayerEventCmd * getPlayerEvent(unsigned iPlayerId,
                                          int iEventType) const;

    /**
     * Automatically set the solo for the current turn.
     * First, all the existing solos (there could be several, in arbitration
     * mode with a crazy arbitrator...) are removed. Then, a solo is given to
     * the player deserving it, if any.
     * Note that the minimum number of players specified in the preferences
     * must be reached for the solo to be applicable.
     */
    void setSoloAuto(unsigned int minNbPlayers, int iSoloValue);

private: // Used by friend classes
    void innerSetMasterMove(const Move &iMove);
    bool isArbitrationGame() const;

private:
    /// Make the AI player whose ID is p play its turn
    void playAI(unsigned int p);

    /**
     * Find the player who scored the most  (with a valid move) at this turn.
     * If several players have the same score, one is returned arbitrarily.
     * If nobody played a valid move, the method returns a null pointer.
     */
    Player * findBestPlayer() const;

    /**
     * This function really changes the turn, i.e. the best word is played,
     * the game history is updated, a "solo" bonus is given if needed, and
     * all racks are made equal to the one of the player who played the
     * best move.
     * We suppose here that all the players have finished to play for this
     * turn (this should have been checked by tryEndturn())
     */
    void endTurn();

    /**
     * Master move, i.e. the move that will be played on the board
     * at this turn (even if no player actually played it).
     * This is particularly useful for arbitration mode, but could also
     * be used in normal Duplicate games.
     */
    Move m_masterMove;
};

#endif /* _DUPLICATE_H_ */

