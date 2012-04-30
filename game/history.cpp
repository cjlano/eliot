/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2007 Antoine Fraboulet & Olivier Teulière
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

#include <boost/foreach.hpp>

#include <string>
#include "rack.h"
#include "pldrack.h"
#include "move.h"
#include "turn.h"
#include "history.h"
#include "encoding.h"
#include "debug.h"


INIT_LOGGER(game, History);


History::History()
{
    Turn* t = new Turn();
    m_history.clear();
    m_history.push_back(t);
}


History::~History()
{
    BOOST_FOREACH(Turn *turn, m_history)
    {
        delete turn;
    }
}


unsigned int History::getSize() const
{
    ASSERT(!m_history.empty(), "Invalid history size");
    return m_history.size() - 1;
}


const PlayedRack& History::getCurrentRack() const
{
    return m_history.back()->getPlayedRack();
}


void History::setCurrentRack(const PlayedRack &iPld)
{
    m_history.back()->setPlayedRack(iPld);
}


const Turn& History::getPreviousTurn() const
{
    int idx = m_history.size() - 2;
    ASSERT(0 <= idx , "No previous turn");
    return *(m_history[idx]);
}


const Turn& History::getTurn(unsigned int n) const
{
    ASSERT(n < m_history.size(), "Wrong turn number");
    return *(m_history[n]);
}


bool History::beforeFirstRound() const
{
    for (unsigned int i = 0; i < m_history.size() - 1; i++)
    {
        if (m_history[i]->getMove().isValid())
            return false;
    }
    return true;
}


void History::playMove(unsigned int iPlayer,
                       const Move &iMove,
                       const PlayedRack &iNewRack)
{
    Turn * current_turn = m_history.back();

    // Set the number and the round
    current_turn->setPlayer(iPlayer);
    current_turn->setMove(iMove);

    // Create a new turn
    Turn * next_turn = new Turn();
    next_turn->setPlayedRack(iNewRack);
    m_history.push_back(next_turn);
}


void History::removeLastTurn()
{
    int idx = m_history.size();
    ASSERT(0 < idx , "Wrong turn number");

    if (idx > 1)
    {
        Turn *t = m_history.back();
        m_history.pop_back();
        delete t;
    }

    // Now we have the previous played round in back()
    Turn *t = m_history.back();
    t->setPlayer(0);
    //t->setRound(Round());
#ifdef BACK_REMOVE_RACK_NEW_PART
    t->getPlayedRound().setNew(Rack());
#endif
}


void History::addWarning()
{
    ASSERT(m_history.size() > 1, "Too short history");
    m_history[m_history.size() - 2]->addWarning();
}


void History::removeWarning()
{
    ASSERT(m_history.size() > 1, "Too short history");
    m_history[m_history.size() - 2]->addWarning(-1);
}


void History::addPenaltyPoints(int iPoints)
{
    ASSERT(m_history.size() > 1, "Too short history");
    m_history[m_history.size() - 2]->addPenaltyPoints(iPoints);
}


void History::addSoloPoints(int iPoints)
{
    ASSERT(m_history.size() > 1, "Too short history");
    m_history[m_history.size() - 2]->addSoloPoints(iPoints);
}


wstring History::toString() const
{
    wstring rs;
#ifdef DEBUG
    wchar_t buff[5];
    _swprintf(buff, 4, L"%ld", m_history.size());
    rs = L"history size = " + wstring(buff) + L"\n\n";
#endif
    BOOST_FOREACH(const Turn *turn, m_history)
    {
        rs += turn->toString() + L"\n";
    }
    return rs;
}

