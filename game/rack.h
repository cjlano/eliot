/*****************************************************************************
 * Eliot
 * Copyright (C) 2002-2007 Antoine Fraboulet & Olivier Teulière
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

#ifndef _RACK_H_
#define _RACK_H_

#include <vector>
#include <string>

#include "tile.h"

using namespace std;


/**
 * A rack is a set of tiles, no more.
 * Tiles have to be in the bag for the rack to be valid.
 */
class Rack
{
public:
    Rack();

    unsigned int getNbTiles() const      { return m_ntiles; }
    bool isEmpty() const        { return getNbTiles() == 0; }

    unsigned int in(const Tile &t) const { return m_tiles[t.toCode()]; }
    void add(const Tile &t)     { m_tiles[t.toCode()]++; m_ntiles++; }
    void remove(const Tile &t);
    void clear();
    void getTiles(vector<Tile> &oTiles) const;

    wstring toString() const;

private:
    /// Vector indexed by tile codes, containing the number of tiles
    vector<unsigned int> m_tiles;
    unsigned int m_ntiles;
};

#endif

