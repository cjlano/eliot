/*****************************************************************************
 * Eliot
 * Copyright (C) 2007-2009 Olivier Teulière
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

#ifndef HEADER_H_
#define HEADER_H_

#include <iosfwd>
#include <map>
#include <vector>
#include <time.h>
#include <stdint.h>

#include "logging.h"

using namespace std;

// XXX: duplicated typedef (also present in dic.h)
typedef wstring wdstring;
typedef wstring wistring;


/**
 * Structure used to create a Header object.
 * Note: this structure doesn't pretend to map the way the data is stored in a
 * file. For (de)serialization, always use a Header object
 */
struct DictHeaderInfo
{
    uint32_t root;
    uint32_t nwords;
    uint32_t edgesused;
    uint32_t nodesused;
    uint32_t nodessaved;
    uint32_t edgessaved;
    bool dawg;
    wstring dicName;
    wstring letters;
    vector<uint8_t> points;
    vector<uint8_t> frequency;
    vector<bool> vowels;
    vector<bool> consonants;
    map<wchar_t, vector<wstring> > displayInputData;
};


/**
 * Dictionary header.
 * There are 2 ways to create a Header object:
 *  - fill a DictHeaderInfo structure and give it in the constructor of the
 *    Header class (usually to call write() afterwards)
 *  - give it an input stream on a compiled dictionary
 *
 * The class is immutable: a Header object cannot be modified after creation.
 *
 * Serialization:
 * Several formats of headers will be handled by this class, even though only
 * the first one is handled at the moment. You can use the write() method to
 * write the latest version of the header into a given file.
 * When using the constructor taking an input stream, all the header versions
 * are supported.
 */
class Header
{
    DEFINE_LOGGER();
public:

    /// Dictionary type
    enum DictType
    {
        kDAWG = 1,
        kGADDAG = 2
    };

    /**
     * Constructor from an input stream
     * @param iStream: Input stream where to read the header
     */
    Header(istream &iStream);

    /**
     * Constructor from a filled structure
     * @param iInfo: info needed to build the header
     */
    Header(const DictHeaderInfo &iInfo);

    /// Return the version of the dictionary format
    uint8_t getVersion() const { return m_version; }

    /// Getters
    //@{
    unsigned int getRoot()         const { return m_root; }
    unsigned int getNbWords()      const { return m_nbWords; }
    unsigned int getNbNodesUsed()  const { return m_nodesUsed; }
    unsigned int getNbEdgesUsed()  const { return m_edgesUsed; }
    unsigned int getNbNodesSaved() const { return m_nodesSaved; }
    unsigned int getNbEdgesSaved() const { return m_edgesSaved; }
    wstring      getName()         const { return m_dicName; }
    DictType     getType()         const { return m_type; }
    wstring      getLetters()      const { return m_letters; }
    wstring      getInputChars()   const { return m_inputChars; }
    unsigned int getMinCode()      const { return 1; }
    unsigned int getMaxCode()      const { return m_letters.size(); }
    uint8_t      getPoints(unsigned int iCode) const { return m_points[iCode - 1]; }
    uint8_t      getFrequency(unsigned int iCode) const { return m_frequency[iCode - 1]; }
    bool         isVowel(unsigned int iCode) const { return m_vowels[iCode - 1]; }
    bool         isConsonant(unsigned int iCode) const { return m_consonants[iCode - 1]; }
    //@}

    const map<wchar_t, vector<wstring> > & getDisplayInputData() const { return m_displayAndInputData; }

    /**
     * Return the letter corresponding to the given code
     */
    wchar_t getCharFromCode(unsigned int iCode) const;

    /**
     * Return the code corresponding to the given letter
     */
    unsigned int getCodeFromChar(wchar_t iChar) const;

    /**
     * Return the display string corresponding to the given code
     */
    const wdstring & getDisplayStr(unsigned int iCode) const;

    /**
     * Return all the accepted input strings corresponding to the given code
     */
    vector<wistring> getInputStr(unsigned int iCode) const;

    /**
     * Print a readable summary of the header on standard output
     */
    void print() const;

    /**
     * Write the header to a file, using the latest format
     * @param oStream: Output stream where to write the header
     * @exception: Throw a DicException in case of problem
     */
    void write(ostream &oStream) const;

private:
    /// Version of the serialization
    uint8_t m_version;

    wstring m_userHost;
    time_t m_compressDate;

    uint32_t m_root;
    uint32_t m_nbWords;
    uint32_t m_nodesUsed;
    uint32_t m_edgesUsed;
    uint32_t m_nodesSaved;
    uint32_t m_edgesSaved;

    /// Specify whether the dictionary is a DAWG or a GADDAG
    DictType m_type;

    /// Dictionary name (e.g.: ODS 5.0)
    wstring m_dicName;

    /// (Internal) letters constituting the words of the dictionary
    wstring m_letters;

    /// Characters usable to input the dictionary letters
    wstring m_inputChars;

    /// Points of the letters
    vector<uint8_t> m_points;

    /// Frequency of the letters
    vector<uint8_t> m_frequency;

    /// Vowels
    vector<bool> m_vowels;

    /// Consonants
    vector<bool> m_consonants;

    /// Additional display and input strings for some letters
    map<wchar_t, vector<wstring> > m_displayAndInputData;

    /// Cache for the char --> code associations
    map<wchar_t, unsigned int> m_mapCodeFromChar;

    /// Cache for the display string of each code
    vector<wdstring> m_displayCache;

    /**
     * Load the header from a file
     * @param iStream: Input stream where to read the header
     * @exception: Throw a DicException in case of problem
     */
    void read(istream &iStream);

    /** Build various caches */
    void buildCaches();

    /**
     * Fill the m_displayAndInputData field from the serialized data
     * of the given string
     */
    void readDisplayAndInput(const wstring &serialized);

    /**
     * Return a serialized version of the data contained in the
     * m_displayAndInputData field
     */
    wstring writeDisplayAndInput() const;
};

#endif /* _HEADER_H */

