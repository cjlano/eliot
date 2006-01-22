/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
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

#ifndef _TILE_H_
#define _TILE_H_

#include <list>
#include <vector>

using namespace std;


/*************************
 * A Tile is the internal representation
 * used within the game library to
 * handle letters
 *************************/

class Tile
{
public:

  // a lowercase character always a joker
  // - this permits to detect joker in already played games
  // - we need to pay attention when inserting character taken
  //   from user input

    Tile(wchar_t c = 0);
    virtual ~Tile() {}

    bool isEmpty() const        { return m_dummy; }
    bool isJoker() const        { return m_joker; }
    bool isVowel() const;
    bool isConsonant() const;
    unsigned int maxNumber() const;
    unsigned int getPoints() const;
    wchar_t toChar() const;
    unsigned int toCode() const;

    static const Tile &dummy()  { return m_TheDummy; }
    static const Tile &Joker()  { return m_TheJoker; }
    static const list<Tile>& getAllTiles();
    static const Tile &GetTileFromCode(unsigned int iCode);

    bool operator <(const Tile &iOther) const;
    bool operator ==(const Tile &iOther) const;
    bool operator !=(const Tile &iOther) const;

private:
    wchar_t m_char;
    bool m_joker;
    bool m_dummy;

    /**
     * Internal code, used in the dictionary to represent the letter.
     * It is mainly used by the Cross class.
     */
    int m_code;

    // Special tiles are declared static
    static const Tile m_TheJoker;
    static const Tile m_TheDummy;

    /// List of available tiles
    static list<Tile> m_tilesList;
    /// Vector of tiles indexed by their code, for fast look-up
    static vector<Tile> m_tilesVect;
    /// True when m_tilesVect is correctly initialized
    static bool m_vectInitialized;
};

#endif

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
