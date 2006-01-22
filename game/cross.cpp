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
    // The default behaviour is to match everything
    m_any = true;
    m_mask = 0xFFFFFFFF;
}


void Cross::clear()
{
    m_any = false;
    m_mask = 0;
}


bool Cross::check(const Tile& iTile) const
{
    if (m_any || (iTile.isJoker() && m_mask != 0))
        return true;
    return m_mask & (1 << iTile.toCode());
}


bool Cross::operator==(const Cross &iOther) const
{
    if (isAny() || iOther.isAny())
        return isAny() && iOther.isAny();

    return m_mask == iOther.m_mask;
}

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
