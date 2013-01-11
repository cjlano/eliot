/*****************************************************************************
 * Eliot
 * Copyright (C) 2008-2012 Olivier Teulière
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

#ifndef PUBLIC_GAME_H_
#define PUBLIC_GAME_H_

#include <vector>
#include <string>

class GameParams;
class Game;
class Dictionary;
class Bag;
class Board;
class History;
class Player;
class Navigation;
class Round;
class Results;
class LimitResults;
class Move;
class PlayedRack;

using namespace std;


/**
 * This class is a wrapper around a Game object (Façade design pattern).
 *
 * Game objects are not meant for direct use outside of the game library,
 * because they expose too many internal members.
 * A PublicGame provides a cleaner interface to implement a UI, avoiding
 * the need to know too much about the Game internals.
 */
class PublicGame
{
public:
    /**
     * Build a PublicGame from a Game object
     * The PublicGame takes the ownership of the Game object.
     */
    // XXX: should be private?
    PublicGame(Game &iGame);
    ~PublicGame();

    /***************
     * Game type
     ***************/

    // XXX: should not be in Game?
    /// Game mode: each one of these modes is implemented in an inherited class
    enum GameMode
    {
        kTRAINING,
        kFREEGAME,
        kDUPLICATE,
        kARBITRATION,
        kTOPPING,
    };
    GameMode getMode() const;

    /***************
     * Various getters
     ***************/

    /// Get the game characteristics
    const GameParams & getParams() const;

    bool hasMasterGame() const;

    /**
     * Get the dictionary associated with the game.
     * You should never create a new dictionary object while a Game
     * object still exists
     */
    const Dictionary & getDic() const;

    /// Get the board
    const Board& getBoard() const;
    /// Get the bag
    const Bag& getBag() const;
    /// Get the rack
    const PlayedRack & getCurrentRack() const;

    /// Get the history of the game */
    const History& getHistory() const;

    /***************
     * Methods to access players.
     ***************/

    /**
     * Add a player to the game.
     * The Game object takes ownership of the given player
     */
    void addPlayer(Player *iPlayer);

    const Player& getPlayer(unsigned int iNum) const;
    const Player& getCurrentPlayer() const;
    unsigned int getNbPlayers() const;
    unsigned int getNbHumanPlayers() const;

    void setPlayerName(unsigned iPlayerId, const wstring &iName);
    void setPlayerTableNb(unsigned iPlayerId, unsigned iTableNb);

    /// Return true if the player has played for the current turn
    // XXX: not very nice API, should be a player property...
    bool hasPlayed(unsigned int player) const;

    /***************
     * Game handling
     ***************/

    /**
     * Start the game.
     * AI players are handled automatically, so if the game only has AI
     * players, it will play until the end.
     */
    void start();

    /**
     * Indicate whether we reached the end of the game.
     * This should be checked regularly.
     * XXX: using a signal would be nice here...
     */
    bool isFinished() const;

    /**
     * Method used by human players to play the word iWord at coordinates
     * iCoord, and end the turn (if possible)
     * Possible return values:
     *  0: correct word, the Round can be used by the caller
     *  1: one letter of the word is invalid in the current dictionary
     *  2: invalid coordinates (unreadable or out of the board)
     *  3: word not present in the dictionary
     *  4: not enough letters in the rack to play the word
     *  5: word is part of a longer one
     *  6: word overwriting an existing letter
     *  7: invalid crosscheck
     *  8: word already present on the board (no new letter from the rack)
     *  9: isolated word (not connected to the rest)
     * 10: first word not horizontal
     * 11: first word not covering the H8 square
     * 12: word going out of the board
     */
    int play(const wstring &iWord, const wstring &iCoord);

    // TODO: doc
    // Ignores the word validity and connexion with the rest
    int checkPlayedWord(const wstring &iWord, const wstring &iCoord,
                        Move &oMove) const;

    /**
     * Compute the points for playing the word iWord at coordinates iCoord.
     * A negative return value indicates an error (same codes as for the
     * play() method, but negative instead of positive).
     */
    int computePoints(const wstring &iWord, const wstring &iCoord) const;

    /// Shuffle the rack of the current player
    void shuffleRack();

