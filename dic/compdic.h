/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2007 Antoine Fraboulet
 * Authors: Antoine Fraboulet <antoine.fraboulet @@ free.fr>
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

#ifndef DIC_COMPDIC_H_
#define DIC_COMPDIC_H_

#include <vector>
#include <string>
#include <iosfwd>
#include <boost/unordered_map.hpp>

#include "header.h"
#include "dic_internals.h"
#include "logging.h"

class DicEdge;
class DictHeaderInfo;
class Header;

using namespace std;

//#define DEBUG_OUTPUT
#define CHECK_RECURSION


class CompDic
{
    DEFINE_LOGGER();
    typedef boost::unordered_map<vector<DicEdge>, unsigned int> HashMap;

public:
    CompDic();

    /**
     * Define a new letter. The letter must be alphabetic (i.e. iswalpha()
     * returns true for it).
     * @param letter: Letter to addLetter
     * @param points: Points of the letter
     * @param frequency: Number of occurrences of the letter in the game
     * @param isVowel: True if the letter can be considered as a vowel,
     *      false otherwise
     * @param isConsonant: True if the letter can be considered as a consonant,
     *      false otherwise
     * @param iInputs: Vector containing the various ways to input the letter.
     *      If not empty, the first value corresponds to the display string.
     */
    void addLetter(wchar_t letter, int points, int frequency,
                   bool isVowel, bool isConsonant,
                   const vector<wstring> &iInputs);

    unsigned getLettersCount() const { return m_headerInfo.letters.size(); }

    /**
     * Generate the dictionary. You must have called addLetter() before
     * (once for each letter of the word list, and possible once for the
     * joker).
     * @param iWordListFile: Name (and path) of the word list file
     * @param iDawgFile: Name (and path) of the generated dawg file
     * @param iDicName: Internal name of the dictionary
     * @return The header of the generated dawg
     */
    Header generateDawg(const string &iWordListFile,
                        const string &iDawgFile,
                        const string &iDicName);

    // Statistics
    double getLoadTime() const { return m_loadTime; }
    double getBuildTime() const { return m_buildTime; }
#ifdef CHECK_RECURSION
    double getMaxRecursion() const { return m_maxRec; }
#endif

private:
    DictHeaderInfo m_headerInfo;

    HashMap m_hashMap;

#define MAX_STRING_LENGTH 200

    /// Space for the current string
    wchar_t m_stringBuf[MAX_STRING_LENGTH];
    /// Point to the end of the string
    wchar_t* m_endString;
#ifdef CHECK_RECURSION
    map<int, vector<DicEdge> > m_mapForDepth;
    int m_currentRec;
    int m_maxRec;
#endif

    double m_loadTime;
    double m_buildTime;


    /**
     * Read the word list stored in iFileName, convert it to wide chars,
     * and return it (in the oWordList argument).
     * In case of problem, an exception is thrown.
     * @param iFileName: Name (and path) of the file containing the word list.
     * @param oWordList: Word list
     */
    void loadWordList(const string &iFileName, vector<wstring> &oWordList);

    Header writeHeader(ostream &outFile) const;

    /**
     * Change the endianness of the pointed edges (if needed),
     * and write them to the given ostream.
     * @param ioEdges: array of edges
     * @param num: number of edges in the array
     * @param outFile: stream where to write the edges
     */
    void writeNode(uint32_t *ioEdges, unsigned int num, ostream &outFile);

    /**
     * MakeNode takes a prefix (as position relative to m_stringBuf) and
     * returns the index of the start node of a dawg that recognizes all
     * the words beginning with that prefix.  String is a pointer (relative
     * to m_stringBuf) indicating how much of iPrefix is matched in the
     * input.
     * @param outfile: stream where to write the nodes
     * @param iHeader: temporary header, used only to do the conversion between
     *      the (wide) chars and their corresponding internal code
     * @param itCurrWord: iterator on the word list
     * @param itLastWord: end of the word list
     * @param itPosInWord: iterator on the letters of the current word
     * @param iPrefix: prefix to work on
     * @return the index of a DAWG matching all the words with prefix iPrefix
     */
    unsigned int makeNode(ostream &outFile, const Header &iHeader,
                          vector<wstring>::const_iterator &itCurrWord,
                          const vector<wstring>::const_iterator &itLastWord,
                          wstring::const_iterator &itPosInWord,
                          const wchar_t *iPrefix);

};

#endif /* DIC_COMPDIC_H_ */
