/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: game.h,v 1.4 2005/02/09 22:33:56 ipkiss Exp $
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

#ifndef _GAME_H_
#define _GAME_H_

#include <string>
#include <vector>
#include <iostream>
#include "bag.h"
#include "board.h"

class Player;
class PlayedRack;
class Round;
class Rack;
typedef struct _Dictionary * Dictionary;

using namespace std;


/*************************
 * Ident string used to identify saved Eliot games
 *************************/
#define IDENT_STRING "Eliot"

/*************************
 * Dimensions of the board, the tiles placed on
 * the board can be accessed via Game_getboardchar()
 *************************/
#define BOARD_MIN 1
#define BOARD_MAX 15

/*************************
 * Dimensions of the strings that are used to return
 * values to the GUI
 *************************/
#define WORD_SIZE_MAX 16
#define RACK_SIZE_MAX 10
#define COOR_SIZE_MAX  6

// typedef struct tgame* Game;

class Game
{
public:
    /*************************
     * Functions to create and destroy a game
     * the dictionary does not belong to the
     * game (ie: it won't be destroyed by ~Game)
     *
     * The dictionary can be changed afterwards by setDic
     *************************/
    Game(const Dictionary &iDic);
    virtual ~Game();

    /*************************
     * Handle game mode
     *************************/
    enum GameMode
    {
        kTRAINING,
        kFREEGAME,
        kDUPLICATE
    };
    virtual GameMode getMode() const = 0;
    virtual string getModeAsString() const = 0;

    /*************************
     * handling games
     * init() will set up a new (empty) game
     *
     * load() returns the loaded game, or NULL if there was a problem
     * load() might need some more work to be robust enough to
     * handle "hand written" files
     *************************/

    void init();
    static Game * load(FILE *fin, const Dictionary &iDic);
    void save(ostream &out) const;

    /*************************
    * Dictionary associated with the game
    * The dictionary can be changed during a
    * game without problem
    *************************/
    const Dictionary & getDic() const   { return *m_dic; }
    void setDic(const Dictionary &iDic) { m_dic = &iDic; }

    /*************************
     * Playing the game
     * the int parameter should be 0 <= int < getNRounds
     *
     * testplay will place a temporary word on the board for
     * preview purpose
     * return value is
     *  0 : ok
     *  1 : dictionary is set to NULL
     *  2 : not enough played rounds for the request
     *************************/
    int back(int);
    int testPlay(int);
    int removeTestPlay();

    /*************************
     * int coordinates have to be
     * BOARD_MIN <= int <= BOARD_MAX
     *
     * getBoardChar returns an upper case letter
     * for normal tiles and a lower case letter for jokers.
     *
     * getBoardCharAttr tells the attributes of the tile
     *   0 : normal played tile
     *   1 : joker tile
     *   2 : test tile for preview purpose
     * attributes can be combined with the or (|) operator
     *************************/
#define ATTR_NORMAL 0
#define ATTR_JOKER  1
#define ATTR_TEST   2

    char getBoardChar    (int iRow, int iCol) const;
    int  getBoardCharAttr(int iRow, int iCol) const;

    int  getBoardWordMultiplier  (int iRow, int iCol) const;
    int  getBoardLetterMultiplier(int iRow, int iCol) const;

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
#define RACK_MAX 7
    typedef enum {RACK_ALL, RACK_NEW} set_rack_mode;

    /*************************
     * Get the number of tiles available in the bag.
     * The parameter has to be
     * 'a' <= char <= 'z' or 'A' <= char <= 'Z' or '?'
     *************************/
    int getNCharInBag(char) const;

    /*************************
     * Functions to access already played words
     * The int parameter should be 0 <= int < getnrounds
     *************************/
    int getNRounds() const     { return m_roundHistory.size(); }
    string getPlayedRack(int) const;
    string getPlayedWord(int) const;
    string getPlayedCoords(int num) const;
    int getPlayedPoints(int) const;
    int getPlayedBonus(int) const;
    int getPlayedPlayer(int) const;

    /*************************
     * Functions to access the current search results
     * The int parameter should be 0 <= int < getNResults
     *************************/
    int getNResults() const;
    string getSearchedWord(int) const;
    string getSearchedCoords(int) const;
    int getSearchedPoints(int) const;
    int getSearchedBonus (int) const;

    /*************************
     * Functions to access players.
     * The int parameter should be 0 <= int < getNPlayers
     *************************/
    int  getNPlayers() const    { return m_players.size(); }
    int  getNHumanPlayers() const;
    void addHumanPlayer();
    void addAIPlayer();
    int  getPlayerPoints(int) const;
    string getPlayerRack(int) const;

    int  currPlayer() const     { return m_currPlayer; }
    // Return the previous human player who has not played yet
    void prevHumanPlayer();
    // Return the next human player who has not played yet
    void nextHumanPlayer();

    /*************************
     * Game handling
     *************************/
    virtual int start() = 0;
    virtual int setRackRandom(int, bool, set_rack_mode) = 0;
    virtual int play(const string &iCoord, const string &iWord) = 0;
    virtual int endTurn() = 0;

    /*************************
     * Display methods
     *************************/
    void printBoard(ostream &out) const;
    void printBoardJoker(ostream &out) const;
//     void printBoardPoint(ostream &out) const;
    void printBoardMultipliers(ostream &out) const;
    void printBoardMultipliers2(ostream &out) const;
    void printNonPlayed(ostream &out) const;
    void printPlayedRack(ostream &out, int n) const;
    void printAllRacks(ostream &out) const;
    void printSearchResults(ostream &out, int) const;
    void printPoints(ostream &out) const;
    void printAllPoints(ostream &out) const;


protected:
    int helperPlayRound(const Round &iRound);
    int helperSetRackRandom(int p, bool iCheck, set_rack_mode mode);
    int helperSetRackManual(int p, bool iCheck, const string &iLetters);

    /* All the players, indexed by their ID */
    vector<Player*> m_players;
    int m_currPlayer;

// private:

    const Dictionary * m_dic;

    Bag m_bag;
    Board m_board;

    /*************************
     * History of the game
     * All the vectors are indexed by the number of turns in the game
     *************************/
    /* History of the racks */
    vector<PlayedRack*> m_rackHistory;
    /* History of the rounds */
    vector<Round*> m_roundHistory;
    /* ID of the players that played the round for each turn */
    vector<int> m_playerHistory;

    int m_points;

    bool m_finished;

    /*********************************************************
     * Helper functions
     *********************************************************/

    string formatCoords(const Round &iRound) const;
    string formatPlayedRack(const PlayedRack &iRack,
                            bool showExtraSigns = true) const;
    void prevPlayer();
    void nextPlayer();
    bool rackInBag(const Rack &iRack, const Bag &iBag) const;
    void realBag(Bag &iBag) const;
    int  checkPlayedWord(const string &iCoord,
                         const string &iWord, Round &oRound);
};

#endif /* _GAME_H_ */