    /**
     * Reorder the rack of the given player.
     * An assertion is there to check that the letters are the same.
     */
    void reorderRack(const PlayedRack &iRack);

    /**
     * Place a temporary word on the board for preview purposes.
     * It is up to the caller to provide a Round
     * which makes sense for the current game.
     */
    void setTestRound(const Round &iRound);
    /** Remove the round set with setTestRound */
    void removeTestRound();

    /***************
     * Training games
     * These methods throw an exception if the current game is not in
     * the Training mode.
     ***************/

    void trainingSearch();
    const Results& trainingGetResults() const;
    int trainingPlayResult(unsigned int iResultIndex);

    enum RackMode
    {
        kRACK_NEW, // Only new tiles
        kRACK_ALL  // All tiles
    };

    /**
     * Complete (or reset) the rack randomly.
     * @exception EndGameException if it is impossible to complete the rack
     * for some reason...
     */
    void trainingSetRackRandom(bool iCheck, RackMode iRackMode);

    void trainingSetRackManual(bool iCheck, const wstring &iLetters);


    /***************
     * Topping games
     * These methods throw an exception if the current game is not in
     * the Topping mode.
     ***************/

    void toppingPlay(const wstring &iWord, const wstring &iCoord, int iElapsed);

    void toppingTimeOut();

    void toppingAddPenalty(int iPenalty);

    vector<Move> toppingGetTriedMoves() const;

    Move toppingGetTopMove() const;

    /***************
     * Duplicate games
     * These methods throw an exception if the current game is not in
     * the Duplicate mode.
     ***************/

    /**
     * Set the current player, given its ID.
     * The given player ID must correspond to a human player, who did not
     * play yet for this turn.
     * @param p: ID of the player
     * @exception GameException: Thrown if the player is not human or if
     *      he has already played
     */
    void duplicateSetPlayer(unsigned int p);

    void duplicateSetMasterMove(const Move &iMove);
    const Move & duplicateGetMasterMove() const;

    /***************
     * FreeGame games
     * These methods throw an exception if the current game is not in
     * the FreeGame mode.
     ***************/

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
    int freeGamePass(const wstring &iToChange);

    /***************
     * Arbitration games
     * These methods throw an exception if the current game is not in
     * the Arbitration mode
     ***************/

    /**
     * Complete the rack randomly.
     * @exception EndGameException if it is impossible to complete the rack
     * for some reason...
     */
    void arbitrationSetRackRandom();

    /**
     * Set the rack manually
     * @exception EndGameException if the game is over
     * @exception GameException if any other error occurs
     */
    void arbitrationSetRackManual(const wstring &iLetters);

    void arbitrationSearch(LimitResults &oResults);

    Move arbitrationCheckWord(const wstring &iWord,
                              const wstring &iCoords) const;

    void arbitrationToggleSolo(unsigned iPlayerId);
    int arbitrationGetSolo(unsigned iPlayerId) const;

    void arbitrationToggleWarning(unsigned iPlayerId);
    bool arbitrationHasWarning(unsigned iPlayerId) const;

    void arbitrationTogglePenalty(unsigned iPlayerId);
    int arbitrationGetPenalty(unsigned iPlayer) const;

    void arbitrationAssign(unsigned playerId, const Move &iMove);
    void arbitrationFinalizeTurn();

    /***************
     * Saved games handling
     ***************/

    /**
     * Return the loaded game, from an XML file.
     * An exception is thrown in case of problem.
     */
    static PublicGame * load(const string &iFileName, const Dictionary &iDic);

    /**
     * Save a game to a XML file
     */
    void save(const string &iFileName) const;

    /***************
     * Navigation in the game history
     ***************/

    unsigned int getCurrTurn() const;
    unsigned int getNbTurns() const;
    bool isFirstTurn() const;
    bool isLastTurn() const;

    void firstTurn();
    void prevTurn();
    void nextTurn();
    void lastTurn();

    /**
     * Get rid of the future turns of the game, the current turn
     * becoming the last one.
     */
    void clearFuture();

    /**
     * Print the contents of the commands history, to ease debugging
     */
    void printTurns() const;

private:
    /// Wrapped game
    Game &m_game;
};

#endif

