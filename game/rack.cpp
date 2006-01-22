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
 *  \file   rack.cpp
 *  \brief  Rack class : multiset of tiles
 *  \author Antoine Fraboulet & Olivier Teuliere
 *  \date   2002 - 2005
 */

#include "rack.h"
#include "encoding.h"
#include "debug.h"

// FIXME: should not be here (duplicated from tile.cpp)
#define TILES_NUMBER 28
#define MIN_CODE 1


Rack::Rack()
    : m_tiles(TILES_NUMBER, 0), m_ntiles(0)
{
}

void Rack::remove(const Tile &t)
{
    ASSERT(in(t),
           "The rack does not contain the letter " + convertToMb(t.toChar()));
    m_tiles[t.toCode()]--;
    m_ntiles--;
}


void Rack::clear()
{
    for (unsigned int i = 0; i < m_tiles.size(); i++)
    {
        m_tiles[i] = 0;
    }
    m_ntiles = 0;
}


void Rack::getTiles(list<Tile> &oTiles) const
{
    for (unsigned int i = MIN_CODE; i < m_tiles.size(); i++)
    {
        for (unsigned int j = 0; j < m_tiles[i]; j++)
        {
            oTiles.push_back(Tile::GetTileFromCode(i));
        }
    }
}


wstring Rack::toString()
{
    wstring rs;
    for (unsigned int i = MIN_CODE; i < m_tiles.size(); i++)
    {
        for (unsigned int j = 0; j < m_tiles[i]; j++)
        {
            rs += Tile::GetTileFromCode(i).toChar();
        }
    }
    return rs;
}

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
