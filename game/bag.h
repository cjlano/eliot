/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: bag.h,v 1.2 2005/02/05 11:14:56 ipkiss Exp $
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

#ifndef _BAG_H_
#define _BAG_H_

#include "tile.h"
#include <map>

using namespace std;


/*************************
 * A bag stores the set of free tiles for the game
 *************************/

class Bag
{
public:
    Bag();
    virtual ~Bag() {}
    void init();

    /*************************
     * take or replace a tile in the bag
     * return value :
     * 0 : Ok
     * 1 : an error occured (not enough or too many tiles
     *       of that type are in the bag)
     *************************/
    int takeTile(const Tile &iTile);
    int replaceTile(const Tile &iTile);

    /*************************
     * Returns how many 't' tiles are available
     *************************/
    int in(const Tile &iTile) const;

    /*************************
     * Returns how many tiles/vowels/consonants are available
     * Warning: nVowels(b) + nConsonants(b) != nTiles(b),
     * because of the jokers and the 'Y'.
     *************************/
    unsigned int nTiles() const  { return m_ntiles; }
    int nVowels() const;
    int nConsonants() const;

    /*************************
     * return a random available tile
     * the tile is not taken out of the bag.
     * returns 0 on failure
     *************************/
    Tile selectRandom();

    void operator=(const Bag &iOther);

    void dumpAll() const;

private:
    // Associate to each tile its number of occurrences in the bag
    map<Tile, int> m_tilesMap;
    // Total number of tiles in the bag
    int m_ntiles;
};

#endif
