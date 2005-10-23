/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: cross.h,v 1.4 2005/10/23 14:53:43 ipkiss Exp $
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

#include "tile.h"
#include <set>

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
    unsigned int size() const       { return m_tilesSet.size(); }
    void insert(const Tile& iTile)  { m_tilesSet.insert(iTile); }
    void clear();

private:
    // Set of the tiles accepted for the cross check
    set<Tile> m_tilesSet;

    // When this value is true, any letter matches the cross check
    bool m_any;
};

#endif
