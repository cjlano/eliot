/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: training.cpp,v 1.1 2005/02/05 11:14:56 ipkiss Exp $
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

#include "dic.h"
#include "tile.h"
#include "rack.h"
#include "round.h"
#include "pldrack.h"
#include "results.h"
#include "player.h"
#include "training.h"

#include "debug.h"


/*********************************************************
 *********************************************************/

Training::Training(const Dictionary &iDic): Game(iDic)
{
}


Training::~Training()
{
}


int Training::search()
{
    // Search for the current player
    return m_players[m_currPlayer]->aiSearch(*m_dic, m_board, getNRounds());
}


int Training::play(const string &iCoord, const string &iWord)
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


int Training::playResult(int n)
{
    Player *player = m_players[m_currPlayer];
    const Results &results = player->aiGetResults();
    if (n >= results.in())
        return 2;
    const Round &round = results.get(n);

    /* Update the rack and the score of the current player */
    player->addPoints(round.getPoints());
    player->endTurn(round, getNRounds());

    int res = helperPlayRound(round);

    if (res == 0)
        player->clearResults();

    /* Next turn */
    // XXX: Should it be done by the interface instead?
    endTurn();

    return res;
}


int Training::setRackRandom(int p, bool iCheck, set_rack_mode mode)
{
    int res;
    m_players[p]->clearResults();
    do
    {
        res = helperSetRackRandom(p, iCheck, mode);
    } while (res == 2);

    return res;
}


int Training::setRackManual(bool iCheck, const string &iLetters)
{
    int res;
    int p = m_currPlayer;
    m_players[p]->clearResults();
    do
    {
        res = helperSetRackManual(p, iCheck, iLetters);
    } while (res == 2);

    return res;
}


int Training::start()
{
    if (getNPlayers() != 0)
        return 1;

    /* Training mode implicitly uses 1 human player */
    addHumanPlayer();
    m_currPlayer = 0;
    return 0;
}


int Training::endTurn()
{
    // Nothing to do?
    return 0;
}

