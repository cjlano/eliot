/* Eliot                                                                     */
/* Copyright (C) 1999  Antoine Fraboulet                                     */
/*                                                                           */
/* This file is part of Eliot.                                               */
/*                                                                           */
/* Eliot is free software; you can redistribute it and/or modify             */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* Eliot is distributed in the hope that it will be useful,                  */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

/**
 *  \file   history.cpp
 *  \brief  Game history  system
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#include <string>
#include "rack.h"
#include "pldrack.h"
#include "round.h"
#include "turn.h"
#include "history.h"
#include "encoding.h"
#include "debug.h"


/* ******************************************************** */
/* ******************************************************** */
/* ******************************************************** */


History::History()
{
    Turn* t = new Turn ();
    m_history.clear();
    m_history.push_back(t);
}


History::~History()
{
    for (unsigned int i = 0; i < m_history.size(); i++)
    {
        if (m_history[i] != NULL)
        {
            delete m_history[i];
            m_history[i] = NULL;
        }
    }
}


int History::getSize() const
{
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
    ASSERT(0 <= n && n < m_history.size(), "Wrong turn number");
    return *(m_history[n]);
}

/*
 * This function increments the number of racks, and fills the new rack
 * with the unplayed tiles from the previous one.
 * 03 sept 2000 : We have to sort the tiles according to the new rules
 */
void History::playRound(int player, int turn, const Round& round)
{
    Rack rack;
    Turn * current_turn;

    current_turn = m_history.back();

    /* set the number and the round */
    current_turn->setNum(turn);
    current_turn->setPlayer(player);
    current_turn->setRound(round);

    /* get what was the rack for the current turn */
    current_turn->getPlayedRack().getRack(rack);

    /* remove the played tiles from the rack */
    for (int i = 0; i < round.getWordLen(); i++)
    {
        if (round.isPlayedFromRack(i))
        {
            if (round.isJoker(i))
                rack.remove(Tile::Joker());
            else
                rack.remove(round.getTile(i));
        }
    }

    /* create a new turn */
    Turn * next_turn = new Turn();
    PlayedRack pldrack;
    pldrack.setOld(rack);
    next_turn->setPlayedRack(pldrack);
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

    // now we have the previous played round in back()
    Turn* t = m_history.back();
    t->setNum(0);
    t->setPlayer(0);
    t->setRound(Round());
#ifdef BACK_REMOVE_RACK_NEW_PART
    t->getPlayedRound().setNew(Rack());
#endif
}


wstring History::toString() const
{
    wstring rs;
#ifdef DEBUG
    wchar_t buff[5];
    _swprintf(buff, 4, L"%ld", m_history.size());
    rs = L"history size = " + wstring(buff) + L"\n\n";
#endif
    for (unsigned int i = 0; i < m_history.size(); i++)
    {
        Turn *t = m_history[i];
        rs += t->toString() + L"\n";
    }
    return rs;
}

/* ******************************************************** */
/* ******************************************************** */
/* ******************************************************** */


/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
