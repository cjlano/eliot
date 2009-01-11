/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2008 Antoine Fraboulet & Olivier Teulière
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

#ifndef _GAME_H_
#define _GAME_H_

#include <string>
#include <vector>
#include <iostream>
#include "bag.h"
#include "board.h"
#include "history.h"
#include "navigation.h"
#include "command.h"

class Dictionary;
class Player;
class PlayedRack;
class Round;
class Rack;
class Turn;

using namespace std;


/**
 * Parent class of all the Game types.
 * It offers the common attributes (Board, Bag, etc...) as well as useful
 * "helper" methods to factorize some code.
 */
class Game
{
public:
    /// Game specs.
    static const unsigned int RACK_SIZE;
    static const int BONUS_POINTS;


    Game(const Dictionary &iDic);
    virtual ~Game();

    /***************
     * Game type
     ***************/

    /// Game mode: each one of these modes is implemented in an inherited class
    enum GameMode
    {
        kTRAINING,
        kFREEGAME,
        kDUPLICATE
    };
    virtual GameMode getMode() const = 0;
    virtual string getModeAsString() const = 0;

    /// Game variant: it slightly modifies the rules of the game
    enum GameVariant
    {
        kNONE,      // Normal game rules
        kJOKER      // Joker game
    };

    /**
     * Accessors for the variant of the game.
     * The variant can be changed during a game without any problem
     * (though it seems rather useless...)
     */
    void setVariant(GameVariant iVariant)   { m_variant = iVariant; }
    GameVariant getVariant() const          { return m_variant; }

    /***************
     * Various getters
     ***************/

    /**
     * Get the dictionary associated with the game.
     * You should never create a new dictionary object while a Game
     * object still exists
     */
    const Dictionary & getDic() const   { return m_dic; }

    /// Get the board
    const Board& getBoard() const { return m_board; }
    Board & accessBoard() { return m_board; }
    /// Get the bag
    const Bag& getBag() const { return m_bag; }
    Bag & accessBag() { return m_bag; }
    /**
     * The realBag is the current bag minus all the racks
     * present in the game. It represents the actual
     * letters that are left in the bag.
     * FIXME: in Duplicate mode, this method uses m_currPlayer to find the
     * rack of the player. Since not all the players played the same word,
     * it is important to set m_currPlayer accurately before!
     */
    void realBag(Bag &iBag) const;


    /// Get the history of the game */
    const History& getHistory() const { return m_history; }
    History & accessHistory() { return m_history; }

    /***************
     * Methods to access players.
     ***************/

    const Player& getPlayer(unsigned int iNum) const;
    const Player& getCurrentPlayer() const { return getPlayer(currPlayer()); };
    unsigned int getNPlayers() const { return m_players.size(); }
    unsigned int getNHumanPlayers() const;
    unsigned int currPlayer() const { return m_currPlayer; }

    /**
     * Add a player to the game.
     * The Game object takes ownership of the given player
     */
    virtual void addPlayer(Player *iPlayer);

    /***************
     * Game handling
     ***************/

    /**
     * Start the game.
     * AI players are handled automatically, so if the game only has AI
     * players, it will play until the end.
     */
    virtual void start() = 0;

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
    virtual int play(const wstring &iCoord, const wstring &iWord) = 0;

    /// Shuffle the rack of the current player
    void shuffleRack();

    /// Return true if the player has played for the current turn
    // XXX: not very nice API, should be a player property...
    virtual bool hasPlayed(unsigned int player) const { return player != currPlayer(); }

    /***************
     * Saved games handling
     ***************/

    /**
     * Possible formats for the saved games
     */
    enum game_file_format
    {
        FILE_FORMAT_STANDARD,
        FILE_FORMAT_ADVANCED
    };

    /**
     * load() returns the loaded game, or NULL if there was a problem
     * load() does need some more work to be robust enough to
     * handle "hand written" files
     */
    static Game * load(FILE *fin, const Dictionary &iDic);

    /**
     * Save a game to a file
     * Standard format is used for training games so that it is compatible
     * with previous versions of Eliot.
     *
     * Saving can be forced to advanced format for training games by
     * setting the last parameter to FILE_FORMAT_ADVANCED
     */
    void save(ostream &out, game_file_format format = FILE_FORMAT_STANDARD) const;

