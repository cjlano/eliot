/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2007 Antoine Fraboulet & Olivier Teulière
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
#include <sstream>
#include <map>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cwctype>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstring>

#include "compdic.h"
#include "encoding.h"
#include "dic_exception.h"

// For htonl & Co.
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

// Useful shortcut
#define fmt(a) boost::format(a)


INIT_LOGGER(dic, CompDic);


CompDic::CompDic()
    : m_currentRec(0), m_maxRec(0), m_loadTime(0), m_buildTime(0)
{
    m_headerInfo.root       = 0;
    m_headerInfo.nwords     = 0;
    m_headerInfo.nodesused  = 1;
    m_headerInfo.edgesused  = 1;
    m_headerInfo.nodessaved = 0;
    m_headerInfo.edgessaved = 0;
}


void CompDic::addLetter(wchar_t chr, int points, int frequency,
                        bool isVowel, bool isConsonant,
                        const vector<wstring> &iInputs)
{
    // We don't support non-alphabetical characters in the dictionary
    // apart from the joker '?'. For more explanations on the issue, see
    // on the eliot-dev mailing-list the thread with the following title:
    //   re: Unable to show menus in Catalan, and some weird char "problem"
    // (started on 2009/12/31)
    if (!iswalpha(chr) && chr != L'?')
    {
        ostringstream ss;
        ss << fmt(_("'%1%' is not a valid letter.")) % lfw(chr) << endl;
        ss << fmt(_("For technical reasons, Eliot currently only supports "
                    "alphabetical characters as internal character "
                    "representation, even if the tile has a display string "
                    "defined. Please use another character and change your "
                    "word list accordingly."));
        throw DicException(ss.str());
    }

    const wchar_t upChar = towupper(chr);
    m_headerInfo.letters += upChar;
    m_headerInfo.points.push_back(points);
    m_headerInfo.frequency.push_back(frequency);
    m_headerInfo.vowels.push_back(isVowel);
    m_headerInfo.consonants.push_back(isConsonant);

    // Ensure the input strings are in upper case
    if (!iInputs.empty())
    {
        vector<wstring> upperInputs = iInputs;
        BOOST_FOREACH(wstring &str, upperInputs)
        {
            std::transform(str.begin(), str.end(), str.begin(), towupper);
        }

        // If the display string is identical to the internal char and if
        // there is no other input, no need to save this information, as
        // it is already the default.
        if (upperInputs.size() != 1 || upperInputs[0] != wstring(1, upChar))
        {
            m_headerInfo.displayInputData[upChar] = upperInputs;
        }
    }
}


void CompDic::loadWordList(const string &iFileName, vector<wstring> &oWordList)
{
    ifstream file(iFileName.c_str(), ios::in | ios::binary);
    if (!file.is_open())
        throw DicException((fmt(_("Could not open file '%1%'")) % iFileName).str());

    // Get the file size
    struct stat stat_buf;
    if (stat(iFileName.c_str(), &stat_buf) < 0)
        throw DicException((fmt(_("Could not open file '%1%'")) % iFileName).str());
    int dicSize = (unsigned int)stat_buf.st_size;

    // Reserve some space (heuristic: the average length of words is 11)
    oWordList.reserve(dicSize / 11);

    string line;
    while (getline(file, line))
    {
        // If there is a BOM in the file, remove it from the first word
        if (oWordList.empty() && line.size() >= 3 &&
            (uint8_t)line[0] == 0xEF &&
            (uint8_t)line[1] == 0xBB &&
            (uint8_t)line[2] == 0xBF)
        {
            line = line.substr(3);
        }
        // Remove potential \r
        if (line[line.size() - 1] == '\r')
            line = line.substr(0, line.size() - 1);
        // Ignore empty lines
        if (line == "")
            continue;
        oWordList.push_back(readFromUTF8(line, "loadWordList"));
    }

    // Sort the word list, to perform a better compression
    sort(oWordList.begin(), oWordList.end());
}


Header CompDic::writeHeader(ostream &outFile) const
{
    // Go back to the beginning of the stream before writing the header
    outFile.seekp(0, ios::beg);
    Header aHeader(m_headerInfo);
    aHeader.write(outFile);
    return aHeader;
}


void CompDic::writeNode(uint32_t *ioEdges, unsigned int num, ostream &outFile)
{
    // Handle endianness
    for (unsigned int i = 0; i < num; ++i)
    {
        ioEdges[i] = htonl(ioEdges[i]);
    }

    LOG_TRACE(fmt("writing %1% edges") % num);
    outFile.write((char*)ioEdges, num * sizeof(DicEdge));
}

#define MAX_EDGES 2000
/* ods3: ??   */
/* ods4: 1746 */

// Hashing function for a vector of DicEdge, based on the hashing function
// of the HashTable
size_t hash_value(const DicEdge &iEdge)
{
    const uint32_t *num = reinterpret_cast<const uint32_t*>(&iEdge);
    size_t seed = 0;
    boost::hash_combine(seed, *num);
    return seed;
}

#ifdef CHECK_RECURSION
class IncDec
{
    public:
        IncDec(int &ioCounter) : m_counter(ioCounter) { ++m_counter; }
        ~IncDec() { --m_counter; }
    private:
        int &m_counter;
};
#endif


