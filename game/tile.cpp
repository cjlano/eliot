/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: tile.cpp,v 1.1 2005/02/05 11:14:56 ipkiss Exp $
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

#include "tile.h"
#include <ctype.h>


/*************************
 * Zero + 26 letters + joker
 *************************/
#define TILES_NUMBER 28

/* The jokers and the 'Y' can be considered both as vowels or consonants */
const int Tiles_vowels[TILES_NUMBER] =
{
/* x A B C D  E F G H I J  K L M N O P Q R S T U V  W  X  Y  Z ? */
   0,1,0,0,0, 1,0,0,0,1,0, 0,0,0,0,1,0,0,0,0,0,1,0, 0, 0, 1, 0,1
};

const int Tiles_consonants[TILES_NUMBER] =
{
/* x A B C D  E F G H I J  K L M N O P Q R S T U V  W  X  Y  Z ? */
   0,0,1,1,1, 0,1,1,1,0,1, 1,1,1,1,0,1,1,1,1,1,0,1, 1, 1, 1, 1,1
};

const int Tiles_numbers[TILES_NUMBER] =
{
/* x A B C D  E F G H I J  K L M N O P Q R S T U V  W  X  Y  Z ? */
   0,9,2,2,3,15,2,2,2,8,1, 1,5,3,6,6,2,1,6,6,6,6,2, 1, 1, 1, 1,2
};

const int Tiles_points[TILES_NUMBER] =
{
/* x A B C D  E F G H I J  K L M N O P Q R S T U V  W  X  Y  Z ? */
   0,1,3,3,2, 1,4,2,4,1,8,10,1,2,1,1,3,8,1,1,1,1,4,10,10,10,10,0
};


list<Tile> Tile::m_tilesList;
const Tile Tile::m_TheJoker('?');
const Tile Tile::m_TheDummy(0);


Tile::Tile(char c)
{
    if (c == '?')
    {
        m_joker = true;
        m_dummy = false;
        m_char = 63;
    }
    else if (isalpha(c))
    {
        // XXX: is a lowercase character always a joker?
        m_joker = islower(c);
        m_dummy = false;
        m_char = toupper(c);
    }
    else
    {
        m_joker = false;
        m_dummy = true;
        m_char = 0;
    }
}


bool Tile::isVowel() const
{
    if (m_dummy)
        return false;
    if (m_joker)
        return Tiles_vowels[27];
    return Tiles_vowels[m_char - 'A' + 1];
}


bool Tile::isConsonant() const
{
    if (m_dummy)
        return false;
    if (m_joker)
        return Tiles_consonants[27];
    return Tiles_consonants[m_char - 'A' + 1];
}


int Tile::maxNumber() const
{
    if (m_dummy)
        return false;
    if (m_joker)
        return Tiles_numbers[27];
    return Tiles_numbers[m_char - 'A' + 1];
}


int Tile::getPoints() const
{
    if (m_dummy)
        return false;
    if (m_joker)
        return Tiles_points[27];
    return Tiles_points[m_char - 'A' + 1];
}


const list<Tile>& Tile::getAllTiles()
{
    if (Tile::m_tilesList.size() == 0)
    {
        // XXX: this should be filled from a "language file" instead
        for (char i = 'A'; i <= 'Z'; i++)
            Tile::m_tilesList.push_back(Tile(i));
        m_tilesList.push_back(Tile('?'));
    }
    return Tile::m_tilesList;
}


char Tile::toChar() const
{
    // XXX: What should we return for a dummy char?
    if (m_dummy)
        return '%';
    // XXX: is this clever enough?
    if (m_joker)
    {
        if (isalpha(m_char))
            return tolower(m_char);
        else
            return '?';
    }
    return m_char;
}


bool Tile::operator <(const Tile &iOther) const
{
    if (iOther.m_dummy)
        return false;
    else if (m_dummy)
        return true;
    else if (m_joker)
        return false;
    else if (iOther.m_joker)
        return true;
    else
        return m_char < iOther.m_char;
}


bool Tile::operator ==(const Tile &iOther) const
{
    if (m_dummy || iOther.m_dummy)
        return m_dummy == iOther.m_dummy;
    if (m_joker || iOther.m_joker)
    {
        if (m_joker != iOther.m_joker)
            return false;
        return m_char == iOther.m_char;
    }
    return m_char == iOther.m_char;
//     return (m_joker && iOther.m_joker && m_char == iOther.m_char) ||
//            (m_dummy && iOther.m_dummy) ||
//            (!m_dummy && !iOther.m_dummy
//             && !m_joker && !iOther.m_joker
//             && m_char == iOther.m_char);
}


bool Tile::operator !=(const Tile &iOther) const
{
    return !(*this == iOther);
}