    /***************
     * Setting the rack
     ***************/

    enum set_rack_mode {RACK_ALL, RACK_NEW};

    void addPoints(int iPoints) { m_points += iPoints; }

    const Navigation & getNavigation() const { return m_navigation; }
    Navigation & accessNavigation() { return m_navigation; }

private:
    /// Variant
    GameVariant m_variant;

    /// Dictionary currently associated to the game
    const Dictionary & m_dic;

    /**
     * History of the game.
     */
    History m_history;

    Navigation m_navigation;

    int m_points;

    /// Change the player who is supposed to play
    void setCurrentPlayer(unsigned int iPlayerId) { m_currPlayer = iPlayerId; }

    /// Command used to keep track of the current player changes
    class CurrentPlayerCmd: public Command
    {
        public:
            CurrentPlayerCmd(Game &ioGame,
                             unsigned int iPlayerId);

            virtual wstring toString() const;

        protected:
            virtual void doExecute();
            virtual void doUndo();

        private:
            Game &m_game;
            unsigned int m_newPlayerId;
            unsigned int m_oldPlayerId;
    };

// TODO: check what should be private and what should be protected
protected:
    /// All the players, indexed by their ID
    vector<Player*> m_players;
    /// ID of the "current" player
    unsigned int m_currPlayer;

    /// Board
    Board m_board;

    /// Bag
    Bag m_bag;

    bool m_finished;

    /*********************************************************
     * Helper functions
     *********************************************************/

    /**
     * Complete the given rack randomly.
     *
     * Completing a rack randomly is more complex than it seems, because we
     * must take into account several constraints:
     *  - if iCheck is true, we must ensure that the rack contains a minimum
     *    number of vowels and consonants (2 of each in the 15 first moves of
     *    the game, 1 of each after)
     *  - the game is over if the (real) bag contains only vowels or only
     *    consonants, and in particular if it contains only one letter
     *  - some letters (in particular the joker) can count both as a vowel and
     *    as a consonant (but not at the same time)
     *  - in a joker game, the joker must be present in the rack unless there
     *    is no joker left in the bag. In addition, we must prevent that both
     *    jokers are present in the rack at the same time
     *  - if completing a rack doesn't meet the requirements on the vowels and
     *    consonants, we must reject the rack completely (but only once,
     *    otherwise we have no guarantee that the rejects will stop eventually).
     *    This also means we have to check whether completing the rack with the
     *    requirements is possible...
     */
    PlayedRack helperSetRackRandom(const PlayedRack &iPld,
                                   bool iCheck, set_rack_mode mode) const;

    /**
     * Set the rack for the player p with the given letters
     * Possible return values:
     *  0: everything went fine
     *  1: the bag doesn't have the wanted letters
     *  2: the rack was checked for vowels/consonants and was not correct
     */
    int helperSetRackManual(unsigned int p, bool iCheck, const wstring &iLetters);

    void firstPlayer();
    void prevPlayer();
    void nextPlayer();

    /**
     * Check if the players rack can be obtained from the bag.
     * Since letters are removed from the bag only when the
     * round is played we need to check that ALL the racks
     * are in the bag simultaneously.
     *
     * FIXME: since we do not check for all racks it works
     * for training and duplicate but it won't work for
     * freegames.
     */
    bool rackInBag(const Rack &iRack, const Bag &iBag) const;

    /**
     * This function checks whether it is legal to play the given word at the
     * given coordinates. If so, the function fills a Round object, also given
     * as a parameter.
     * Possible return values: same as the play() method
     */
    int checkPlayedWord(const wstring &iCoord,
                        const wstring &iWord, Round &oRound) const;

    /**
     * load games from File using the first format.
     * This format is used for Training games
     */
    static Game* gameLoadFormat_14(FILE *fin, const Dictionary& iDic);

    /**
     * load games from File using advanced format (since Eliot 1.5)
     * This format is used for Duplicate, FreeGame, ...
     */
    static Game* gameLoadFormat_15(FILE *fin, const Dictionary& iDic);

    /**
     * Training games ares saved using the initial Eliot format
     */
    void gameSaveFormat_14(ostream &out) const;

    /**
     * Advanced game file format output
     */
    void gameSaveFormat_15(ostream &out) const;

};

#endif /* _GAME_H_ */

