/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2012 Antoine Fraboulet & Olivier Teulière
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

#include <sstream>

#include "turn.h"


INIT_LOGGER(game, Turn);


// FIXME: move set to an arbitrary one (no move). It would be better to get rid of this
// constructor completely
Turn::Turn()
    : m_playerId(0), m_warningsNb(0),
    m_penaltyPoints(0), m_soloPoints(0)
{
}


Turn::Turn(unsigned int iPlayerId, const PlayedRack& iPldRack,
           const Move& iMove)
    : m_playerId(iPlayerId), m_pldrack(iPldRack), m_move(iMove),
    m_warningsNb(0), m_penaltyPoints(0), m_soloPoints(0)
{
}


wstring Turn::toString() const
{
    wostringstream oss;
    oss << m_pldrack.toString() << L" " << m_move.toString()
        << L" (W=" << m_warningsNb
        << L" P=" << m_penaltyPoints
        << L" S=" << m_soloPoints << L")";
    return oss.str();
}

