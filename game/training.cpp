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
#include <cwctype> // For towupper

#include "config.h"
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
#include "game_rack_cmd.h"
#include "encoding.h"

#include "debug.h"


INIT_LOGGER(game, Training);


Training::Training(const GameParams &iParams)
    : Game(iParams), m_results(1000)
{
}


void Training::setRackRandom(bool iCheck, set_rack_mode mode)
{
    m_results.clear();
    const PlayedRack &newRack =
        helperSetRackRandom(getHistory().getCurrentRack(), iCheck, mode);
    Command *pCmd1 = new GameRackCmd(*this, newRack);
    pCmd1->setHumanIndependent(false);
    accessNavigation().addAndExecute(pCmd1);
    Command *pCmd2 = new PlayerRackCmd(*m_players[m_currPlayer], newRack);
    pCmd2->setHumanIndependent(false);
    accessNavigation().addAndExecute(pCmd2);
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
    Command *pCmd1 = new GameRackCmd(*this, newRack);
    pCmd1->setHumanIndependent(false);
    accessNavigation().addAndExecute(pCmd1);
    Command *pCmd2 = new PlayerRackCmd(*m_players[m_currPlayer], newRack);
    pCmd2->setHumanIndependent(false);
    accessNavigation().addAndExecute(pCmd2);
    // Clear the results if everything went well
    m_results.clear();
}


int Training::play(const wstring &iCoord, const wstring &iWord)
{
    // Perform all the validity checks, and fill a round
    Round round;

    m_board.removeTestRound();

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
    LOG_INFO("Player " << ioPlayer.getId() << " plays: " << lfw(iMove.toString()));
    // Update the rack and the score of the current player
    // PlayerMoveCmd::execute() must be called before Game::helperPlayMove()
    // (called in this class in endTurn()).
    // See the big comment in game.cpp, line 96
    Command *pCmd = new PlayerMoveCmd(ioPlayer, iMove);
    pCmd->setHumanIndependent(false);
    accessNavigation().addAndExecute(pCmd);
}


void Training::start()
{
    firstPlayer();
}


bool Training::isFinished() const
{
    // The game is finished when there is no more tile
    // (in the bag or in the racks)
    return getBag().getNbTiles() == 0;
}


void Training::endTurn()
{
    m_results.clear();

    // Play the word on the board
    const Move &move = m_players[m_currPlayer]->getLastMove();
    Command *pCmd = new GameMoveCmd(*this, move, m_currPlayer);
    accessNavigation().addAndExecute(pCmd);
    accessNavigation().newTurn();
}


void Training::search()
{
    // Search for the current player
    const Rack &rack = getHistory().getCurrentRack().getRack();
    int limit = Settings::Instance().getInt("training.search-limit");
    m_results.setLimit(limit);
    m_results.search(getDic(), getBoard(), rack, getHistory().beforeFirstRound());
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
    ASSERT(getNPlayers() == 0,
           "Only one player can be added in Training mode");
    // Force the name of the player
    iPlayer->setName(wfl(_("Training")));
    Game::addPlayer(iPlayer);
}

