/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2012 Olivier Teulière & Antoine Fraboulet
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

#ifndef CROSS_H_
#define CROSS_H_

#include <set>
#include "tile.h"
#include "logging.h"

using namespace std;


/*************************
 *
 *************************/

// TODO: implement using the bitset class
class Cross
{
    DEFINE_LOGGER();
public:
    Cross();

    void setAny();
    void setNone() { m_mask = 0; }

    bool isAny() const;
    bool isNone() const { return m_mask == 0; }

    bool check(const Tile& iTile) const;

    bool operator==(const Cross &iOther) const;
    bool operator!=(const Cross &iOther) const { return !(*this == iOther); }

    // Standard set methods (almost)
    void insert(const Tile& iTile);

    string getHexContent() const;
private:
    /// Mask indicating which tiles are accepted for the cross check
    unsigned int m_mask;
};

#endif

