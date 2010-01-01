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

#include "config.h"

#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cctype>
#include <boost/foreach.hpp>

// For ntohl & Co.
#ifdef WIN32
#   include <winsock2.h>
#else
#    if HAVE_NETINET_IN_H
#       include <netinet/in.h>
#    endif
#    if HAVE_ARPA_INET_H
#       include <arpa/inet.h>
#    endif
#endif

#include "dic.h"
#include "header.h"
#include "dic_exception.h"
#include "dic_internals.h"
#include "tile.h"


const Dictionary *Dictionary::m_dic = NULL;


Dictionary::Dictionary(const string &iPath)
    : m_dawg(NULL), m_hasDisplay(false)
{
    ifstream file(iPath.c_str(), ios::in | ios::binary);

    if (!file.is_open())
        throw DicException("Cannot open " + iPath);

    // XXX: we should protect these allocations with auto_ptr
    m_header = new Header(file);
    m_dawg = new uint32_t[m_header->getNbEdgesUsed() + 1];

    streamsize toRead = (m_header->getNbEdgesUsed() + 1) * sizeof(uint32_t);
    file.read((char*)m_dawg, toRead);
    if (file.gcount() != toRead)
    {
        delete[] m_dawg;
        delete m_header;
        throw DicException("Problem reading dictionary arcs");
    }

    // Handle endianness
    convertDataToArch();

    initializeTiles();

    // Concatenate the uppercase and lowercase letters
    wstring lower = m_header->getLetters();
    std::transform(lower.begin(), lower.end(), lower.begin(), towlower);
    m_allLetters = m_header->getLetters() + lower;

    // Same for the input characters
    lower = m_header->getInputChars();
    std::transform(lower.begin(), lower.end(), lower.begin(), towlower);
    m_allInputChars = m_header->getInputChars() + lower;

    // Build the cache for the convertToDisplay() and convertFromInput()
    // methods.
    map<wchar_t, vector<wstring> >::const_iterator it;
    for (it = m_header->getDisplayInputData().begin();
         it != m_header->getDisplayInputData().end(); ++it)
    {
        // Save both the upper case and lower case versions
        BOOST_FOREACH(wstring str, it->second)
        {
            // Make sure the string is in uppercase
            std::transform(str.begin(), str.end(), str.begin(), towupper);
            // Make a lowercase copy
            wstring lower = str;
            std::transform(lower.begin(), lower.end(), lower.begin(), towlower);
            // Fill the cache
            m_displayInputCache[towupper(it->first)].push_back(str);
            m_displayInputCache[towlower(it->first)].push_back(lower);
        }

        // Update the m_hasDisplay flag
        if (!m_hasDisplay && it->second[0] != wstring(1, it->first))
            m_hasDisplay = true;
    }


    m_dic = this;
}


Dictionary::~Dictionary()
{
    delete[] m_dawg;
    delete m_header;
}


void Dictionary::convertDataToArch()
{
    for (unsigned int i = 0; i < (m_header->getNbEdgesUsed() + 1); i++)
    {
        m_dawg[i] = ntohl(m_dawg[i]);
    }
}


void Dictionary::initializeTiles()
{
    // "Activate" the dictionary by giving the header to the Tile class
    Tile::SetHeader(*m_header);

    m_tilesVect.reserve(m_header->getLetters().size() + 1);
    // Create a tile for each letter in the dictionary header
    for (unsigned int i = 0; i < m_header->getLetters().size(); ++i)
    {
        const wchar_t &chr = m_header->getLetters()[i];
        unsigned int code = m_header->getCodeFromChar(chr);
        m_tilesVect.push_back(Tile(code, chr == Tile::kTILE_JOKER));
    }
}


bool Dictionary::validateLetters(const wstring &iLetters,
                                 const wstring &iAccepted) const
{
    return iLetters.empty()
        || iLetters.find_first_not_of(m_allLetters + iAccepted) == string::npos;
}


