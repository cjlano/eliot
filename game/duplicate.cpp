/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2009 Olivier Teulière
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

#include <boost/foreach.hpp>
#include <sstream>

#include "config.h"
#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
#else
#   define _(String) String
#endif

#include "duplicate.h"
#include "game_exception.h"
#include "dic.h"
#include "tile.h"
#include "rack.h"
#include "round.h"
#include "move.h"
#include "pldrack.h"
#include "results.h"
#include "player.h"
#include "player_move_cmd.h"
#include "player_rack_cmd.h"
#include "game_move_cmd.h"
#include "mark_played_cmd.h"
#include "ai_player.h"
#include "settings.h"
#include "encoding.h"
#include "debug.h"


INIT_LOGGER(game, Duplicate);


Duplicate::Duplicate(const Dictionary &iDic)
    : Game(iDic)
{
}


int Duplicate::play(const wstring &iCoord, const wstring &iWord)
{
    // Perform all the validity checks, and try to fill a round
    Round round;
    int res = checkPlayedWord(iCoord, iWord, round);
    if (res != 0 && Settings::Instance().getBool("duplicate.reject-invalid"))
    {
        return res;
    }

    // If we reach this point, either the move is valid and we can use the
    // "round" variable, or it is invalid but played nevertheless
    Player &currPlayer = *m_players[m_currPlayer];
    if (res == 0)
    {
        // Everything is OK, we can play the word
        recordPlayerMove(Move(round), currPlayer, true);
    }
    else
    {
        // Convert the invalid word for display
        const wdstring &dispWord = getDic().convertToDisplay(iWord);
        // Record the invalid move of the player
        recordPlayerMove(Move(dispWord, iCoord), currPlayer, true);
    }

    // Little hack to handle duplicate games with only AI players.
    // This will have no effect when there is at least one human player
    tryEndTurn();

    return 0;
}


void Duplicate::playAI(unsigned int p)
{
    ASSERT(p < getNPlayers(), "Wrong player number");

    AIPlayer *player = dynamic_cast<AIPlayer*>(m_players[p]);
    ASSERT(player != NULL, "AI requested for a human player");

    player->compute(getDic(), getBoard(), getHistory().beforeFirstRound());
    const Move &move = player->getMove();
    if (move.getType() == Move::CHANGE_LETTERS ||
        move.getType() == Move::PASS)
    {
        // The AI player must be buggy...
        ASSERT(false, "AI tried to cheat!");
    }

    recordPlayerMove(move, *player, false);
}


void Duplicate::start()
{
    ASSERT(getNPlayers(), "Cannot start a game without any player");

    // Arbitrary player, since they should all have the same rack
    m_currPlayer = 0;

    // Complete the rack for the player that just played
    try
    {
        const PlayedRack &newRack =
            helperSetRackRandom(getCurrentPlayer().getCurrentRack(), true, RACK_NEW);
        // All the players have the same rack
        BOOST_FOREACH(Player *player, m_players)
        {
            Command *pCmd = new PlayerRackCmd(*player, newRack);
            accessNavigation().addAndExecute(pCmd);
            // Nobody has played yet in this round
            Command *pCmd2 = new MarkPlayedCmd(*this, player->getId(), false);
            accessNavigation().addAndExecute(pCmd2);
        }
        // Change the turn _after_ setting the new rack, so that when going
        // back in the history the rack is already there. The turn boundaries
        // must be just before player actions, otherwise restoring the game
        // doesn't work properly.
        accessNavigation().newTurn();
    }
    catch (EndGameException &e)
    {
        endGame();
        return;
    }

    // Little hack to handle duplicate games with only AI players.
    // This will have no effect when there is at least one human player
    tryEndTurn();
}


void Duplicate::tryEndTurn()
{
    for (unsigned int i = 0; i < getNPlayers(); i++)
    {
        if (m_players[i]->isHuman() && !m_hasPlayed[i])
        {
            // A human player has not played...
            m_currPlayer = i;
            // So we don't finish the turn
            return;
        }
    }

    // Now that all the human players have played,
    // make AI players play their turn
    for (unsigned int i = 0; i < getNPlayers(); i++)
    {
        if (!m_players[i]->isHuman())
        {
            playAI(i);
        }
    }

    // Next turn
    endTurn();
}


