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

#ifndef _CROSS_H_
#define _CROSS_H_

#include <set>
#include "tile.h"

using namespace std;


/*************************
 *
 *************************/

class Cross
{
public:
    Cross();
    virtual ~Cross() {}

    void setAny()                   { m_any = true; }
    bool isAny() const              { return m_any; }
    bool check(const Tile& iTile) const;

    bool operator==(const Cross &iOther) const;
    bool operator!=(const Cross &iOther) const { return !(*this == iOther); }

    // Standard set methods (almost)
    void insert(const Tile& iTile)  { m_mask |= (1 << iTile.toCode()); }
    void clear();

private:
    /// Mask indicating which tiles are accepted for the cross check
    unsigned int m_mask;

    /// When this value is true, any letter matches the cross check
    bool m_any;
};

#endif

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
