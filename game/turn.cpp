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
 *  \file   turn.cpp
 *  \brief  Game turn (= id + pldrack + round)
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#include <string>
#include "pldrack.h"
#include "round.h"
#include "turn.h"


Turn::Turn(int iNum, int iPlayerId,
           const PlayedRack& iPldRack, const Round& iRound)
    : m_num(iNum), m_playerId(iPlayerId), m_pldrack(iPldRack), m_round(iRound)
{
}


#if 0
void Turn::operator=(const Turn &iOther)
{
    num     = iOther.num;
    pldrack = iOther.pldrack;
    round   = iOther.round;
}


string Turn::toString(bool iShowExtraSigns) const
{
    string rs = "";
    if (iShowExtraSigns)
    {
        rs = ""; // TODO
    }
    rs = rs + m_pldrack.toString() + " " + m_round.toString();
    return rs;
}
#endif


/// Local Variables:
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
