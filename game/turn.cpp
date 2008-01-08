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

/**
 *  \file   turn.cpp
 *  \brief  Game turn (= id + pldrack + move)
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#include "turn.h"


// FIXME: move set to an invalid value. It would be better to get rid of this
// constructor completely
Turn::Turn()
    : m_num(0), m_playerId(0), m_move(L"", L"")
{
}


Turn::Turn(unsigned int iNum, unsigned int iPlayerId,
           const PlayedRack& iPldRack, const Move& iMove)
    : m_num(iNum), m_playerId(iPlayerId), m_pldrack(iPldRack), m_move(iMove)
{
}


wstring Turn::toString(bool iShowExtraSigns) const
{
    wstring rs;
    if (iShowExtraSigns)
    {
        // TODO
    }
    rs = rs + m_pldrack.toString() + L" " + m_move.toString();
    return rs;
}

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
