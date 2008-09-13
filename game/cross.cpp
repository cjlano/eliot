/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2007 Olivier Teulière & Antoine Fraboulet
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
 *          Antoine Fraboulet <antoine.fraboulet @@ free.fr>
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

#include <string>
#include "cross.h"

#define CROSS_MASK 0xFFFFFFFF


Cross::Cross()
{
    // The default behaviour is to match everything
    setAny();
}


void Cross::setAny()
{
    m_mask = CROSS_MASK;
}


bool Cross::isAny() const
{
    return m_mask == CROSS_MASK;
}


string Cross::getHexContent() const
{
    char buff[10];
    sprintf(buff,"%08x",m_mask);
    string s(buff);
    return s;
}


bool Cross::check(const Tile& iTile) const
{
    return m_mask & (1 << iTile.toCode());
}


void Cross::insert(const Tile& iTile)
{
    m_mask |= (1 << iTile.toCode());
}


bool Cross::operator==(const Cross &iOther) const
{
    return m_mask == iOther.m_mask;
}

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