void Duplicate::recordPlayerMove(const Move &iMove, Player &ioPlayer, bool isForHuman)
{
    LOG_INFO("Player " << ioPlayer.getId() << " plays: " << convertToMb(iMove.toString()));
    Command *pCmd = new PlayerMoveCmd(ioPlayer, iMove);
    pCmd->setAutoExecution(!isForHuman);
    accessNavigation().addAndExecute(pCmd);

    Command *pCmd2 = new MarkPlayedCmd(*this, ioPlayer.getId(), true);
    pCmd2->setAutoExecution(!isForHuman);
    accessNavigation().addAndExecute(pCmd2);
}


void Duplicate::endTurn()
{
    // Find the player with the best score
    Player *bestPlayer = NULL;
    int bestScore = 0;
    BOOST_FOREACH(Player *player, m_players)
    {
        if (player->getLastMove().getScore() > bestScore)
        {
            bestScore = player->getLastMove().getScore();
            bestPlayer = player;
        }
    }

    // If nobody played a valid round, go to the next turn
    if (bestPlayer == NULL)
    {
        // In fact, maybe someone played a valid round with a score of 0
        // (just playing a joker, for example)
        BOOST_FOREACH(Player *player, m_players)
        {
            if (player->getLastMove().getType() == Move::VALID_ROUND)
            {
                bestPlayer = player;
                break;
            }
        }
        if (bestPlayer == NULL)
        {
            // Nobody played a valid round. Go to the next turn...
            start();
            return;
        }
    }

    // Get the best valid move
    const Move &bestMove = bestPlayer->getLastMove();

    // Handle solo bonus
    // First check whether there are enough players in the game for the
    // bonus to apply
    unsigned int minNbPlayers = Settings::Instance().getInt("duplicate.solo-players");
    if (getNPlayers() >= minNbPlayers &&
        bestMove.getType() == Move::VALID_ROUND)
    {
        // Find whether other players than imax have the same score
        bool otherWithSameScore = false;
        BOOST_FOREACH(const Player *player, m_players)
        {
            if (player != bestPlayer &&
                player->getLastMove().getScore() >= bestScore &&
                player->getLastMove().getType() == Move::VALID_ROUND)
            {
                otherWithSameScore = true;
                break;
            }
        }
        if (!otherWithSameScore)
        {
            // Give the bonus to the player of the best move
            int bonus = Settings::Instance().getInt("duplicate.solo-value");
            bestPlayer->addPoints(bonus);
            // TODO: keep a trace of the solo, so the interface
            // can be aware of it...
        }
    }

    // Play the best word on the board
    Command *pCmd = new GameMoveCmd(*this, bestMove, bestPlayer->getId());
    accessNavigation().addAndExecute(pCmd);

    // Leave the same reliquate to all players
    // This is required by the start() method which will be called to
    // start the next turn
    const PlayedRack& pld = bestPlayer->getCurrentRack();
    BOOST_FOREACH(Player *player, m_players)
    {
        if (player != bestPlayer)
        {
            Command *pCmd = new PlayerRackCmd(*player, pld);
            accessNavigation().addAndExecute(pCmd);
        }
    }

    // Start next turn...
    start();
}


void Duplicate::endGame()
{
    LOG_INFO("End of the game");
    m_finished = true;
}


void Duplicate::setPlayer(unsigned int p)
{
    ASSERT(p < getNPlayers(), "Wrong player number");

    // Forbid switching to an AI player
    if (!m_players[p]->isHuman())
        throw GameException(_("Cannot switch to a non-human player"));

    // Forbid switching back to a player who has already played
    if (hasPlayed(p))
        throw GameException(_("Cannot switch to a player who has already played"));

    m_currPlayer = p;
}


bool Duplicate::hasPlayed(unsigned int p) const
{
    ASSERT(p < getNPlayers(), "Wrong player number");

    map<unsigned int, bool>::const_iterator it = m_hasPlayed.find(p);
    return it != m_hasPlayed.end() && it->second;
}


void Duplicate::setPlayedFlag(unsigned int iPlayerId, bool iNewFlag)
{
    ASSERT(iPlayerId < getNPlayers(), "Wrong player number");

    m_hasPlayed[iPlayerId] = iNewFlag;
}

