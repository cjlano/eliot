/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2012 Olivier Teulière & Antoine Fraboulet & Olivier Teulière
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
#include <cwctype>
#include "tile.h"
#include "header.h"
#include "encoding.h"
#include "dic_exception.h"
#include "debug.h"


using namespace std;


INIT_LOGGER(dic, Tile);


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
        ss << "Tile::Tile: Unknown character: " << lfw(c);
        throw DicException(ss.str());
    }
}


Tile::Tile(unsigned int iCode, bool isJoker)
{
    m_joker = isJoker;
    m_code = iCode;
    m_char = m_header->getCharFromCode(iCode);
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


wstring Tile::getDisplayStr() const
{
    if (m_code == 0)
        throw DicException("Tile::getDisplayStr: Invalid tile");
    if (m_joker && iswalpha(m_char))
    {
        return ::toLower(m_header->getDisplayStr(m_code));
    }
    return m_header->getDisplayStr(m_code);
}


vector<wistring> Tile::getInputStr() const
{
    if (m_code == 0)
        throw DicException("Tile::getInputStr: Invalid tile");
    // We always return uppercase strings
    return m_header->getInputStr(m_code);
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


Tile Tile::toLower() const
{
    ASSERT(iswalpha(m_char),
           "toLower() should be called on alphabetical tiles");
    return Tile(towlower(m_char));
}


Tile Tile::toUpper() const
{
    ASSERT(iswalpha(m_char),
           "toUpper() should be called on alphabetical tiles");
    return Tile(towupper(m_char));
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
    return m_char == iOther.m_char
        && m_joker == iOther.m_joker;
}


bool Tile::operator!=(const Tile &iOther) const
{
    return !(*this == iOther);
}

void Tile::SetHeader(const Header &iHeader)
{
    m_header = &iHeader;

    // The joker tile depends on the dictionary,
    // because its code may be different
    // But since it might be valid to play without jokers,
    // we first check if the dictionary contains a joker.
    if (m_header->getLetters().find(kTILE_JOKER) != wstring::npos)
        Tile::m_TheJoker = Tile(kTILE_JOKER);
}
