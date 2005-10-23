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

/* $Id: turn.cpp,v 1.2 2005/10/23 14:53:43 ipkiss Exp $ */

/**
 *  \file   turn.cpp
 *  \brief  Game turn (= id + pldrack + round) 
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#include <string>
#include "pldrack.h"
#include "round.h"
#include "turn.h"


Turn::Turn()
{
    num     = 0;
    pldrack = PlayedRack();
    round   = Round();
}
    

Turn::~Turn()
{
}


void Turn::setNum(int n)
{
    num = n;
}


void Turn::setPlayer(int p)
{
    player = p;
}


void Turn::setPlayedRack(const PlayedRack &r)
{
    pldrack = r;
}

void Turn::setRound(const Round &r)
{
    round = r;
}


int Turn::getNum() const
{
    return num;
}


int Turn::getPlayer() const
{
    return player;
}


PlayedRack Turn::getPlayedRack() const
{
    return pldrack;
}


Round Turn::getRound() const
{
    return round;
}


void 
Turn::operator=(const Turn &iOther)
{
    num     = iOther.num;
    pldrack = iOther.pldrack;
    round   = iOther.round;
}


string 
Turn::toString(bool showExtraSigns) const
{
    string rs = "";
    if (showExtraSigns)
	{
	    rs = ""; // TODO
	}
    rs = rs + pldrack.toString() + " " + round.toString();
    return rs;
}



/// Local Variables:
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
