/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
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

#include "cross.h"


Cross::Cross()
{
    // the default behaviour is to match everything
    m_any = true;
}


void Cross::clear()
{
    m_tilesSet.clear();
    m_any = false;
}


bool Cross::check(const Tile& iTile) const
{
    if (m_any || (iTile.isJoker() && !m_tilesSet.empty()))
        return true;
    set<Tile>::const_iterator it = m_tilesSet.find(iTile);
    return it != m_tilesSet.end();
}


bool Cross::operator==(const Cross &iOther) const
{
    if (isAny() && iOther.isAny())
        return true;
    // Two sets are equal if they have the same size and one of them contains
    // the other one
    if (m_tilesSet.size() == iOther.m_tilesSet.size())
    {
        set<Tile>::const_iterator it;
        for (it = m_tilesSet.begin(); it != m_tilesSet.end(); it++)
        {
            if (!iOther.check(*it))
                return false;
        }
        return true;
    }
    else
        return false;
}

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
