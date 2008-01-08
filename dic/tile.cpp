/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2007 Olivier Teulière & Antoine Fraboulet
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
 *          Antoine Fraboulet <antoine.fraboulet @@ free.fr>
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

#include <sstream>
#include <string>
#include <wctype.h>
#include "tile.h"
#include "header.h"
#include "encoding.h"
#include "dic_exception.h"


const Header * Tile::m_header = NULL;
Tile Tile::m_TheJoker;


Tile::Tile(wchar_t c)
{
    if (iswalpha(c))
    {
        m_joker = iswlower(c);
        m_char = towupper(c);
        m_code = m_header->getCodeFromChar(m_char);
    }
    else if (c == kTILE_JOKER)
    {
        m_joker = true;
        m_char = kTILE_JOKER;
        m_code = m_header->getCodeFromChar(m_char);
    }
    else if (c == kTILE_DUMMY)
    {
        // The char and the code are chosen to be different from any possible
        // real tile
        m_joker = false;
        m_char = kTILE_DUMMY;
        m_code = 0;
    }
    else
    {
        ostringstream ss;
        ss << "Tile::Tile: Unknown character: " << convertToMb(c);
        throw DicException(ss.str());
    }
}


bool Tile::isVowel() const
{
    if (m_code == 0)
        throw DicException("Tile::isVowel: Invalid tile");
    return m_header->isVowel(m_code);
}


bool Tile::isConsonant() const
{
    if (m_code == 0)
        throw DicException("Tile::isConsonant: Invalid tile");
    return m_header->isConsonant(m_code);
}


unsigned int Tile::maxNumber() const
{
    if (m_code == 0)
        throw DicException("Tile::maxNumber: Invalid tile");
    return m_header->getFrequency(m_code);
}


unsigned int Tile::getPoints() const
{
    if (m_code == 0)
        throw DicException("Tile::getPoints: Invalid tile");
    return m_header->getPoints(m_code);
}


wchar_t Tile::toChar() const
{
    if (m_joker)
    {
        if (iswalpha(m_char))
            return towlower(m_char);
        else
            return kTILE_JOKER;
    }
    return m_char;
}


unsigned int Tile::toCode() const
{
    return m_code;
}


bool Tile::operator<(const Tile &iOther) const
{
    if (m_joker)
        return false;
    else if (iOther.m_joker)
        return true;
    else
        return m_char < iOther.m_char;
}


bool Tile::operator==(const Tile &iOther) const
{
    if (m_joker || iOther.m_joker)
    {
        if (m_joker != iOther.m_joker)
            return false;
        return m_char == iOther.m_char;
    }
    return m_char == iOther.m_char;
}


bool Tile::operator!=(const Tile &iOther) const
{
    return !(*this == iOther);
}

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
