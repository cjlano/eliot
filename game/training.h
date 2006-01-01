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

#ifndef _TRAINING_H_
#define _TRAINING_H_

#include <string>

#include "game.h"
#include "results.h"

using std::string;


/**
 * This class handles the logic specific to a training game.
 * As its name indicates, it is not a game in the literal meaning of the word,
 * in particular because the rack can be set at will.
 * Note: No player should be added to this game, a human player is added
 * automatically (in the start() method)
 */
class Training: public Game
{
    friend class GameFactory;
public:
    virtual GameMode getMode() const { return kTRAINING; }
    virtual string getModeAsString() const { return "Training"; }

    /*************************
     * Game handling
     *************************/
    virtual int start();
    virtual int play(const string &iCoord, const string &iWord);
    virtual int endTurn();
    void search();
    int playResult(int);

    int setRackRandom(bool, set_rack_mode);
    int setRackManual(bool iCheck, const string &iLetters);
    int setRack(set_rack_mode iMode, bool iCheck, const string &iLetters);

    /*************************
     * Override the default behaviour of these methods, because in training
     * we only want a human player
     *************************/
    virtual void addHumanPlayer();
    virtual void addAIPlayer();

    /*************************
     * Functions to access the current search results
     * The int parameter should be 0 <= int < getNResults
     *************************/
    const Results& getResults() const { return m_results; };

    /// Place a temporary word on the board for preview purpose
    void testPlay(int);
    /// Remove the temporary word(s)
    void removeTestPlay();
    /// Get the temporary word
    string getTestPlayWord() const;

private:
    // Private constructor and destructor to force using the GameFactory class
    Training(const Dictionary &iDic);
    virtual ~Training();

    // Search results, with all the possible rounds
    Round   m_testRound;
    Results m_results;
};

#endif /* _TRAINING_H_ */

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
