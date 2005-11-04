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

#include "rack.h"


void Rack::remove(const Tile &t)
{
    multiset<Tile>::const_iterator it = m_tiles.find(t);
    if (it != m_tiles.end())
        m_tiles.erase(it);
}


void Rack::getTiles(list<Tile> &oTiles) const
{
    multiset<Tile>::const_iterator it;
    for (it = m_tiles.begin(); it != m_tiles.end(); it++)
    {
        oTiles.push_back(*it);
    }
}

