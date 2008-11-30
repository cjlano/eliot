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

#include <boost/foreach.hpp>

#include <iomanip>
#include <wctype.h>

#include "freegame.h"
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
#include "turn.h"

#include "debug.h"


FreeGame::FreeGame(const Dictionary &iDic)
    : Game(iDic)
{
}


int FreeGame::play(const wstring &iCoord, const wstring &iWord)
{
    // Perform all the validity checks, and try to fill a round
    Round round;
    int res = checkPlayedWord(iCoord, iWord, round);
    if (res != 0 && Settings::Instance().getBool("freegame.reject-invalid"))
    {
        return res;
    }

    // If we reach this point, either the move is valid and we can use the
    // "round" variable, or it is invalid but played nevertheless
    if (res == 0)
    {
        Move move(round);

        // Update the rack and the score of the current player
        recordPlayerMove(move, *m_players[m_currPlayer], true);
    }
    else
    {
        Move move(iWord, iCoord);

        // Record the invalid move of the player
        recordPlayerMove(move, *m_players[m_currPlayer], true);
    }

    // Next turn
    endTurn();

    return 0;
}


void FreeGame::playAI(unsigned int p)
{
    ASSERT(p < getNPlayers(), "Wrong player number");
    ASSERT(!m_players[p]->isHuman(), "AI requested for a human player");

    AIPlayer *player = static_cast<AIPlayer*>(m_players[p]);

    player->compute(getDic(), getBoard(), getHistory().beforeFirstRound());
    const Move &move = player->getMove();
    if (move.getType() == Move::CHANGE_LETTERS ||
        move.getType() == Move::PASS)
    {
        ASSERT(checkPass(*player, move.getChangedLetters()) == 0,
               "AI tried to cheat!");
    }

    // Update the rack and the score of the current player
    recordPlayerMove(move, *player, false);

    endTurn();
}


void FreeGame::recordPlayerMove(const Move &iMove, Player &ioPlayer,
                                bool isForHuman)
{
    Command *pCmd = new PlayerMoveCmd(ioPlayer, iMove);
    pCmd->setAutoExecution(!isForHuman);
    accessNavigation().addAndExecute(pCmd);
}


void FreeGame::start()
{
    ASSERT(getNPlayers(), "Cannot start a game without any player");

    // Set the initial racks of the players
    for (unsigned int i = 0; i < getNPlayers(); i++)
    {
        const PlayedRack &newRack =
            helperSetRackRandom(getPlayer(i).getCurrentRack(), false, RACK_NEW);
        Command *pCmd = new PlayerRackCmd(*m_players[i], newRack);
        accessNavigation().addAndExecute(pCmd);
    }

    firstPlayer();

    accessNavigation().newTurn();

    // If the first player is an AI, make it play now
    if (!m_players[m_currPlayer]->isHuman())
    {
        playAI(m_currPlayer);
    }
}


int FreeGame::endTurn()
{
    const Move &move = getCurrentPlayer().getLastMove();
    // Update the game
    Command *pCmd = new GameMoveCmd(*this, move,
                                    getCurrentPlayer().getLastRack(),
                                    m_currPlayer);
    accessNavigation().addAndExecute(pCmd);

    // Complete the rack for the player that just played
    if (move.getType() == Move::VALID_ROUND ||
        move.getType() == Move::CHANGE_LETTERS)
    {
        try
        {
            const PlayedRack &newRack =
                helperSetRackRandom(getCurrentPlayer().getCurrentRack(), false, RACK_NEW);
            Command *pCmd = new PlayerRackCmd(*m_players[m_currPlayer],
                                              newRack);
            accessNavigation().addAndExecute(pCmd);
        }
        catch (EndGameException &e)
        {
            // End of the game
            endGame();
            return 1;
        }
    }

    // Next player
    nextPlayer();

    accessNavigation().newTurn();

    // If this player is an AI, make it play now
    if (!getCurrentPlayer().isHuman())
    {
        playAI(m_currPlayer);
    }

    return 0;
}


// Adjust the scores of the players with the points of the remaining tiles
void FreeGame::endGame()
{
    vector<Tile> tiles;

    // TODO: According to the rules of the game in the ODS, a game can end in 3
    // cases:
    // 1) no more letter in the bag, and one player has no letter in his rack
    // 2) the game is "blocked", no one can play
    // 3) the players have used all the time they had (for example: 30 min
    //    in total, for each player)
    // We currently handle case 1, and cannot handle case 3 until timers are
    // implemented.
    // For case 2, we need both to detect a blocked situation (not easy...) and
    // to handle it in the endGame() method (very easy).

    /* Add the points of the remaining tiles to the score of the current
     * player (i.e. the first player with an empty rack), and remove them
     * from the score of the players who still have tiles */
    for (unsigned int i = 0; i < getNPlayers(); i++)
    {
        if (i != m_currPlayer)
        {
            const PlayedRack &pld = m_players[i]->getCurrentRack();
            pld.getAllTiles(tiles);
            BOOST_FOREACH(const Tile &tile, tiles)
            {
                m_players[i]->addPoints(- tile.getPoints());
                m_players[m_currPlayer]->addPoints(tile.getPoints());
            }
        }
    }

    // Lock game
    m_finished = true;
}


int FreeGame::checkPass(const Player &iPlayer,
                        const wstring &iToChange) const
{
    // Check that the game is not finished
    if (m_finished)
        return 3;

    // Check that the letters are valid for the current dictionary
    if (!getDic().validateLetters(iToChange))
        return 4;

    // It is forbidden to change letters when the bag does not contain at
    // least 7 letters (this is explicitly stated in the ODS). But it is
    // still allowed to pass
    Bag bag(getDic());
    realBag(bag);
    if (bag.getNbTiles() < 7 && !iToChange.empty())
    {
        return 1;
    }

    // Check that the letters are all present in the player's rack
    const PlayedRack &pld = iPlayer.getCurrentRack();
    Rack rack;
    pld.getRack(rack);
    BOOST_FOREACH(wchar_t wch, iToChange)
    {
        // Remove the letter from the rack
        if (!rack.in(Tile(wch)))
        {
            return 2;
        }
        rack.remove(Tile(wch));
    }

    // According to the rules in the ODS, it is allowed to pass its turn (no
    // need to change letters for that).
    // TODO: However, if all the players pass their turn, the first one has to
    // play, or change at least one letter. To implement this behaviour, we
    // must also take care of blocked positions, where no one _can_ play (see
    // also comment in the endGame() method).

    return 0;
}


int FreeGame::pass(const wstring &iToChange)
{
    Player &player = *m_players[m_currPlayer];
    int res = checkPass(player, iToChange);
    if (res != 0)
        return res;

    Move move(iToChange);
    // End the player's turn
    recordPlayerMove(move, player, true);

    // Next game turn
    endTurn();

    return 0;
}

