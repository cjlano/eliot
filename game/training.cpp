/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
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

#include "dic.h"
#include "tile.h"
#include "rack.h"
#include "round.h"
#include "pldrack.h"
#include "player.h"
#include "training.h"

#include "debug.h"


Training::Training(const Dictionary &iDic): Game(iDic)
{
}


Training::~Training()
{
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


int Training::setRackRandom(int p, bool iCheck, set_rack_mode mode)
{
    int res;
    m_results.clear();
    do
    {
        res = helperSetRackRandom(p, iCheck, mode);
    } while (res == 2);
    // 0 : ok
    // 1 : not enough tiles
    // 2 : check failed (number of voyels before round 15)
    return res;
}


int Training::setRackManual(bool iCheck, const string &iLetters)
{
    int res;
    int p = m_currPlayer;
    string::iterator it;
    string uLetters; // uppercase letters
    // letters can be lowercase or uppercase as they are
    // coming from user input. We do not consider a lowercase
    // letter to be a joker which has been assigned to a letter.
    m_results.clear();
    uLetters = iLetters;
    for(it = uLetters.begin(); it != uLetters.end(); it ++)
      {
	*it = toupper(*it);
      }
    res = helperSetRackManual(p, iCheck, uLetters);
    // 0 : ok
    // 1 : not enough tiles
    // 2 : check failed (number of voyels before round 15)
    return res;
}


int Training::start()
{
    if (getNPlayers() != 0)
        return 1;

    // Training mode implicitly uses 1 human player
    Game::addHumanPlayer();
    m_currPlayer = 0;
    return 0;
}


int Training::endTurn()
{
    // Nothing to do?
    return 0;
}


void Training::search()
{
    // Search for the current player
    Rack r;
    m_players[m_currPlayer]->getCurrentRack().getRack(r);
    m_results.search(*m_dic, m_board, r, getNRounds());
}


int Training::playResult(int n)
{
    Player *player = m_players[m_currPlayer];
    if (n >= m_results.size())
        return 2;
    const Round &round = m_results.get(n);

    /* Update the rack and the score of the current player */
    player->addPoints(round.getPoints());
    player->endTurn(round, getNRounds());

    int res = helperPlayRound(round);

    if (res == 0)
        m_results.clear();

    /* Next turn */
    // XXX: Should it be done by the interface instead?
    endTurn();

    return res;
}


void Training::addHumanPlayer()
{
    // We are not supposed to be here...
    ASSERT(false, "Trying to add a human player in Training mode");
}


void Training::addAIPlayer()
{
    // We are not supposed to be here...
    ASSERT(false, "Trying to add a AI player in Training mode");
}


int Training::getNResults() const
{
    return m_results.size();
}


string Training::getSearchedWord(int num) const
{
    ASSERT(0 <= num && num < m_results.size(), "Wrong result number");
    char c;
    string s;
    const Round &r = m_results.get(num);
    for (int i = 0; i < r.getWordLen(); i++)
    {
        c = r.getTile(i).toChar();
        if (r.isJoker(i))
            c = tolower(c);
        s += c;
    }
    return s;
}


string Training::getSearchedCoords(int num) const
{
    ASSERT(0 <= num && num < m_results.size(), "Wrong result number");
    return formatCoords(m_results.get(num));
}


int Training::getSearchedPoints(int num) const
{
    ASSERT(0 <= num && num < m_results.size(), "Wrong result number");
    return m_results.get(num).getPoints();
}


int Training::getSearchedBonus(int num) const
{
    ASSERT(0 <= num && num < m_results.size(), "Wrong result number");
    return m_results.get(num).getBonus();
}


void Training::testPlay(int num)
{
    ASSERT(0 <= num && num < m_results.size(), "Wrong result number");
    m_board.testRound(m_results.get(num));
}


void Training::removeTestPlay()
{
    m_board.removeTestRound();
}

