/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2008 Olivier Teulière
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
#include "ai_player.h"
#include "settings.h"
#include "turn_cmd.h"
#include "debug.h"


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
        recordPlayerMove(Move(round), currPlayer);
    }
    else
    {
        // Record the invalid move of the player
        recordPlayerMove(Move(iWord, iCoord), currPlayer);
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

    recordPlayerMove(move, *player);
}


int Duplicate::start()
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
        for (unsigned int i = 0; i < getNPlayers(); i++)
        {
            Command *pCmd = new PlayerRackCmd(*m_players[i], newRack);
            m_turnCommands[m_currTurn]->addAndExecute(pCmd);
            // Nobody has played yet in this round
            m_hasPlayed[i] = false;
        }
    }
    catch (EndGameException &e)
    {
        endGame();
        return 1;
    }

    // Little hack to handle duplicate games with only AI players.
    // This will have no effect when there is at least one human player
    tryEndTurn();

    return 0;
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


void Duplicate::recordPlayerMove(const Move &iMove, Player &ioPlayer)
{
    Command *pCmd = new PlayerMoveCmd(ioPlayer, iMove);
    m_turnCommands[m_currTurn]->addAndExecute(pCmd);

    m_hasPlayed[ioPlayer.getId()] = true;
}


void Duplicate::endTurn()
{
    // Find the player with the best score
    unsigned int imax = 0;
    for (unsigned int i = 1; i < getNPlayers(); i++)
    {
        if (m_players[i]->getLastMove().getScore() >
            m_players[imax]->getLastMove().getScore())
        {
            imax = i;
        }
    }

    // TODO: do something if nobody played a valid round!

    // Get the best valid move
    const Move &bestMove = m_players[imax]->getLastMove();

    // Handle solo bonus
    // First check whether there are enough players in the game for the
    // bonus to apply
    int minNbPlayers = Settings::Instance().getInt("duplicate.solo-players");
    if (getNPlayers() >= (unsigned int)minNbPlayers &&
        bestMove.getType() == Move::VALID_ROUND)
    {
        int maxScore = bestMove.getScore();
        // Find whether other players than imax have the same score
        bool otherWithSameScore = false;
        for (unsigned int i = imax + 1; i < getNPlayers(); i++)
        {
            if (m_players[i]->getLastMove().getScore() >= maxScore)
            {
                otherWithSameScore = true;
                break;
            }
        }
        if (!otherWithSameScore)
        {
            // Give the bonus to player imax
            int bonus = Settings::Instance().getInt("duplicate.solo-value");
            m_players[imax]->addPoints(bonus);
            // TODO: keep a trace of the solo, so the interface
            // can be aware of it...
        }
    }

    // Play the best word on the board
    Command *pCmd = new GameMoveCmd(*this, bestMove,
                                    getPlayer(imax).getLastRack(), imax);
    m_turnCommands[m_currTurn]->addAndExecute(pCmd);

    // Leave the same reliquate to all players
    // This is required by the start() method which will be called to
    // start the next turn
    const PlayedRack& pld = m_players[imax]->getCurrentRack();
    for (unsigned int i = 0; i < getNPlayers(); i++)
    {
        if (i != imax)
        {
            Command *pCmd = new PlayerRackCmd(*m_players[i], pld);
            m_turnCommands[m_currTurn]->addAndExecute(pCmd);
        }
    }

    newTurn();

    // Start next turn...
    start();
}


void Duplicate::endGame()
{
    m_finished = true;
}


int Duplicate::setPlayer(unsigned int p)
{
    ASSERT(p < getNPlayers(), "Wrong player number");

    // Forbid switching to an AI player
    if (!m_players[p]->isHuman())
        return 1;

    m_currPlayer = p;
    return 0;
}


void Duplicate::prevHumanPlayer()
{
    if (getNHumanPlayers() == 0)
        return;
    // FIXME: possible infinite loop...
    do
    {
        prevPlayer();
    } while (!m_players[m_currPlayer]->isHuman() ||
             m_hasPlayed[m_currPlayer]);
}


void Duplicate::nextHumanPlayer()
{
    if (getNHumanPlayers() == 0)
        return;
    // FIXME: possible infinite loop...
    do
    {
        nextPlayer();
    } while (!m_players[m_currPlayer]->isHuman() ||
             m_hasPlayed[m_currPlayer]);
}


bool Duplicate::hasPlayed(unsigned int p) const
{
    ASSERT(p < getNPlayers(), "Wrong player number");

    map<unsigned int, bool>::const_iterator it = m_hasPlayed.find(p);
    return it != m_hasPlayed.end() && it->second;
}