bool Dictionary::validateInputChars(const wistring &iLetters,
                                    const wistring &iAccepted) const
{
    return iLetters.empty()
        || iLetters.find_first_not_of(m_allInputChars + iAccepted) == string::npos;
}


wdstring Dictionary::convertToDisplay(const wstring &iWord) const
{
    // Optimization for dictionaries without display nor input chars,
    // which is the case in most languages.
    if (!m_hasDisplay)
        return iWord;

    wdstring dispStr = iWord;
    map<wchar_t, vector<wstring> >::const_iterator it;
    for (it = m_displayInputCache.begin();
         it != m_displayInputCache.end(); ++it)
    {
        const wstring &disp = it->second[0];
        string::size_type pos = 0;
        while (pos < dispStr.size() &&
               (pos = dispStr.find(it->first, pos)) != string::npos)
        {
            dispStr.replace(pos, 1, disp);
            pos += disp.size();
        }
    }
    return dispStr;
}


wstring Dictionary::convertFromInput(const wistring &iWord) const
{
    // Optimization for dictionaries without display nor input chars,
    // which is the case in most languages.
    if (m_displayInputCache.empty())
        return iWord;

    wstring str = iWord;
    map<wchar_t, vector<wstring> >::const_iterator it;
    for (it = m_displayInputCache.begin();
         it != m_displayInputCache.end(); ++it)
    {
        BOOST_FOREACH(const wstring &input, it->second)
        {
            string::size_type pos = 0;
            while (pos < str.size() &&
                   (pos = str.find(input, pos)) != string::npos)
            {
                str.replace(pos, input.size(), wstring(1, it->first));
                pos += input.size();
            }
        }
    }
    return str;
}


dic_elt_t Dictionary::getNext(const dic_elt_t &e) const
{
     if (!isLast(e))
         return e + 1;
     return 0;
}


dic_elt_t Dictionary::getSucc(const dic_elt_t &e) const
{
    return reinterpret_cast<const DicEdge*>(m_dawg + e)->ptr;
}


dic_elt_t Dictionary::getRoot() const
{
    return m_header->getRoot();
}


dic_code_t Dictionary::getCode(const dic_elt_t &e) const
{
    return reinterpret_cast<const DicEdge*>(m_dawg + e)->chr;
}


wchar_t Dictionary::getChar(const dic_elt_t &e) const
{
    return m_header->getCharFromCode(getCode(e));
}


bool Dictionary::isLast(const dic_elt_t &e) const
{
    return reinterpret_cast<const DicEdge*>(m_dawg + e)->last;
}


bool Dictionary::isEndOfWord(const dic_elt_t &e) const
{
    return reinterpret_cast<const DicEdge*>(m_dawg + e)->term;
}


unsigned int Dictionary::lookup(const dic_elt_t &root, const dic_code_t *s) const
{
    unsigned int p;
    dic_elt_t rootCopy = root;
begin:
    if (! *s)
        return rootCopy;
    if (! getSucc(rootCopy))
        return 0;
    p = getSucc(rootCopy);
    do
    {
        if (getCode(p) == *s)
        {
            rootCopy = p;
            s++;
            goto begin;
        }
        else if (isLast( p))
        {
            return 0;
        }
        p = getNext(p);
    } while (1);

    return 0;
}


unsigned int Dictionary::charLookup(const dic_elt_t &iRoot, const wchar_t *s) const
{
    unsigned int p;
    dic_elt_t rootCopy = iRoot;
begin:
    if (! *s)
        return rootCopy;
    if (! getSucc(rootCopy))
        return 0;
    p = getSucc(rootCopy);
    do
    {
        if (getChar(p) == *s)
        {
            rootCopy = p;
            s++;
            goto begin;
        }
        else if (isLast(p))
        {
            return 0;
        }
        p = getNext(p);
    } while (1);

    return 0;
}

