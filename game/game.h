/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
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

class Player;
class PlayedRack;
class Round;
class Rack;
class Turn;
typedef struct _Dictionary * Dictionary;

using namespace std;


/*************************
 * Ident string used to identify saved Eliot games
 *************************/
#define IDENT_STRING "Eliot"

/**
 * Parent class of all the Game types.
 * It offers the common attributes (Board, Bag, etc...) as well as useful
 * "helper" methods to factorize some code.
 */
class Game
{
public:
    Game(const Dictionary &iDic);
    virtual ~Game();

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

    /**
     * Dictionary associated with the game.
     * The dictionary can be changed during a game without problem
     */
    const Dictionary & getDic() const   { return *m_dic; }
    void setDic(const Dictionary &iDic) { m_dic = &iDic; }

    const Board&  getBoard() const { return m_board; }
    const Bag&    getBag()   const { return m_bag; }
    const Player& getPlayer(int iNum) const;
    const Turn&   getTurn(int iNum) const;
    const Player& getCurrentPlayer() const { return getPlayer(currPlayer()); };

    /**
     * Saved games handling.
     *
     * load() returns the loaded game, or NULL if there was a problem
     * load() might need some more work to be robust enough to
     * handle "hand written" files
     */
    static Game * load(FILE *fin, const Dictionary &iDic);
    void save(ostream &out) const;

    /*************************
     * Playing the game
     * the int parameter should be 0 <= int < getNTurns
     *************************/
    int back(int);

    /*************************
     * Set the rack for searching
     *
     * The int parameter is a boolean, if this parameter
     * set the rack will check that there are at least
     * 2 vowels and 2 consonants before the round 15.
     *
     * The setrackmanual parameter string has to contain
     * 'a' <= char <= 'z' or 'A' <= char <= 'Z' or '?'
     *
     * return value
     *    0 : the rack has been set
     *    1 : the bag does not contain enough tiles
     *    2 : the rack check was set on and failed
     *    3 : the rack cannot be completed (Game_*_setrackrandom only)
     *************************/
    static const int RACK_SIZE;
    enum set_rack_mode {RACK_ALL, RACK_NEW, RACK_MANUAL};
    int setRack(int player, set_rack_mode mode, bool check, const string& str);
    string getPlayerRack(int, bool = false) const;

    /**
     * Methods to access already played words.
     * The int parameter should be 0 <= int < getNTurns()
     */
    const History& getHistory() { return m_history; }

    /**
     * Methods to access players.
     * The int parameter should be 0 <= int < getNPlayers()
     */
    int  getNPlayers() const    { return m_players.size(); }
    int  getNHumanPlayers() const;
    virtual void addHumanPlayer();
    // TODO: Ability to specify which kind of AI player is wanted
    virtual void addAIPlayer();
    int  currPlayer() const     { return m_currPlayer; }

    /**
     * Game handling
     */
    virtual int start() = 0;
    virtual int play(const string &iCoord, const string &iWord) = 0;
    virtual int endTurn() = 0;

protected:
    /// All the players, indexed by their ID
    vector<Player*> m_players;
    /// ID of the "current" player
    int m_currPlayer;

// TODO: check what should be private and what should be protected
// private:

    /// Variant
    GameVariant m_variant;

    /// Dictionary currently associated to the game
    const Dictionary * m_dic;

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

    int helperPlayRound(const Round &iRound);
    int helperSetRackRandom(int p, bool iCheck, set_rack_mode mode);
    int helperSetRackManual(int p, bool iCheck, const string &iLetters);

    void prevPlayer();
    void nextPlayer();
    bool rackInBag(const Rack &iRack, const Bag &iBag) const;
    void realBag(Bag &iBag) const;
    int  checkPlayedWord(const string &iCoord,
                         const string &iWord, Round &oRound);
};

#endif /* _GAME_H_ */
