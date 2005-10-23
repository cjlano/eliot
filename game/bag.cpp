/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: bag.cpp,v 1.4 2005/10/23 14:53:43 ipkiss Exp $
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

#include "tile.h"
#include "bag.h"
#include "debug.h"


Bag::Bag()
{
    init();
}


void Bag::init()
{
    m_ntiles = 0;
    const list<Tile>& allTiles = Tile::getAllTiles();
    list<Tile>::const_iterator it;
    for (it = allTiles.begin(); it != allTiles.end(); it++)
    {
        m_tilesMap[*it] = it->maxNumber();
        m_ntiles += it->maxNumber();
    }
}


unsigned int Bag::in(const Tile &iTile) const
{
    map<Tile, int>::const_iterator it = m_tilesMap.find(iTile);
    if (it != m_tilesMap.end())
        return (*it).second;
    return 0;
}


unsigned int Bag::nVowels() const
{
    map<Tile, int>::const_iterator it;
    int v = 0;

    for (it = m_tilesMap.begin(); it != m_tilesMap.end(); it++)
    {
        if (it->first.isVowel())
            v += it->second;
    }
    return v;
}


unsigned int Bag::nConsonants() const
{
    map<Tile, int>::const_iterator it;
    int c = 0;

    for (it = m_tilesMap.begin(); it != m_tilesMap.end(); it++)
    {
        if (it->first.isConsonant())
            c += it->second;
    }
    return c;
}


void Bag::takeTile(const Tile &iTile)
{
    ASSERT(in(iTile),
           string("The bag does not contain the letter ") + iTile.toChar());

    m_tilesMap[iTile]--;
    m_ntiles--;
}


void Bag::replaceTile(const Tile &iTile)
{
    ASSERT(in(iTile) < iTile.maxNumber(),
           string("Cannot replace tile: ") + iTile.toChar());

    m_tilesMap[iTile]++;
    m_ntiles++;
}


Tile Bag::selectRandom()
{
    map<Tile, int>::const_iterator it;
    int n;
    double max = m_ntiles;

    n = (int)(max * rand() / (RAND_MAX + 1.0));
    for (it = m_tilesMap.begin(); it != m_tilesMap.end(); it++)
    {
        if (n < it->second)
            return it->first;
        n -= it->second;
    }
    ASSERT(false, "We should not come here");
    return Tile::dummy();
}


void Bag::operator=(const Bag &iOther)
{
    m_tilesMap = iOther.m_tilesMap;
    m_ntiles = iOther.m_ntiles;
}


void Bag::dumpAll() const
{
    map<Tile, int>::const_iterator it;
    for (it = m_tilesMap.begin(); it != m_tilesMap.end(); it++)
    {
        if (it->second)
            fprintf(stderr, "%c[%i] ", it->first.toChar(), it->second);
    }
    fprintf(stderr, "\n");
}
