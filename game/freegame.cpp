/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
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

#include <iomanip>
#include <wctype.h>
#include "dic.h"
#include "tile.h"
#include "rack.h"
#include "round.h"
#include "pldrack.h"
#include "results.h"
#include "player.h"
#include "ai_player.h"
#include "freegame.h"

#include "debug.h"


FreeGame::FreeGame(const Dictionary &iDic): Game(iDic)
{
}


FreeGame::~FreeGame()
{
}


int FreeGame::setRackRandom(int p, bool iCheck, set_rack_mode mode)
{
    int res;
    do
    {
        res = helperSetRackRandom(p, iCheck, mode);
    } while (res == 2);
    return res;
}


int FreeGame::play(const wstring &iCoord, const wstring &iWord)
{
    /* Perform all the validity checks, and fill a round */
    Round round;

    int res = checkPlayedWord(iCoord, iWord, round);
    if (res != 0)
    {
        return res;
    }

    /* Update the rack and the score of the current player */
    m_players[m_currPlayer]->addPoints(round.getPoints());
    m_players[m_currPlayer]->endTurn(round, m_history.getSize());

    /* Everything is OK, we can play the word */
    helperPlayRound(round);

    /* Next turn */
    // XXX: Should it be done by the interface instead?
    endTurn();

    return 0;
}


void FreeGame::freegameAI(int n)
{
    ASSERT(0 <= n && n < getNPlayers(), "Wrong player number");
    ASSERT(!m_players[n]->isHuman(), "AI requested for a human player");

    AIPlayer *player = static_cast<AIPlayer*>(m_players[n]);

    player->compute(*m_dic, m_board, m_history.getSize());
    if (player->changesLetters())
    {
        helperPass(player->getChangedLetters(), n);
        endTurn();
    }
    else
    {
        const Round &round = player->getChosenRound();
        /* Update the rack and the score of the current player */
        player->addPoints(round.getPoints());
        player->endTurn(round, m_history.getSize());

        helperPlayRound(round);
        endTurn();
    }
}


int FreeGame::start()
{
    ASSERT(getNPlayers(), "Cannot start a game without any player");

    /* Set the initial racks of the players */
    for (int i = 0; i < getNPlayers(); i++)
    {
        setRackRandom(i, false, RACK_NEW);
    }

    // XXX
    m_currPlayer = 0;

    /* If the first player is an AI, make it play now */
    if (!m_players[0]->isHuman())
    {
        freegameAI(0);
    }

    return 0;
}


int FreeGame::endTurn()
{
    /* Complete the rack for the player that just played */
    if (setRackRandom(m_currPlayer, false, RACK_NEW) == 1)
    {
        /* End of the game */
        end();
        return 1;
    }

    /* Next player */
    nextPlayer();

    /* If this player is an AI, make it play now */
    if (!m_players[m_currPlayer]->isHuman())
    {
        freegameAI(m_currPlayer);
    }

    return 0;
}


// Adjust the scores of the players with the points of the remaining tiles
void FreeGame::end()
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
    // to handle it in the end() method (very easy).

    /* Add the points of the remaining tiles to the score of the current
     * player (i.e. the first player with an empty rack), and remove them
     * from the score of the players who still have tiles */
    for (int i = 0; i < getNPlayers(); i++)
    {
        if (i != m_currPlayer)
        {
            const PlayedRack &pld = m_players[i]->getCurrentRack();
            pld.getAllTiles(tiles);
            for (unsigned int j = 0; j < tiles.size(); j++)
            {
                m_players[i]->addPoints(- tiles[j].getPoints());
                m_players[m_currPlayer]->addPoints(tiles[j].getPoints());
            }
        }
    }

    /* Lock game */
    m_finished = true;
}


int FreeGame::pass(const wstring &iToChange, int n)
{
    if (m_finished)
        return 3;

    // According to the rules in the ODS, it is allowed to pass its turn (no
    // need to change letters for that).
    // TODO: However, if all the players pass their turn, the first one has to
    // play, or change at least one letter. To implement this behaviour, we
    // must also take care of blocked positions, where no one _can_ play (see
    // also comment in the end() method).

    // Convert the string into tiles
    vector<Tile> tilesVect;
    for (unsigned int i = 0; i < iToChange.size(); i++)
    {
        Tile tile(towupper(iToChange[i]));
        tilesVect.push_back(tile);
    }

    int res = helperPass(tilesVect, n);
    if (res == 0)
        endTurn();
    return res;
}


int FreeGame::helperPass(const vector<Tile> &iToChange, int n)
{
    ASSERT(0 <= n && n < getNPlayers(), "Wrong player number");

    // It is forbidden to change letters when the bag does not contain at
    // least 7 letters (this is explicitely stated in the ODS).
    Bag bag;
    realBag(bag);
    if (bag.nTiles() < 7)
    {
        return 1;
    }

    Player *player = m_players[n];
    PlayedRack pld = player->getCurrentRack();
    Rack rack;
    pld.getRack(rack);

    for (unsigned int i = 0; i < iToChange.size(); i++)
    {
        /* Remove the letter from the rack */
        if (!rack.in(iToChange[i]))
        {
            return 2;
        }
        rack.remove(iToChange[i]);
    }

    pld.reset();
    pld.setOld(rack);

    player->setCurrentRack(pld);

    // FIXME: the letters to change should not be in the bag while generating
    // the new rack!

    return 0;
}

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
