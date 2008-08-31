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

/**
 *  \file   dic.h
 *  \brief  Dawg dictionary
 *  \author Antoine Fraboulet & Olivier Teuliere
 *  \date   2002
 */

#ifndef _DIC_H_
#define _DIC_H_

#include <string>
#include <vector>
#include <map>

#include "tile.h"

using namespace std;


/**
 * max length of words (including last \0)
 */
#define DIC_WORD_MAX 16

class Header;
typedef unsigned int dic_elt_t;
typedef unsigned char dic_code_t;
struct params_cross_t;
struct params_7plus1_t;
struct params_regexp_t;
struct search_RegE_list_t;

class Dictionary
{
public:
    /**
     * Dictionary creation and loading from a file
     * @param path: compressed dictionary path
     */
    Dictionary(const string &path);

    /// Destructor
    ~Dictionary();

    /**
     * Return the current instance of the dictionary object
     * XXX: This is ugly, but I don't see any clean way apart from carrying
     * a reference to a Dictionary object in many places...
     * Other more or less ugly options:
     *  - Make the dictionary a singleton (2 dictionaries cannot coexist...)
     *  - Make many classes inherit from a common base class with a dictionary
     *    member (possibly bad for performances)
     * A new created dictionary replaces the previous instance, even if the
     * previous instance is not destroyed yet
     * If no dictionary object is instanciated when this method is called,
     * it will probably crash...
     */
    static const Dictionary& GetDic() { return *m_dic; }

    /** Give access to the dictionary header */
    const Header& getHeader() const { return *m_header; }

    /**
     * Check whether all the given letters are present in the dictionary,
     * or are one of the other accepted letters.
     * Return true if this is the case, false otherwise
     */
    bool validateLetters(const wstring &iLetters,
                         const wstring &iAccepted = L"") const;

    /** Return a vector containing one of each possible tile */
    const vector<Tile>& getAllTiles() const { return m_tilesVect; }

    /** Return the number of different tiles (including the joker) */
    unsigned int getTileNumber() const { return m_tilesVect.size(); }

    /** Return a tile from its code */
    const Tile &getTileFromCode(unsigned int iCode) const { return m_tilesVect[iCode - 1]; }

    /**
     * Returns the character code associated with an element,
     * codes may range from 0 to 63. 0 is the null character.
     * @returns code for the encoded character
     */
    dic_code_t getCode(const dic_elt_t &elt) const;

    /**
     * Returns the wide character associated with an element.
     * @returns wide character for the element
     */
    wchar_t getChar(const dic_elt_t &elt) const;

    /**
     * Returns a boolean to show if there is another available
     * character in the current depth (a neighbor in the tree)
     * @return true if the character is the last one at the current depth
     */
    bool isLast(const dic_elt_t &elt) const;

    /**
     * Returns a boolean to show if we are at the end of a word
     * (see getNext())
     * @return true if this is the end of a word
     */
    bool isEndOfWord(const dic_elt_t &elt) const;

    /**
     * Returns the root of the dictionary
     * @returns root element
     */
    dic_elt_t getRoot() const;

    /**
     * Returns the next available neighbor (see isLast())
     * @returns next dictionary element at the same depth
     */
    dic_elt_t getNext(const dic_elt_t &elt) const;

    /**
     * Returns the first element available at the next depth
     * in the dictionary
     * @params elt : current dictionary element
     * @returns next element (successor)
     */
    dic_elt_t getSucc(const dic_elt_t &elt) const;

    /**
     * Find the dictionary element matching the pattern starting
     * from the given root node by walking the dictionary tree
     * @params root : starting dictionary node for the search
     * @params pattern : string encoded according to the dictionary codes,
     * the pattern must be null ('\0') terminated
     * @returns 0 if the string cannot be matched otherwise returns the
     * element that results from walking the dictionary according to the
     * pattern
     */
    unsigned int lookup(const dic_elt_t &root, const dic_code_t *pattern) const;

    /**
     * Find the dictionary element matching the pattern starting
     * from the given root node by walking the dictionary tree
     * @params root : starting dictionary node for the search
     * @params pattern : string made of uppercase characters in the range
     * ['A'-'Z']. The pattern must be null ('\0') terminated
     * @returns 0 if the string cannot be matched otherwise returns the
     * element that results from walking the dictionary according to the
     * pattern
     */
    unsigned int charLookup(const dic_elt_t &iRoot, const wchar_t *iPattern) const;

    /// Getter for the edge at the given position
    const uint32_t *getEdgeAt(const dic_elt_t &iElt) const { return m_dawg + iElt; }

