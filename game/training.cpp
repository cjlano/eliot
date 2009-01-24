/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2009 Antoine Fraboulet & Olivier Teulière
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

#include "training.h"
#include "dic.h"
#include "tile.h"
#include "settings.h"
#include "rack.h"
#include "round.h"
#include "move.h"
#include "pldrack.h"
#include "player.h"
#include "player_move_cmd.h"
#include "player_rack_cmd.h"
#include "game_move_cmd.h"
#include "encoding.h"

#include "debug.h"


Training::Training(const Dictionary &iDic)
    : Game(iDic), m_results(1000)
{
    // Training mode implicitly uses 1 human player
    Game::addPlayer(new HumanPlayer);
    m_players[0]->setName(convertToWc(_("Training")));
}


void Training::setRackRandom(bool iCheck, set_rack_mode mode)
{
    m_results.clear();
    const PlayedRack &newRack =
        helperSetRackRandom(getCurrentPlayer().getCurrentRack(), iCheck, mode);
    Command *pCmd = new PlayerRackCmd(*m_players[m_currPlayer], newRack);
    pCmd->setAutoExecution(false);
    accessNavigation().addAndExecute(pCmd);
}


void Training::setRackManual(bool iCheck, const wstring &iLetters)
{
    // Letters can be lowercase or uppercase as they are
    // coming from user input. We do not consider a lowercase
    // letter to be a joker which has been assigned to a letter.
    // As a result, we simply make all the letters uppercase
    wstring upperLetters = iLetters;
    std::transform(upperLetters.begin(), upperLetters.end(),
                   upperLetters.begin(), towupper);
    const PlayedRack &newRack = helperSetRackManual(iCheck, upperLetters);
    Command *pCmd = new PlayerRackCmd(*m_players[m_currPlayer], newRack);
    pCmd->setAutoExecution(false);
    accessNavigation().addAndExecute(pCmd);
    // Clear the results if everything went well
    m_results.clear();
}


int Training::play(const wstring &iCoord, const wstring &iWord)
{
    // Perform all the validity checks, and fill a round
    Round round;

    int res = checkPlayedWord(iCoord, iWord, round);
    if (res != 0)
    {
        return res;
    }

    Move move(round);
    recordPlayerMove(move, *m_players[m_currPlayer]);

    // Next turn
    endTurn();

    return 0;
}


void Training::recordPlayerMove(const Move &iMove, Player &ioPlayer)
{
    // Update the rack and the score of the current player
    // PlayerMoveCmd::execute() must be called before Game::helperPlayMove()
    // (called in this class in endTurn()).
    // See the big comment in game.cpp, line 96
    Command *pCmd = new PlayerMoveCmd(ioPlayer, iMove);
    pCmd->setAutoExecution(false);
    accessNavigation().addAndExecute(pCmd);
}


void Training::start()
{
    firstPlayer();
    // Dummy new turn, because the navigation prevents undoing the first turn.
    // Since in this mode the player can set the rack, we cannot do like in the
    // duplicate and free game modes, where we change turn just before a move
    // is played...
    accessNavigation().newTurn();
}


void Training::endTurn()
{
    m_results.clear();

    // Play the word on the board
    const Move &move = m_players[m_currPlayer]->getLastMove();
    Command *pCmd = new GameMoveCmd(*this, move,
                                    getCurrentPlayer().getLastRack(),
                                    m_currPlayer);
    accessNavigation().addAndExecute(pCmd);
    accessNavigation().newTurn();
}


void Training::search()
{
    // Search for the current player
    Rack r;
    m_players[m_currPlayer]->getCurrentRack().getRack(r);
    int limit = Settings::Instance().getInt("training.search-limit");
    m_results.setLimit(limit);
    m_results.search(getDic(), getBoard(), r, getHistory().beforeFirstRound());
}


int Training::playResult(unsigned int n)
{
    if (n >= m_results.size())
        return 2;

    Move move(m_results.get(n));
    // Update the rack and the score of the current player
    recordPlayerMove(move, *m_players[m_currPlayer]);

    // Next turn
    endTurn();

    return 0;
}


void Training::addPlayer(Player *iPlayer)
{
    // Override the default behaviour to do nothing
    // except releasing memory
    delete iPlayer;
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

