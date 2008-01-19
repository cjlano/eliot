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

#include <algorithm>

#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
#else
#   define _(String) String
#endif

#include "dic.h"
#include "tile.h"
#include "rack.h"
#include "round.h"
#include "move.h"
#include "pldrack.h"
#include "player.h"
#include "training.h"
#include "encoding.h"

#include "debug.h"


Training::Training(const Dictionary &iDic)
    : Game(iDic)
{
    // Training mode implicitly uses 1 human player
    Game::addHumanPlayer();
    m_players[0]->setName(convertToWc(_("Training")));
}


int Training::setRackRandom(bool iCheck, set_rack_mode mode)
{
    m_results.clear();
    return helperSetRackRandom(m_currPlayer, iCheck, mode);
}


int Training::setRackManual(bool iCheck, const wstring &iLetters)
{
    // Letters can be lowercase or uppercase as they are
    // coming from user input. We do not consider a lowercase
    // letter to be a joker which has been assigned to a letter.
    // As a result, we simply make all the letters uppercase
    wstring upperLetters = iLetters;
    std::transform(upperLetters.begin(), upperLetters.end(),
                   upperLetters.begin(), towupper);
    int res = helperSetRackManual(m_currPlayer, iCheck, upperLetters);
    // 0: ok
    // 1: not enough tiles
    // 2: check failed (number of vowels before round 15)
    // 3: letters not in the dictionary
    if (res == 0)
        m_results.clear();
    return res;
}


int Training::setRack(set_rack_mode iMode, bool iCheck, const wstring &iLetters)
{
    int res = 0;
    switch(iMode)
    {
        case RACK_MANUAL:
            res = setRackManual(iCheck, iLetters);
            break;
        case RACK_ALL:
            res = setRackRandom(iCheck, iMode);
            break;
        case RACK_NEW:
            res = setRackRandom(iCheck, iMode);
            break;
    }
    return res;
}


int Training::play(const wstring &iCoord, const wstring &iWord)
{
    // Perform all the validity checks, and fill a round
    Round round;

    int res = checkPlayedWord(iCoord, iWord, round);
    if (res != 0)
    {
        debug("check returned with an error %d\n",res);
        return res;
    }

    debug("play: %s %s %d\n",
          convertToMb(round.getWord()).c_str(),
          convertToMb(round.getCoord().toString()).c_str(),
          round.getPoints());

    Move move(round);
    // Update the rack and the score of the current player
    // Player::endTurn() must be called before Game::helperPlayMove().
    // See the big comment in game.cpp, line 96
    m_players[m_currPlayer]->endTurn(move, m_history.getSize());

    // Everything is OK, we can play the word
    helperPlayMove(m_currPlayer, move);

    // Next turn
    endTurn();

    return 0;
}


int Training::start()
{
    if (getNPlayers() != 0)
        return 1;

    m_currPlayer = 0;
    return 0;
}


void Training::endTurn()
{
    // Nothing to do, but this method is kept for consistency with other modes
}


void Training::search()
{
    // Search for the current player
    Rack r;
    m_players[m_currPlayer]->getCurrentRack().getRack(r);
    debug("Training::search for %s\n", convertToMb(r.toString()).c_str());
    m_results.search(m_dic, m_board, r, m_history.beforeFirstRound());
}


int Training::playResult(unsigned int n)
{
    if (n >= m_results.size())
        return 2;

    Move move(m_results.get(n));
    // Update the rack and the score of the current player
    m_players[m_currPlayer]->endTurn(move, m_history.getSize());

    // Update the game
    helperPlayMove(m_currPlayer, move);
    m_results.clear();

    // Next turn
    endTurn();

    return 0;
}


void Training::addHumanPlayer()
{
    // We are not supposed to be here...
    ASSERT(false, "Trying to add a human player in Training mode");
}


void Training::addAIPlayer()
{
    // We are not supposed to be here...
    ASSERT(false, "Trying to add a AI player in Training mode");
}


void Training::testPlay(unsigned int num)
{
    ASSERT(num < m_results.size(), "Wrong result number");
    m_testRound = m_results.get(num);
    m_board.testRound(m_results.get(num));
}


void Training::removeTestPlay()
{
    m_board.removeTestRound();
    m_testRound = Round();
}


wstring Training::getTestPlayWord() const
{
    return m_testRound.getWord();
}

/****************************************************************/
/****************************************************************/

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