unsigned int CompDic::makeNode(ostream &outFile, const Header &iHeader,
                               vector<wstring>::const_iterator &itCurrWord,
                               const vector<wstring>::const_iterator &itLastWord,
                               wstring::const_iterator &itPosInWord,
                               const wchar_t *iPrefix)
{
#ifdef CHECK_RECURSION
    IncDec inc(m_currentRec);
    if (m_currentRec > m_maxRec)
        m_maxRec = m_currentRec;

    // Instead of creating a vector, try to reuse an existing one
    vector<DicEdge> &edges = m_mapForDepth[m_currentRec];
    edges.reserve(MAX_EDGES);
    edges.clear();
#else
    vector<DicEdge> edges;
    // Optimize allocation
    edges.reserve(MAX_EDGES);
#endif
    DicEdge newEdge;

    while (iPrefix == m_endString)
    {
        // More edges out of node
        newEdge.ptr  = 0;
        newEdge.term = 0;
        newEdge.last = 0;
        try
        {
            newEdge.chr = iHeader.getCodeFromChar(*m_endString = *itPosInWord);
            ++m_endString;
            ++itPosInWord;
        }
        catch (DicException &e)
        {
            // If an invalid character is found, be specific about the problem
            ostringstream oss;
            oss << fmt(_("Error on line %1%, col %2%: %3%"))
                % (1 + m_headerInfo.nwords)
                % (m_endString - m_stringBuf)
                % e.what() << endl;
            throw DicException(oss.str());
        }
        edges.push_back(newEdge);

        // End of a word?
        if (itPosInWord == itCurrWord->end())
        {
            m_headerInfo.nwords++;
            *m_endString = L'\0';
            // Mark edge as word
            edges.back().term = 1;

            // Next word
            ++itCurrWord;
            // At the end of input?
            if (itCurrWord == itLastWord)
                break;
            itPosInWord = itCurrWord->begin();

            m_endString = m_stringBuf;
            // This assumes that a word cannot be a prefix of the previous one
            while (*m_endString == *itPosInWord)
            {
                ++m_endString;
                ++itPosInWord;
            }
        }
        // Make dawg pointed to by this edge
        edges.back().ptr = makeNode(outFile, iHeader, itCurrWord, itLastWord,
                                    itPosInWord, iPrefix + 1);
    }

    int numedges = edges.size();
    if (numedges == 0)
    {
        // Special node zero - no edges
        return 0;
    }

    // Mark the last edge
    edges.back().last = 1;

    HashMap::const_iterator itMap = m_hashMap.find(edges);
    if (itMap != m_hashMap.end())
    {
        m_headerInfo.edgessaved += numedges;
        m_headerInfo.nodessaved++;

        return itMap->second;
    }
    else
    {
        unsigned int node_pos = m_headerInfo.edgesused;
        m_hashMap[edges] = m_headerInfo.edgesused;
        m_headerInfo.edgesused += numedges;
        m_headerInfo.nodesused++;
        writeNode(reinterpret_cast<uint32_t*>(&edges.front()),
                   numedges, outFile);

        return node_pos;
    }
}


Header CompDic::generateDawg(const string &iWordListFile,
                             const string &iDawgFile,
                             const string &iDicName)
{
    m_headerInfo.dicName = wfl(iDicName);
    // We are not (yet) able to build the GADDAG format
    m_headerInfo.dawg = true;

    // Open the output file
    ofstream outFile(iDawgFile.c_str(), ios::out | ios::binary | ios::trunc);
    if (!outFile.is_open())
    {
        ostringstream oss;
        oss << fmt(_("Cannot open output file '%1%'")) % iDawgFile;
        throw DicException(oss.str());
    }

    const clock_t startLoadTime = clock();
    vector<wstring> wordList;
    loadWordList(iWordListFile, wordList);
    const clock_t endLoadTime = clock();
    m_loadTime = 1.0 * (endLoadTime - startLoadTime) / CLOCKS_PER_SEC;

    if (wordList.empty())
    {
        throw DicException(_("The word list is empty!"));
    }

    // Write the header a first time, to reserve the space in the file
    Header tempHeader = writeHeader(outFile);

    DicEdge specialNode = {0, 0, 0, 0};
    specialNode.last = 1;
    // Temporary variable to avoid a warning when compiling with -O2
    // (there is no warning with -O0... g++ bug?)
    DicEdge *tmpPtr = &specialNode;
    writeNode(reinterpret_cast<uint32_t*>(tmpPtr), 1, outFile);

    vector<wstring>::const_iterator firstWord = wordList.begin();
    wstring::const_iterator initialPos = firstWord->begin();

    // Call makeNode with null (relative to stringbuf) prefix;
    // Initialize string to null; Put index of start node on output
    DicEdge rootNode = {0, 0, 0, 0};
    m_endString = m_stringBuf;
    const clock_t startBuildTime = clock();
    rootNode.ptr = makeNode(outFile, tempHeader,
                            firstWord, wordList.end(),
                            initialPos, m_endString);
    // Reuse the temporary variable
    tmpPtr = &rootNode;
    writeNode(reinterpret_cast<uint32_t*>(tmpPtr), 1, outFile);
    const clock_t endBuildTime = clock();
    m_buildTime = 1.0 * (endBuildTime - startBuildTime) / CLOCKS_PER_SEC;

    // Write the header again, now that it is complete
    m_headerInfo.root = m_headerInfo.edgesused;
    const Header finalHeader = writeHeader(outFile);

    // Clean up
    outFile.close();

    return finalHeader;
}

