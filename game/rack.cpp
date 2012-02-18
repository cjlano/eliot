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

#include "rack.h"
#include "dic.h"
#include "encoding.h"
#include "debug.h"


INIT_LOGGER(game, Rack);


Rack::Rack()
    : m_tiles(Dictionary::GetDic().getTileNumber() + 1, 0), m_ntiles(0)
{
}


void Rack::remove(const Tile &t)
{
    ASSERT(in(t),
           "The rack does not contain the letter " + lfw(t.getDisplayStr()));
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


void Rack::getTiles(vector<Tile> &oTiles) const
{
    oTiles.reserve(m_ntiles);
    for (unsigned int i = 1; i < m_tiles.size(); i++)
    {
        // Add m_tiles[i] copies of the tile at the end of the vector
        oTiles.insert(oTiles.end(), m_tiles[i], Dictionary::GetDic().getTileFromCode(i));
    }
}


wstring Rack::toString() const
{
    wstring rs;
    for (unsigned int i = 1; i < m_tiles.size(); i++)
    {
        // Append m_tiles[i] copies of the char
        const wstring &chr = Dictionary::GetDic().getTileFromCode(i).getDisplayStr();
        for (unsigned int j = 0; j < m_tiles[i]; ++j)
        {
            rs += chr;
        }
    }
    return rs;
}

