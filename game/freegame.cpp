/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: freegame.cpp,v 1.2 2005/02/09 22:33:56 ipkiss Exp $
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/

#include <iomanip>
#include "dic.h"
#include "tile.h"
#include "rack.h"
#include "round.h"
#include "pldrack.h"
#include "results.h"
#include "player.h"
#include "freegame.h"

#include "debug.h"


/*********************************************************
 *********************************************************/

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


int FreeGame::play(const string &iCoord, const string &iWord)
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
    m_players[m_currPlayer]->endTurn(round, getNRounds());

    /* Everything is OK, we can play the word */
    helperPlayRound(round);

    /* Next turn */
    // XXX: Should it be done by the interface instead?
    endTurn();

    return 0;
}


int FreeGame::freegameAI(int n)
{
    PDEBUG(n < 0 || n >= getNPlayers(), "GAME: wrong player number\n");
    Player *player = m_players[n];

    PDEBUG(player->isHuman(), "GAME: AI requested for a human player!\n");

    player->aiSearch(*m_dic, m_board, getNRounds());
    const Results &results = player->aiGetResults();
    if (results.in() == 0)
    {
        /* XXX: a better way to indicate that the AI player passes
         * should be found. In particular, we don't know the letters
         * it wants to change. */
        pass("", n);
    }
    else
    {
        const Round &round = results.get(0);
        /* Update the rack and the score of the current player */
        player->addPoints(round.getPoints());
        player->endTurn(round, getNRounds());

        helperPlayRound(round);
        endTurn();
    }

    return 0;
}


int FreeGame::start()
{
    int i;
    if (getNPlayers() == 0)
        return 1;

    /* Set the initial racks of the players */
    for (i = 0; i < getNPlayers(); i++)
    {
        setRackRandom(i, 0, RACK_ALL);
    }

    // XXX
    m_currPlayer = 0;

    /* If the first player is an AI, make it play now */
    if (! m_players[0]->isHuman())
    {
        if (freegameAI(0))
            return 2;
    }

    return 0;
}


int FreeGame::endTurn()
{
    /* Complete the rack for the player that just played */
    if (setRackRandom(m_currPlayer, 0, RACK_NEW) == 1)
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
        if (freegameAI(m_currPlayer))
            return 2;
    }

    return 0;
}


/* Adjust the scores of the players with the points of the remaining tiles */
void FreeGame::end()
{
    vector<Tile> tiles;

    /* Add the points of the remaining tiles to the score of the current
     * player (i.e. the first player with an empty rack), and remove them
     * from the score of the players who still had tiles */
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


int FreeGame::pass(const string &iToChange, int n)
{
    if (m_finished)
        return 3;
    PDEBUG(n < 0 || n >= getNPlayers(), "GAME: wrong player number\n");
    Player *player = m_players[n];

    /* You cannot change more letters than what is left in the bag! */
    Bag bag;
    realBag(bag);
    if (bag.nTiles() < iToChange.size())
    {
        return 1;
    }

    PlayedRack pld = player->getCurrentRack();
    Rack rack;
    pld.getRack(rack);

    for (unsigned int i = 0; i < iToChange.size(); i++)
    {
        /* Transform into a tile */
        Tile tile(iToChange[i]);
        if (islower(iToChange[i]))
            tile = Tile::Joker();

        /* Remove it from the rack */
        if (! rack.in(tile))
        {
            return 2;
        }
        rack.remove(tile);
    }

    pld.reset();
    pld.setOld(rack);

    player->setCurrentRack(pld);

    // FIXME: the letters to change should not be in the bag while generating
    // the new rack!

    /* Next turn */
    // XXX: Should it be done by the interface instead?
    endTurn();

    return 0;
}

