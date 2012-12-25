/*****************************************************************************
 * Eliot
 * Copyright (C) 2012 Olivier Teulière
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

#include "config.h"
#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
#else
#   define _(String) String
#endif

#include "topping.h"
#include "dic.h"
#include "tile.h"
#include "settings.h"
#include "rack.h"
#include "round.h"
#include "results.h"
#include "move.h"
#include "pldrack.h"
#include "player.h"
#include "cmd/player_move_cmd.h"
#include "cmd/player_rack_cmd.h"
#include "cmd/game_move_cmd.h"
#include "cmd/game_rack_cmd.h"
#include "encoding.h"

#include "debug.h"


INIT_LOGGER(game, Topping);


Topping::Topping(const GameParams &iParams)
    : Game(iParams)
{
}


void Topping::start()
{
    ASSERT(getNPlayers(), "Cannot start a game without any player");

    // Complete the rack
    try
    {
        const PlayedRack &newRack =
            helperSetRackRandom(getHistory().getCurrentRack(), true, RACK_NEW);
        setGameAndPlayersRack(newRack);
    }
    catch (EndGameException &e)
    {
        endGame();
        return;
    }
}


int Topping::play(const wstring &iCoord, const wstring &iWord)
{
    m_board.removeTestRound();

    // Perform all the validity checks, and fill a move
    Move move;
    int res = checkPlayedWord(iCoord, iWord, move);
    if (res != 0)
        return res;

    // Find the best score
    BestResults results;
    results.search(getDic(), getBoard(), m_players[0]->getCurrentRack().getRack(),
                   getHistory().beforeFirstRound());
    if (results.size() == 0)
    {
        // Just to be safe
        endGame();
        return 0;
    }
    int bestScore = results.get(0).getPoints();
    LOG_DEBUG("Top score to be found: " << bestScore);

    if (move.getScore() < bestScore)
    {
        // The player hasn't found the top yet
        LOG_INFO("Player plays a subtop move: " << lfw(move.toString()));
        // TODO
    }
    else
    {
        // End the turn
        recordPlayerMove(move, *m_players[m_currPlayer]);

        // Next turn
        endTurn();
    }

    return 0;
}


void Topping::recordPlayerMove(const Move &iMove, Player &ioPlayer)
{
    // FIXME: the score of the player should not be the score of the move in topping mode
    LOG_INFO("Player " << ioPlayer.getId() << " plays: " << lfw(iMove.toString()));
    // Update the rack and the score of the current player
    // PlayerMoveCmd::execute() must be called before Game::helperPlayMove()
    // (called in this class in endTurn()).
    // See the big comment in game.cpp, line 96
    Command *pCmd = new PlayerMoveCmd(ioPlayer, iMove);
    accessNavigation().addAndExecute(pCmd);
}


bool Topping::isFinished() const
{
    return !canDrawRack(m_players[0]->getHistory().getCurrentRack(), true);
}


void Topping::endTurn()
{
    // Play the word on the board
    const Move &move = m_players[m_currPlayer]->getLastMove();
    Command *pCmd = new GameMoveCmd(*this, move, m_currPlayer);
    accessNavigation().addAndExecute(pCmd);
    accessNavigation().newTurn();

    // Start next turn...
    start();
}


void Topping::endGame()
{
    LOG_INFO("End of the game");
}


void Topping::addPlayer(Player *iPlayer)
{
    ASSERT(getNPlayers() == 0,
           "Only one player can be added in Topping mode");
    // Force the name of the player
    iPlayer->setName(wfl(_("Topping")));
    Game::addPlayer(iPlayer);
}

