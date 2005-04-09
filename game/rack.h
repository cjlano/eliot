/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: rack.h,v 1.4 2005/04/09 14:58:24 afrab Exp $
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/

#ifndef _RACK_H_
#define _RACK_H_

#include "tile.h"
#include <set>
#include <list>

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

private:
    multiset<Tile> m_tiles;
};

#endif
