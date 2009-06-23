/*****************************************************************************
 * Eliot
 * Copyright (C) 2007 Olivier Teulière
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
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

#include <cstring> // for strcpy
#include <string>
#include <sstream>
#include <iostream>

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

#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
#else
#   define _(String) String
#endif

#include "header.h"
#include "encoding.h"
#include "dic_exception.h"


#if defined(WORDS_BIGENDIAN)
// Nothing to do on big-endian machines
#   define ntohll(x) (x)
#   define htonll(x) (x)
#else
static inline uint64_t htonll(uint64_t host64)
{
    return (((uint64_t)htonl((host64 << 32) >> 32)) << 32) | htonl(host64 >> 32);
}

static inline uint64_t ntohll(uint64_t net64)
{
    return htonll(net64);
}
#endif


/**
 * Keyword included in dictionary headers
 * Implies little endian storage on words
 */
#define _COMPIL_KEYWORD_ "_COMPILED_DICTIONARY_"

/** Old format of the header (i.e. version 0) */
struct Dict_header_old
{
    /// Identification string
    char ident[sizeof(_COMPIL_KEYWORD_)];
    /// Version of the serialization format
    uint8_t version;
    /// Unused at the moment, reserved for further use
    char unused;
    uint32_t root;
    uint32_t nwords;
    /// Information about the compression
    //@{
    uint32_t edgesused;
    uint32_t nodesused;
    uint32_t nodessaved;
    uint32_t edgessaved;
    //@}
};

// Do not change these values, as they impact the size of the structure!
// Note: they are chosen carefully to avoid alignment issues
#define _MAX_USER_HOST_ 32
#define _MAX_DIC_NAME_SIZE_ 30
#define _MAX_LETTERS_NB_ 63
#define _MAX_LETTERS_SIZE_ 80

/** Extension of the old format (used in version 1)*/
struct Dict_header_ext
{
    // Time when the dictionary was compressed
    // (number of seconds since the Epoch)
    uint64_t compressDate;
    // Build information
    char userHost[_MAX_USER_HOST_];
    // Size taken by the build information
    uint32_t userHostSize;

    // --- we have a multiple of 64 bytes here

    // Compression algorithm (1 = DAWG, 2 = GADDAG)
    uint8_t algorithm;
    // Variant used in the rules (XXX: currently unused)
    uint8_t variant;

    // --- we have a multiple of 64 bytes here

    // Dictionary official name and version (e.g.: ODS 5.0)
    char dicName[_MAX_DIC_NAME_SIZE_];
    // Size taken by the dictionary name
    uint32_t dicNameSize;

    // --- we have a multiple of 64 bytes here

    // Letters used in the dictionary
    // We should have: nbLetters <= lettersSize <= _MAX_LETTERS_SIZE_
    // and:            nbLetters <= _MAX_LETTERS_NB_
    // The letters themselves, in UTF-8
    char letters[_MAX_LETTERS_SIZE_];
    // Size taken by the letters
    uint32_t lettersSize;
    // Number of letters (XXX: in theory useless, but allows a sanity check)
    uint32_t nbLetters;

    // --- we have a multiple of 64 bytes here

    // Points of the letters (indexed by their code)
    // The "+ 1" is there for struct alignment
    uint8_t points[_MAX_LETTERS_NB_ + 1];
    // Frequency of the letters (indexed by their code)
    // The "+ 1" is there for struct alignment
    uint8_t frequency[_MAX_LETTERS_NB_ + 1];
    // Bitfield indicating whether letters are vowels
    uint64_t vowels;
    // Bitfield indicating whether letters are consonants
    uint64_t consonants;

    // --- we have a multiple of 64 bytes here
};


Header::Header(istream &iStream)
    : m_root(0), m_nbWords(0), m_nodesUsed(0), m_edgesUsed(0),
      m_nodesSaved(0), m_edgesSaved(0), m_type(kDAWG)
{
    // Simply delegate to the read() method
    // The code is not moved here because I find it more natural to have a
    // read() method symmetrical to the write() one
    read(iStream);
    buildMapCodeFromChar();
}


