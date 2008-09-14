/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2007 Olivier Teulière
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
#include "dic.h"
#include "tile.h"
#include "rack.h"
#include "round.h"
#include "move.h"
#include "pldrack.h"
#include "results.h"
#include "player.h"
#include "ai_player.h"
#include "settings.h"
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
    if (res != 0 && Settings::Instance().getBool("duplicate-reject-invalid"))
    {
        return res;
    }

    // If we reach this point, either the move is valid and we can use the
    // "round" variable, or it is invalid but played nevertheless
    if (res == 0)
    {
        // Everything is OK, we can play the word
        playMove(Move(round), m_currPlayer);
    }
    else
    {
        // Record the invalid move of the player
        playMove(Move(iWord, iCoord), m_currPlayer);
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

    player->compute(m_dic, m_board, m_history.beforeFirstRound());
    const Move move = player->getMove();
    if (move.getType() == Move::CHANGE_LETTERS ||
        move.getType() == Move::PASS)
    {
        // The AI player must be buggy...
        ASSERT(false, "AI tried to cheat!");
    }

    playMove(move, p);
}


int Duplicate::start()
{
    ASSERT(getNPlayers(), "Cannot start a game without any player");

    // Arbitrary player, since they should all have the same rack
    m_currPlayer = 0;

    // Complete the rack for the player that just played
    int res = helperSetRackRandom(m_currPlayer, true, RACK_NEW);
    // End of the game?
    if (res == 1)
    {
        endGame();
        return 1;
    }

    const PlayedRack& pld = m_players[m_currPlayer]->getCurrentRack();
    // All the players have the same rack
    for (unsigned int i = 0; i < getNPlayers(); i++)
    {
        if (i != m_currPlayer)
        {
            m_players[i]->setCurrentRack(pld);
        }
        // Nobody has played yet in this round
        m_hasPlayed[i] = false;
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


void Duplicate::playMove(const Move &iMove, unsigned int p)
{
    ASSERT(p < getNPlayers(), "Wrong player number");

    // Update the rack and the score of the playing player
    m_players[p]->endTurn(iMove, m_history.getSize());

    m_hasPlayed[p] = true;
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

    // Handle solo bonus
    // First check whether there are enough players in the game for the
    // bonus to apply
    int minNbPlayers = Settings::Instance().getInt("duplicate-solo-players");
    if (getNPlayers() >= (unsigned int)minNbPlayers &&
        m_players[imax]->getLastMove().getType() == Move::VALID_ROUND)
    {
        int maxScore = m_players[imax]->getLastMove().getScore();
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
            int bonus = Settings::Instance().getInt("duplicate-solo-value");
            m_players[imax]->addPoints(bonus);
            // TODO: keep a trace of the solo, so the interface
            // can be aware of it...
        }
    }

    // Play the best word on the board
    helperPlayMove(imax, m_players[imax]->getLastMove());

    // Leave the same reliquate to all players
    // This is required by the start() method which will be called to
    // start the next turn
    const PlayedRack& pld = m_players[imax]->getCurrentRack();
    for (unsigned int i = 0; i < getNPlayers(); i++)
    {
        if (i != imax)
        {
            m_players[i]->setCurrentRack(pld);
        }
    }

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

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
