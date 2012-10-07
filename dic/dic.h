/*****************************************************************************
 * Eliot
 * Copyright (C) 2002-2012 Antoine Fraboulet & Olivier Teulière
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

#ifndef DIC_H_
#define DIC_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

#include "tile.h"
#include "logging.h"

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
class DicEdge;

/**
 * A wdstring is a display string, i.e. it can contain more chars than
 * the represented string. The difference arises in languages such as Catalan,
 * where for example "QU" is made of 2 real characters, but corresponds to a
 * single tile.
 *
 * The wdstring type has no particular interest other than signaling
 * a bit more precisely the type of contents of the string.
 */
typedef wstring wdstring;
typedef wstring wistring;

class Dictionary
{
    DEFINE_LOGGER();
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

    /**
     * Check whether all the given characters can be part of an input
     * form for one or more dictionary letter, or are one of the other
     * accepted letters.
     * Return true if this is the case, false otherwise
     */
    bool validateInputChars(const wstring &iLetters,
                            const wstring &iAccepted = L"") const;

    /**
     * Convert the given string (made of internal characters)
     * into a string suitable for display
     */
    wdstring convertToDisplay(const wstring &iWord) const;

    /**
     * Convert the given string (direct user input)
     * into a string suitable for internal use in Eliot.
     * For example, in Catalan, it will convert the L.L substring
     * into the W character (because it is the internal character
     * associated to the input string "L.L").
     */
    wstring convertFromInput(const wistring &iWord) const;

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
    const DicEdge * getEdgeAt(const dic_elt_t &iElt) const
    {
        return reinterpret_cast<const DicEdge*>(m_dawg + iElt);
    }


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
    void searchBenj(const wstring &iWord, vector<wdstring> &oWordList,
                    unsigned int iMaxResults = 0) const;

    /**
     * Search for all words feasible by adding a letter in front or at the end
     * @param iWord: word
     * @param oWordList: results
     * @param iMaxResults: maximum number of returned results (0 means no limit)
     */
    void searchRacc(const wstring &iWord, vector<wdstring> &oWordList,
                    unsigned int iMaxResults = 0) const;

    /**
     * Search for all feasible word with "rack" plus one letter
     * XXX: the key in the map is the internal code, because it allows an easy
     * iteration in the map in the order of the dictionary letters.
     * Maybe a more powerful structure should be provided, to hide the internal
     * chars to the caller.
     *
     * @param iRack: letters
     * @param oWordlist: results (grouped by code of the added character)
     * @param joker: true if the search must be performed when a joker is in the rack
     * @param iMaxResults: maximum number of returned results (0 means no limit)
     */
    void search7pl1(const wstring &iRack,
                    map<unsigned int, vector<wdstring> > &oWordList,
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
                      vector<wdstring> &oWordList,
                      unsigned int iMinLength,
                      unsigned int iMaxLength,
                      unsigned int iMaxResults = 0) const;

private:
    // Prevent from copying the dictionary!
    Dictionary &operator=(const Dictionary&);
    Dictionary(const Dictionary&);

    Header *m_header;
    uint32_t *m_dawg;

    /**
     * Letters of the dictionary, both in uppercase and lowercase
     * (internal representation)
     */
    wstring m_allLetters;

    /**
     * All the possible characters possibly used to input letters of
     * the dictionary, both in uppercase and lowercase
     */
    wstring m_allInputChars;

    /// Vector of available tiles
    vector<Tile> m_tilesVect;

    /**
     * Associate to some internal chars (both the lower case and
     * upper case versions) all the corresponding input strings.
     * The first one is always the display string.
     *
     * Note: only the chars which have more than 1 input string,
     * or which have a display string different from the internal char,
     * are present in the map.
     */
    map<wchar_t, vector<wstring> > m_displayInputCache;

    /**
     * True if at least one display strings is different from the internal
     * char, false otherwise. This flag is more precise than checking the size
     * of m_displayInputCache, because a tile can have input strings even if
     * its display string is equal to the internal char.
     */
    bool m_hasDisplay;

    static const Dictionary *m_dic;

    void convertDataToArch();
    void initializeTiles();

    /**
     * Walk the dictionary until the end of the word
     * @param s: current pointer to letters
     * @param eptr: current edge in the dawg
     */
    const DicEdge * seekEdgePtr(const wchar_t *s, const DicEdge *eptr) const;

    /// Helper for search7pl1()
    void searchWordByLen(struct params_7plus1_t &params,
                         int i, const DicEdge *edgeptr) const;

    /// Helper for searchRegExp()
    void searchRegexpRec(const struct params_regexp_t &params,
                         int state,
                         const DicEdge *edgeptr,
                         vector<wdstring> &oWordList,
                         unsigned int iMaxResults,
                         const wdstring &iCurrWord = L"",
                         unsigned int iNbChars = 0) const;
};

#endif /* _DIC_H_ */