Header::Header(const DictHeaderInfo &iInfo)
{
    // Use the latest serialization format
    m_version = 1;

    // Sanity checks
    if (iInfo.letters.size() > _MAX_LETTERS_NB_)
    {
        ostringstream ss;
        ss << _MAX_LETTERS_NB_;
        throw DicException("Header::Header: Too many different letters for "
                           "the current format; only " + ss.str() +
                           " are supported");
    }
    if (iInfo.points.size() != iInfo.letters.size())
    {
        throw DicException("Header::Header: Different sizes for "
                           "iInfo.points and iInfo.letters");
    }
    if (iInfo.frequency.size() != iInfo.letters.size())
    {
        throw DicException("Header::Header: Different sizes for "
                           "iInfo.frequency and iInfo.letters");
    }
    if (iInfo.vowels.size() != iInfo.letters.size())
    {
        throw DicException("Header::Header: Different sizes for "
                           "iInfo.vowels and iInfo.letters");
    }
    if (iInfo.consonants.size() != iInfo.letters.size())
    {
        throw DicException("Header::Header: Different sizes for "
                           "iInfo.consonants and iInfo.letters");
    }

    m_compressDate = time(NULL);
    m_userHost = convertToWc(ELIOT_COMPILE_BY + string("@") + ELIOT_COMPILE_HOST);
    m_root = iInfo.root;
    m_nbWords = iInfo.nwords;
    m_nodesUsed = iInfo.nodesused;
    m_edgesUsed = iInfo.edgesused;
    m_nodesSaved = iInfo.nodessaved;
    m_edgesSaved = iInfo.edgessaved;
    m_type = iInfo.dawg ? kDAWG : kGADDAG;
    m_dicName = iInfo.dicName;
    m_letters = iInfo.letters;
    m_points = iInfo.points;
    m_frequency = iInfo.frequency;
    m_vowels = iInfo.vowels;
    m_consonants = iInfo.consonants;

    buildMapCodeFromChar();
}


void Header::buildMapCodeFromChar()
{
    for (unsigned int i = 0; i < m_letters.size(); ++i)
    {
        // We don't differentiate uppercase and lowercase letters
        m_mapCodeFromChar[towlower(m_letters[i])] = i + 1;
        m_mapCodeFromChar[towupper(m_letters[i])] = i + 1;
    }
}


wchar_t Header::getCharFromCode(unsigned int iCode) const
{
    // Safety check
    if (iCode == 0 || iCode > m_letters.size())
    {
        ostringstream oss;
        oss << iCode;
        throw DicException("Header::getCharFromCode: no letter for code " + oss.str());
    }
    return m_letters[iCode - 1];
}


unsigned int Header::getCodeFromChar(wchar_t iChar) const
{
    map<wchar_t, unsigned int>::const_iterator pair =
        m_mapCodeFromChar.find(iChar);
    if (pair == m_mapCodeFromChar.end())
    {
        char s[5];
        sprintf(s, "%d", iChar);
        throw DicException("Header::getCodeFromChar: No code for letter '" +
                           convertToMb(iChar) + "' (val=" + string(s) + ")");
    }
    return pair->second;
}


wstring Header::getDisplayStr(unsigned int iCode) const
{
    // Safety check
    if (iCode == 0 || iCode > m_letters.size())
    {
        ostringstream oss;
        oss << iCode;
        throw DicException("Header::getDisplayStr: No code for letter '" + oss.str());
    }
    // TODO: return a const wstring & instead of a wstring
    return wstring(1, m_letters[iCode - 1]);
}


