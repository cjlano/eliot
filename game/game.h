/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2007 Antoine Fraboulet & Olivier Teulière
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
    /// Get the bag
    const Bag& getBag() const { return m_bag; }
    /// Get the history of the game */
    const History& getHistory() const { return m_history; }

    /***************
     * Methods to access players.
     ***************/

    const Player& getPlayer(unsigned int iNum) const;
    const Player& getCurrentPlayer() const { return getPlayer(currPlayer()); };
    unsigned int getNPlayers() const { return m_players.size(); }
    unsigned int getNHumanPlayers() const;
    virtual void addHumanPlayer();
    // TODO: Ability to specify which kind of AI player is wanted
    virtual void addAIPlayer();
    unsigned int currPlayer() const { return m_currPlayer; }

    /***************
     * Game handling
     ***************/

    /**
     * Start the game.
     * AI players are handled automatically, so if the game only has AI
     * players, it will play until the end.
     */
    virtual int start() = 0;

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
     *  7: invalid crosscheck, or word going out of the board
     *  8: word already present on the board (no new letter from the rack)
     *  9: isolated word (not connected to the rest)
     * 10: first word not horizontal
     * 11: first word not covering the H8 square
     */
    virtual int play(const wstring &iCoord, const wstring &iWord) = 0;

    /**
     * Go back to turn iTurn.
     * We must have: iTurn < getHistory().getSize()
     * Possible return values:
     *  0: everything went fine
     *  1: iTurn is invalid
     */
    int back(unsigned int iTurn);

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

    enum set_rack_mode {RACK_ALL, RACK_NEW, RACK_MANUAL};


protected:
    /// All the players, indexed by their ID
    vector<Player*> m_players;
    /// ID of the "current" player
    unsigned int m_currPlayer;

// TODO: check what should be private and what should be protected
// private:

    /// Variant
    GameVariant m_variant;

    /// Dictionary currently associated to the game
    const Dictionary & m_dic;

    /// Bag
    Bag m_bag;

    /// Board
    Board m_board;

    /**
     * History of the game.
     * The vector is indexed by the number of turns in the game
     */
    History m_history;

    int m_points;

    bool m_finished;

    /*********************************************************
     * Helper functions
     *********************************************************/

    /** Play a Move for the given player, updating game history */
    void helperPlayMove(unsigned int iPlayerId, const Move &iMove);

    /**
     * Set the rack randomly for the player p
     * Possible return values:
     *  0: everything went fine
     *  1: the game is over
     *  3: there is no chance to set the rack with the vowels/consonants
     *     constraints
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
    int helperSetRackRandom(unsigned int p, bool iCheck, set_rack_mode mode);

    /**
     * Set the rack randomly for the player p
     * Possible return values:
     *  0: everything went fine
     *  1: the game is over
     *  2: the rack was checked and was not correct (try calling the
     *     function again)
     *  3: there is no chance to set the rack with the vowels/consonants
     *     constraints
     *
     * @deprecated: use helperSetRackRandom instead
     */
    int helperSetRackRandomOld(unsigned int p, bool iCheck, set_rack_mode mode);

    /**
     * Set the rack for the player p with the given letters
     * Possible return values:
     *  0: everything went fine
     *  1: the bag doesn't have the wanted letters
     *  2: the rack was checked for vowels/consonants and was not correct
     */
    int helperSetRackManual(unsigned int p, bool iCheck, const wstring &iLetters);

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
     * The realBag is the current bag minus all the racks
     * present in the game. It represents the actual
     * letters that are left in the bag.
     * FIXME: in Duplicate mode, this method uses m_currPlayer to find the
     * rack of the player. Since not all the players played the same word,
     * it is important to set m_currPlayer accurately before!
     */
    void realBag(Bag &iBag) const;

    /**
     * This function checks whether it is legal to play the given word at the
     * given coordinates. If so, the function fills a Round object, also given
     * as a parameter.
     * Possible return values: same as the play() method
     */
    int  checkPlayedWord(const wstring &iCoord,
                         const wstring &iWord, Round &oRound);

    /**
     * load games from File using the first format.
     * This format is used for Training games
     */
    static Game* gameLoadFormat_14(FILE *fin, const Dictionary& iDic);

    /**
     * load games from File using advanced format (since Eliot 1.5)
     * This format is used for Duplicate, Freegame, ...
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

private:

    /**
     * Play a round on the board.
     * This should only be called by helperPlayMove().
     */
    void helperPlayRound(unsigned int iPlayerId, const Round &iRound);

};

#endif /* _GAME_H_ */

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
