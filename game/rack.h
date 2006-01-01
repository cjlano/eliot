/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
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
 *  \file   rack.h
 *  \brief  Rack class : multiset of tiles
 *  \author Antoine Fraboulet & Olivier Teuliere
 *  \date   2002 - 2005
 */

#ifndef _RACK_H_
#define _RACK_H_

#include "tile.h"
#include <set>
#include <list>
#include <string>

using namespace std;


/**
 * A rack is a set of tiles, no more.
 * Tiles have to be in the bag for the rack to be valid.
 */
class Rack
{
public:
    Rack() {}
    virtual ~Rack() {}

    int nTiles() const          { return m_tiles.size(); }
    bool isEmpty() const        { return nTiles() == 0; }

    unsigned int in(const Tile &t) const { return m_tiles.count(t); }
    void add(const Tile &t)     { m_tiles.insert(t); }
    void remove(const Tile &t);
    void clear()                { m_tiles.clear(); }
    void getTiles(list<Tile> &oTiles) const;

    string toString();

private:
    multiset<Tile> m_tiles;
};

#endif

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
