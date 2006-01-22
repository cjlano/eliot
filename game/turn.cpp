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


Turn::Turn()
{
    m_num      = 0;
    m_playerId = 0;
    m_pldrack  = PlayedRack();
    m_round    = Round();
}

Turn::Turn(int iNum, int iPlayerId,
           const PlayedRack& iPldRack, const Round& iRound)
    : m_num(iNum), m_playerId(iPlayerId), m_pldrack(iPldRack), m_round(iRound)
{
}

#if 0
void Turn::operator=(const Turn &iOther)
{
    m_num      = iOther.m_num;
    m_playerId = iOther.m_playerId;
    m_pldrack  = iOther.m_pldrack;
    m_round    = iOther.m_round;
}
#endif

wstring Turn::toString(bool iShowExtraSigns) const
{
    wstring rs = L"";
    if (iShowExtraSigns)
    {
        // TODO
    }
    rs = rs + m_pldrack.toString() + L" " + m_round.toString();
    return rs;
}

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
