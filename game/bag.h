/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2012 Antoine Fraboulet & Olivier Teulière
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

#ifndef BAG_H_
#define BAG_H_

#include <map>
#include "tile.h"
#include "logging.h"

using std::map;

class Dictionary;


/**
 * A bag stores the set of free tiles for the game.
 */
class Bag
{
    DEFINE_LOGGER();
public:
    explicit Bag(const Dictionary &iDic);

    /// Take a tile in the bag
    void takeTile(const Tile &iTile);
    /// Replace a tile into the bag
    void replaceTile(const Tile &iTile);

    /// Count how many tiles identical to iTile are available in the bag
    unsigned int count(const Tile &iTile) const;

    /**
     * Return how many tiles/vowels/consonants are available
     * Warning: b.getNbVowels() + b.getNbConsonants() != b.getNbTiles(),
     * because of the jokers and the 'Y'.
     */
    unsigned int getNbTiles() const  { return m_nbTiles; }
    unsigned int getNbVowels() const;
    unsigned int getNbConsonants() const;

    /**
     * Return a random available tile
     * The tile is not taken out of the bag.
     */
    Tile selectRandom() const;

    /**
     * Return a random available vowel.
     * The tile is not taken out of the bag.
     */
    Tile selectRandomVowel() const;

    /**
     * Return a random available consonant.
     * The tile is not taken out of the bag.
     */
    Tile selectRandomConsonant() const;

    Bag & operator=(const Bag &iOther);

    /// Convenience getter on the dictionary
    const Dictionary & getDic() const { return m_dic; }

    /// Print on stderr all the letters of the bag (for debugging purposes)
    void dumpAll() const;

private:
    /// Dictionary
    const Dictionary &m_dic;

    /// Associate to each tile its number of occurrences in the bag
    map<Tile, int> m_tilesMap;

    /// Total number of tiles in the bag
    int m_nbTiles;

    /// Helper method, used by the various selectRandom*() methods
    Tile selectRandomTile(unsigned int total,
                          bool onlyVowels, bool onlyConsonants) const;
};

#endif

