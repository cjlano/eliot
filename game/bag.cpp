/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2007 Antoine Fraboulet & Olivier Teulière
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

#include <string>
#include <cstdlib> // For rand()

#include <dic.h>
#include "bag.h"
#include "debug.h"
#include "encoding.h"


Bag::Bag(const Dictionary &iDic)
    : m_dic(iDic)
{
    m_ntiles = 0;
    const vector<Tile>& allTiles = m_dic.getAllTiles();
    vector<Tile>::const_iterator it;
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


unsigned int Bag::getNbVowels() const
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


unsigned int Bag::getNbConsonants() const
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
           "The bag does not contain the letter " + convertToMb(iTile.toChar()));

    m_tilesMap[iTile]--;
    m_ntiles--;
}


void Bag::replaceTile(const Tile &iTile)
{
    ASSERT(in(iTile) < iTile.maxNumber(),
           "Cannot replace tile: " + convertToMb(iTile.toChar()));

    m_tilesMap[iTile]++;
    m_ntiles++;
}


Tile Bag::selectRandom() const
{
    double max = m_ntiles;
    ASSERT(max > 0, "The bag is empty");

    int n = (int)(max * rand() / (RAND_MAX + 1.0));

    map<Tile, int>::const_iterator it;
    for (it = m_tilesMap.begin(); it != m_tilesMap.end(); it++)
    {
        if (n < it->second)
            return it->first;
        n -= it->second;
    }
    ASSERT(false, "We should not come here");
    return Tile();
}


Tile Bag::selectRandomVowel() const
{
    double max = getNbVowels();
    ASSERT(max > 0, "Not enough vowels in the bag");

    int n = (int)(max * rand() / (RAND_MAX + 1.0));

    map<Tile, int>::const_iterator it;
    for (it = m_tilesMap.begin(); it != m_tilesMap.end(); it++)
    {
        if (!it->first.isVowel())
            continue;
        if (n < it->second)
            return it->first;
        n -= it->second;
    }
    ASSERT(false, "We should not come here");
    return Tile();
}


Tile Bag::selectRandomConsonant() const
{
    double max = getNbConsonants();
    ASSERT(max > 0, "Not enough consonants in the bag");

    int n = (int)(max * rand() / (RAND_MAX + 1.0));

    map<Tile, int>::const_iterator it;
    for (it = m_tilesMap.begin(); it != m_tilesMap.end(); it++)
    {
        if (!it->first.isConsonant())
            continue;
        if (n < it->second)
            return it->first;
        n -= it->second;
    }
    ASSERT(false, "We should not come here");
    return Tile();
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
            fprintf(stderr, "%lc[%i] ", it->first.toChar(), it->second);
    }
    fprintf(stderr, "\n");
}

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