    /**
     * Search for a word in the dictionary
     * @param iWord: lookup word
     * @return true if the word is valid, false otherwise
     */
    bool searchWord(const wstring &iWord) const;

    /**
     * Search for benjamins
     * @param iWord: letters
     * @param oWordList: results
     * @param iMaxResults: maximum number of returned results (0 means no limit)
     */
    void searchBenj(const wstring &iWord, vector<wstring> &oWordList,
                    unsigned int iMaxResults = 0) const;

    /**
     * Search for all words feasible by adding a letter in front or at the end
     * @param iWord: word
     * @param oWordList: results
     * @param iMaxResults: maximum number of returned results (0 means no limit)
     */
    void searchRacc(const wstring &iWord, vector<wstring> &oWordList,
                    unsigned int iMaxResults = 0) const;

    /**
     * Search for crosswords
     * @param iMask: letters
     * @param oWordList: results
     * @param iMaxResults: maximum number of returned results (0 means no limit)
     */
    void searchCross(const wstring &iMask, vector<wstring> &oWordList,
                     unsigned int iMaxResults = 0) const;

    /**
     * Search for all feasible word with "rack" plus one letter
     * @param iRack: letters
     * @param oWordlist: results
     * @param joker: true if the search must be performed when a joker is in the rack
     * @param iMaxResults: maximum number of returned results (0 means no limit)
     */
    void search7pl1(const wstring &iRack,
                    map<wchar_t, vector<wstring> > &oWordList,
                    bool joker) const;

    /**
     * Search for words matching a regular expression
     * @param iRegexp: regular expression
     * @param oWordList: results
     * @param iList: parameters for the search (?)
     * @param iMaxResults: maximum number of returned results (0 means no limit)
     * @return true if all the matching words were returned, false otherwise
     *      (i.e. if the maximum number of results was reached, and there are
     *      additional results)
     * @throw InvalidRegexpException When the regular expression cannot be parsed
     */
    bool searchRegExp(const wstring &iRegexp,
                      vector<wstring> &oWordList,
                      unsigned int iMinLength,
                      unsigned int iMaxLength,
                      unsigned int iMaxResults = 0) const;


private:
    // Prevent from copying the dictionary!
    Dictionary &operator=(const Dictionary&);
    Dictionary(const Dictionary&);

    Header *m_header;
    uint32_t *m_dawg;

    /** Letters of the dictionary, both in uppercase and lowercase */
    wstring m_allLetters;

    /// Vector of available tiles
    vector<Tile> m_tilesVect;

    static const Dictionary *m_dic;

    void convertDataToArch();
    void initializeTiles();

    /// Template getter for the edge at the given position
    template <typename DAWG_EDGE>
    const DAWG_EDGE * getEdgeAt(const dic_elt_t &iElt) const
    {
        return reinterpret_cast<const DAWG_EDGE*>(m_dawg + iElt);
    }

    /**
     * Walk the dictionary until the end of the word
     * @param s: current pointer to letters
     * @param eptr: current edge in the dawg
     */
    template <typename DAWG_EDGE>
    const DAWG_EDGE * seekEdgePtr(const wchar_t *s, const DAWG_EDGE *eptr) const;

    /// Helper for searchBenj()
    template <typename DAWG_EDGE>
    void searchBenjTempl(const wstring &iWord, vector<wstring> &oWordList,
                         unsigned int iMaxResults) const;

    /// Helper for searchRacc()
    template <typename DAWG_EDGE>
    void searchRaccTempl(const wstring &iWord, vector<wstring> &oWordList,
                         unsigned int iMaxResults) const;

    /// Helper for searchCross()
    template <typename DAWG_EDGE>
    void searchCrossRecTempl(struct params_cross_t *params,
                             vector<wstring> &oWordList,
                             const DAWG_EDGE *edgeptr,
                             unsigned int iMaxResults) const;

    /// Helper for search7pl1()
    template <typename DAWG_EDGE>
    void search7pl1Templ(const wstring &iRack,
                         map<wchar_t, vector<wstring> > &oWordList,
                         bool joker) const;

    /// Second helper for search7pl1()
    template <typename DAWG_EDGE>
    void searchWordByLen(struct params_7plus1_t *params,
                         int i, const DAWG_EDGE *edgeptr) const;

    /// Helper for searchRegExp()
    template <typename DAWG_EDGE>
    void searchRegexpRecTempl(struct params_regexp_t *params,
                              int state,
                              const DAWG_EDGE *edgeptr,
                              vector<wstring> &oWordList,
                              unsigned int iMaxResults) const;
};

#endif /* _DIC_H_ */

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
