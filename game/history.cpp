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

/* $Id: history.cpp,v 1.2 2005/10/23 14:53:43 ipkiss Exp $ */

/**
 *  \file   history.cpp
 *  \brief  Game history  system
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#include <string>
#include "rack.h"
#include "history.h"
#include "debug.h"

/* ******************************************************** */
/* ******************************************************** */
/* ******************************************************** */


History::History()
{
    Turn* t = new Turn ();
    history.push_back(t);
}


History::~History()
{
    for (unsigned int i = 0; i < history.size(); i++)
	{
	    if (history[i] != NULL)
		{
		    delete history[i];
		    history[i] = NULL;
		}
	}
}


int History::getSize() const
{
    return history.size();
}


const PlayedRack History::getCurrentRack() const
{
    return history.back()->getPlayedRack();
}


void History::setCurrentRack(const PlayedRack &iPld)
{
    history.back()->setPlayedRack(iPld);
}

// vector
// size : number of elements
// back : last element
// pop_back : remove at the end
// push_back : add at the end

const Turn History::getPreviousTurn() const
{
    int idx = history.size() - 2;
    ASSERT(0 <= idx , "Wrong turn number");
    return *(history[ idx ]);
}


void History::playRound(int player, int turn, const Round& round)
{
    Rack rack;
    Turn * current_turn;
    Turn * next_turn;
    
    current_turn = history.back();

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
    next_turn = new Turn();
    next_turn->getPlayedRack().setOld(rack);
    history.push_back ( next_turn );
}


void History::removeLastTurn()
{
    int idx = history.size();
    ASSERT(0 < idx , "Wrong turn number");
    
    if (idx > 1)
	{
	    Turn *t = history.back();
	    history.pop_back();
	    delete t;
	}

    // now we have the previous played round in back()
    Turn* t = history.back();
    t->setNum(0);
    t->setPlayer(0);
    t->setRound(Round());
#ifdef BACK_REMOVE_RACK_NEW_PART
    t->getPlayedRound().setNew(Rack());
#endif
}


std::string 
History::toString() const
{
    std::string rs = "";
    unsigned int i;
    for ( i = 0; i < history.size(); i++)
	{
	    string pr,ro;
	    pr = history[i]->getPlayedRack().toString();
	    ro = history[i]->getRound().toString();
	    rs += string(" ") + pr + string(" ") + ro + string("\n");
	}
    return rs;
}

/* ******************************************************** */
/* ******************************************************** */
/* ******************************************************** */


/// Local Variables:
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
