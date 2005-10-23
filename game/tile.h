/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: tile.h,v 1.5 2005/10/23 14:53:43 ipkiss Exp $
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

    Tile(char c = 0);
    virtual ~Tile() {}

    bool isEmpty() const        { return m_dummy; }
    bool isJoker() const        { return m_joker; }
    bool isVowel() const;
    bool isConsonant() const;
    unsigned int maxNumber() const;
    unsigned int getPoints() const;
    char toChar() const;
    int toCode() const;
    
    static const Tile &dummy()  { return m_TheDummy; }
    static const Tile &Joker()  { return m_TheJoker; }
    static const list<Tile>& getAllTiles();

    bool operator <(const Tile &iOther) const;
    bool operator ==(const Tile &iOther) const;
    bool operator !=(const Tile &iOther) const;

private:
    char m_char;
    bool m_joker;
    bool m_dummy;

    // Special tiles are declared static
    static const Tile m_TheJoker;
    static const Tile m_TheDummy;

    // List of available tiles
    static list<Tile> m_tilesList;
};

#endif
