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

#ifndef _TRAINING_H_
#define _TRAINING_H_

#include <string>

#include "game.h"
#include "round.h"
#include "results.h"

using std::string;
using std::wstring;


/**
 * This class handles the logic specific to a training game.
 * As its name indicates, it is not a game in the literal meaning of the word,
 * in particular because the rack can be set at will.
 *
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

    /// See description of Game::play()
    virtual int play(const wstring &iCoord, const wstring &iWord);

    void search();
    const Results& getResults() const { return m_results; };
    int playResult(unsigned int);

    int setRackRandom(bool, set_rack_mode);
    int setRackManual(bool iCheck, const wstring &iLetters);
    int setRack(set_rack_mode iMode, bool iCheck, const wstring &iLetters);

    /*************************
     * Override the default behaviour of addPlayer(), because in training
     * mode we only want a human player
     *************************/
    virtual void addPlayer(Player *iPlayer);

    /*************************
     * Functions to access the current search results
     * The int parameter should be 0 <= int < getNResults
     *************************/

    /// Place a temporary word on the board for preview purposes
    void testPlay(unsigned int);
    /// Remove the temporary word
    void removeTestPlay();
    /// Get the temporary word
    wstring getTestPlayWord() const;

private:
    /// Private constructor and destructor to force using the GameFactory class
    Training(const Dictionary &iDic);

    void endTurn();

    /// Search results, with all the possible rounds
    Results m_results;

    /// Round corresponding to the last test play (if any)
    Round m_testRound;
};

#endif /* _TRAINING_H_ */

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
