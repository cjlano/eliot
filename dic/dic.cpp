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


// Note: duplicated in header.cpp
#if defined(WORDS_BIGENDIAN)
static uint32_t swap4(uint32_t v)
{
    uint32_t r;
    uint8_t *pv = (uint8_t*)&v;
    uint8_t *pr = (uint8_t*)&r;

    pr[0] = pv[3];
    pr[1] = pv[2];
    pr[2] = pv[1];
    pr[3] = pv[0];

    return r;
}
#endif


Dictionary::Dictionary(const string &iPath)
    : m_dawg(NULL)
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

    m_dic = this;
}


Dictionary::~Dictionary()
{
    delete[] m_dawg;
    delete m_header;
}


void Dictionary::convertDataToArch()
{
    if (m_header->getVersion() == 0)
    {
#if defined(WORDS_BIGENDIAN)
        for (unsigned int i = 0; i < (m_header->getNbEdgesUsed() + 1); i++)
        {
            m_dawg[i] = swap4(m_dawg[i]);
        }
#endif
    }
    else
    {
        for (unsigned int i = 0; i < (m_header->getNbEdgesUsed() + 1); i++)
        {
            m_dawg[i] = ntohl(m_dawg[i]);
        }
    }
}


void Dictionary::initializeTiles()
{
    // "Activate" the dictionary by giving the header to the Tile class
    Tile::SetHeader(*m_header);

    // XXX: temp
    Tile::m_TheJoker = Tile(Tile::kTILE_JOKER);

    m_tilesVect.reserve(m_header->getLetters().size() + 1);
    // Create a tile for each letter in the dictionary header
    for (unsigned int i = 0; i < m_header->getLetters().size(); ++i)
        m_tilesVect.push_back(Tile(m_header->getLetters()[i]));
}


bool Dictionary::validateLetters(const wstring &iLetters,
                                 const wstring &iAccepted) const
{
    return iLetters.empty()
        || iLetters.find_first_not_of(m_allLetters + iAccepted) == string::npos;
}


dic_elt_t Dictionary::getNext(const dic_elt_t &e) const
{
     if (!isLast(e))
         return e + 1;
     return 0;
}


dic_elt_t Dictionary::getSucc(const dic_elt_t &e) const
{
    if (m_header->getVersion() == 0)
        return reinterpret_cast<const DicEdgeOld*>(m_dawg + e)->ptr;
    else
        return reinterpret_cast<const DicEdge*>(m_dawg + e)->ptr;
}


dic_elt_t Dictionary::getRoot() const
{
    return m_header->getRoot();
}


dic_code_t Dictionary::getCode(const dic_elt_t &e) const
{
    if (m_header->getVersion() == 0)
        return reinterpret_cast<const DicEdgeOld*>(m_dawg + e)->chr;
    else
        return reinterpret_cast<const DicEdge*>(m_dawg + e)->chr;
}


wchar_t Dictionary::getChar(const dic_elt_t &e) const
{
    return m_header->getCharFromCode(getCode(e));
}


bool Dictionary::isLast(const dic_elt_t &e) const
{
    if (m_header->getVersion() == 0)
        return reinterpret_cast<const DicEdgeOld*>(m_dawg + e)->last;
    else
        return reinterpret_cast<const DicEdge*>(m_dawg + e)->last;
}


bool Dictionary::isEndOfWord(const dic_elt_t &e) const
{
    if (m_header->getVersion() == 0)
        return reinterpret_cast<const DicEdgeOld*>(m_dawg + e)->term;
    else
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