void Header::read(istream &iStream)
{
    Dict_header_old aHeader;
    iStream.read((char*)&aHeader, sizeof(Dict_header_old));
    if (iStream.gcount() != sizeof(Dict_header_old))
        throw DicException("Header::read: expected to read more bytes");

    // Check the identification string
    if (string(aHeader.ident) != _COMPIL_KEYWORD_)
        throw DicException("Header::read: incorrect header keyword; is it a dictionary file?");

    m_version = aHeader.version;

    // Version 0 corresponds to the dictionary format in the first Eliot
    // versions, supported until Eliot 1.8 (excluded).
    // The new version (version 1) was introduced in Eliot 1.6.
    if (m_version == 0)
    {
        throw DicException(_("Too old dictionary format. This format is not "
                             "supported anymore since Eliot 1.8. You can "
                             "create dictionaries in the new format with the "
                             "'compdic' tool provided with Eliot (since "
                             "version 1.6)."));
    }

    // Handle endianness
    m_root = ntohl(aHeader.root);
    m_nbWords = ntohl(aHeader.nwords);
    m_nodesUsed = ntohl(aHeader.nodesused);
    m_edgesUsed = ntohl(aHeader.edgesused);
    m_nodesSaved = ntohl(aHeader.nodessaved);
    m_edgesSaved = ntohl(aHeader.edgessaved);

    // After reading the old header, we now read the extension
    Dict_header_ext aHeaderExt;
    iStream.read((char*)&aHeaderExt, sizeof(Dict_header_ext));
    if (iStream.gcount() != sizeof(Dict_header_ext))
        throw DicException("Header::read: expected to read more bytes");

    // Handle endianness in the extension
    aHeaderExt.compressDate = ntohll(aHeaderExt.compressDate);
    aHeaderExt.userHostSize = ntohl(aHeaderExt.userHostSize);
    aHeaderExt.dicNameSize = ntohl(aHeaderExt.dicNameSize);
    aHeaderExt.lettersSize = ntohl(aHeaderExt.lettersSize);
    aHeaderExt.nbLetters = ntohl(aHeaderExt.nbLetters);
    aHeaderExt.vowels = ntohll(aHeaderExt.vowels);
    aHeaderExt.consonants = ntohll(aHeaderExt.consonants);

    m_compressDate = aHeaderExt.compressDate;

    if (aHeaderExt.algorithm == kDAWG)
        m_type = kDAWG;
    else if (aHeaderExt.algorithm == kGADDAG)
        m_type = kGADDAG;
    else
        throw DicException("Header::read: unrecognized algorithm type");

    m_userHost = readFromUTF8(aHeaderExt.userHost, aHeaderExt.userHostSize,
                              "user and host information");

    // Convert the dictionary letters from UTF-8 to wchar_t*
    m_dicName = readFromUTF8(aHeaderExt.dicName, aHeaderExt.dicNameSize,
                             "dictionary name");

    // Convert the dictionary letters from UTF-8 to wchar_t*
    m_letters = readFromUTF8(aHeaderExt.letters, aHeaderExt.lettersSize,
                             "dictionary letters");
    // Safety check: correct number of letters?
    if (m_letters.size() != aHeaderExt.nbLetters)
    {
        throw DicException("Header::read: inconsistent header");
    }

    // Letters points and frequency
    for (unsigned int i = 0; i < m_letters.size(); ++i)
    {
        m_points.push_back(aHeaderExt.points[i]);
        m_frequency.push_back(aHeaderExt.frequency[i]);
    }

    // Vowels and consonants
    for (unsigned int i = 0; i < m_letters.size(); ++i)
    {
        m_vowels.push_back(aHeaderExt.vowels & (1 << i));
        m_consonants.push_back(aHeaderExt.consonants & (1 << i));
    }
}


void Header::write(ostream &oStream) const
{
    Dict_header_old aHeader;
    strcpy(aHeader.ident, _COMPIL_KEYWORD_);
    aHeader.version = m_version;
    aHeader.unused = 0;
    aHeader.root = htonl(m_root);
    aHeader.nwords = htonl(m_nbWords);
    aHeader.nodesused = htonl(m_nodesUsed);
    aHeader.edgesused = htonl(m_edgesUsed);
    aHeader.nodessaved = htonl(m_nodesSaved);
    aHeader.edgessaved = htonl(m_edgesSaved);

    oStream.write((char*)&aHeader, sizeof(Dict_header_old));
    if (!oStream.good())
        throw DicException("Header::write: error when writing to file");

    Dict_header_ext aHeaderExt;
    aHeaderExt.compressDate = m_compressDate;
    aHeaderExt.userHostSize =
        writeInUTF8(m_userHost, aHeaderExt.userHost,
                    _MAX_USER_HOST_, "user and host information");
    aHeaderExt.algorithm = m_type;

    // Convert the dictionary name to UTF-8
    aHeaderExt.dicNameSize =
        writeInUTF8(m_dicName, aHeaderExt.dicName,
                    _MAX_DIC_NAME_SIZE_, "dictionary name");

    // Convert the dictionary letters to UTF-8
    aHeaderExt.lettersSize =
        writeInUTF8(m_letters, aHeaderExt.letters,
                    _MAX_LETTERS_SIZE_, "dictionary letters");
    aHeaderExt.nbLetters = (uint32_t)m_letters.size();

    // Letters points and frequency
    for (unsigned int i = 0; i < m_letters.size(); ++i)
    {
        aHeaderExt.points[i] = m_points[i];
        aHeaderExt.frequency[i] = m_frequency[i];
    }

    // Vowels and consonants
    aHeaderExt.vowels = 0;
    aHeaderExt.consonants = 0;
    for (unsigned int i = 0; i < m_letters.size(); ++i)
    {
        if (m_vowels[i])
            aHeaderExt.vowels |= 1 << i;
        if (m_consonants[i])
            aHeaderExt.consonants |= 1 << i;
    }

    // Handle endianness in the extension
    aHeaderExt.userHostSize = htonl(aHeaderExt.userHostSize);
    aHeaderExt.compressDate = htonll(aHeaderExt.compressDate);
    aHeaderExt.dicNameSize = htonl(aHeaderExt.dicNameSize);
    aHeaderExt.lettersSize = htonl(aHeaderExt.lettersSize);
    aHeaderExt.nbLetters = htonl(aHeaderExt.nbLetters);
    aHeaderExt.vowels = htonll(aHeaderExt.vowels);
    aHeaderExt.consonants = htonll(aHeaderExt.consonants);

    // Write the extension
    oStream.write((char*)&aHeaderExt, sizeof(Dict_header_ext));
    if (!oStream.good())
        throw DicException("Header::write: error when writing to file");
}


void Header::print() const
{
    printf(_("dictionary name: %s\n"), convertToMb(m_dicName).c_str());
    char buf[50];
    strftime(buf, sizeof(buf), "%c", gmtime(&m_compressDate));
    printf(_("compressed on: %s\n"), buf);
    printf(_("compressed using a binary compiled by: %s\n"), convertToMb(m_userHost).c_str());
    printf(_("dictionary type: %s\n"), m_type == kDAWG ? "DAWG" : "GADDAG");
    printf(_("letters: %s\n"), convertToMb(m_letters).c_str());
    printf(_("number of letters: %lu\n"), (long unsigned int)m_letters.size());
    printf(_("number of words: %d\n"), m_nbWords);
    long unsigned int size = sizeof(Dict_header_old) + sizeof(Dict_header_ext);
    printf(_("header size: %lu bytes\n"), size);
    printf(_("root: %d (edge)\n"), m_root);
    printf(_("nodes: %d used + %d saved\n"), m_nodesUsed, m_nodesSaved);
    printf(_("edges: %d used + %d saved\n"), m_edgesUsed, m_edgesSaved);
    printf("===============================================\n");
    printf(_("letter | points | frequency | vowel | consonant\n"));
    printf("-------+--------+-----------+-------+----------\n");
    for (unsigned int i = 0; i < m_letters.size(); ++i)
    {
        printf("  %s   |   %2d   |    %2d     |   %d   |    %d\n",
               padAndConvert(wstring(1, m_letters[i]), 2).c_str(),
               m_points[i], m_frequency[i], m_vowels[i], m_consonants[i]);
    }
    printf("===============================================\n");
}

