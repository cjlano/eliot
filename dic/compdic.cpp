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
#include <iostream>
#include <vector>
#include <map>
#include <boost/tokenizer.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <getopt.h>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cwctype>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstring>

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
#ifdef WIN32
#   include <windows.h>
#endif

#include "encoding.h"
#include "header.h"
#include "dic_internals.h"
#include "dic_exception.h"

using namespace std;

//#define DEBUG_OUTPUT
#define CHECK_RECURSION


const wchar_t* load_uncompressed(const string &iFileName, unsigned int &ioDicSize)
{
    ifstream file(iFileName.c_str(), ios::in | ios::binary);
    if (!file.is_open())
        throw DicException("Could not open file " + iFileName);

    // Place the buffer in a vector to avoid worrying about memory handling
    vector<char> buffer(ioDicSize);
    // Load the file data, everything in one shot
    file.read(&buffer.front(), ioDicSize);
    file.close();

    // Buffer for the wide characters (it will use at most as many characters
    // as the utf-8 version)
    wchar_t *wideBuf = new wchar_t[ioDicSize];
    unsigned int number;

    try
    {
        number = readFromUTF8(wideBuf, ioDicSize, &buffer.front(),
                              ioDicSize, "load_uncompressed");
        ioDicSize = number;
        return wideBuf;
    }
    catch (...)
    {
        // Avoid leaks, and propagate the exception
        delete[] wideBuf;
        throw;
    }
}


void readLetters(const char *iFileName, DictHeaderInfo &ioHeaderInfo)
{
    ifstream in(iFileName);
    if (!in.is_open())
        throw DicException("Could not open file " + string(iFileName));

    // Use a more friendly type name
    typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;

    int lineNb = 1;
    string line;
    while (getline(in, line))
    {
        // Ignore empty lines
        if (line == "" || line == "\r" || line == "\n")
            continue;

        // Split the lines on space characters
        vector<string> tokens;
        boost::char_separator<char> sep(" ");
        Tokenizer tok(line, sep);
        Tokenizer::iterator it;
        for (it = tok.begin(); it != tok.end(); ++it)
        {
            tokens.push_back(*it);
        }

        // We expect 5 fields on the line, and the first one is a letter, so
        // it cannot exceed 4 bytes
        if (tokens.size() != 5 || tokens[0].size() > 4)
        {
            ostringstream ss;
            ss << "readLetters: Invalid line in " << iFileName;
            ss << " (line " << lineNb << ")";
            throw DicException(ss.str());
        }

#define MAX_SIZE 4
        char buff[MAX_SIZE];
        strncpy(buff, tokens[0].c_str(), MAX_SIZE);

        wstring letter = readFromUTF8(buff, tokens[0].size(), "readLetters");

        if (letter.size() != 1)
        {
            // On the first line, there could be the BOM...
            if (lineNb == 1 && tokens[0].size() > 3 &&
                (uint8_t)tokens[0][0] == 0xEF &&
                (uint8_t)tokens[0][1] == 0xBB &&
                (uint8_t)tokens[0][2] == 0xBF)
            {
                // BOM detected, remove the first char in the wide string
                letter.erase(0, 1);
            }
            else
            {
                ostringstream ss;
                ss << "readLetters: Invalid letter at line " << lineNb;
                throw DicException(ss.str());
            }
        }
#undef MAX_SIZE

        ioHeaderInfo.letters += towupper(letter[0]);

        ioHeaderInfo.points.push_back(atoi(tokens[1].c_str()));
        ioHeaderInfo.frequency.push_back(atoi(tokens[2].c_str()));
        ioHeaderInfo.vowels.push_back(atoi(tokens[3].c_str()));
        ioHeaderInfo.consonants.push_back(atoi(tokens[4].c_str()));

        ++lineNb;
    }
}


Header skip_init_header(ostream &outfile, DictHeaderInfo &ioHeaderInfo)
{
    ioHeaderInfo.root       = 0;
    ioHeaderInfo.nwords     = 0;
    ioHeaderInfo.nodesused  = 1;
    ioHeaderInfo.edgesused  = 1;
    ioHeaderInfo.nodessaved = 0;
    ioHeaderInfo.edgessaved = 0;

    Header aHeader(ioHeaderInfo);
    aHeader.write(outfile);
    return aHeader;
}


void fix_header(ostream &outfile, DictHeaderInfo &ioHeaderInfo)
{
    ioHeaderInfo.root = ioHeaderInfo.edgesused;
    // Go back to the beginning of the stream to overwrite the header
    outfile.seekp(0, ios::beg);
#if defined(WORDS_BIGENDIAN)
#warning "**********************************************"
#warning "compdic does not run yet on bigendian machines"
#warning "**********************************************"
#else
    Header aHeader(ioHeaderInfo);
    aHeader.write(outfile);
#endif
}


// Change endianness of the pointed edges, and write them to the given ostream
void write_node(uint32_t *ioEdges, unsigned int num, ostream &outfile)
{
    // Handle endianness
    for (unsigned int i = 0; i < num; ++i)
    {
        ioEdges[i] = htonl(ioEdges[i]);
    }

#ifdef DEBUG_OUTPUT
    printf("writing %d edges\n", num);
    for (int i = 0; i < num; i++)
    {
        outfile.write((char*)(ioEdges + i), sizeof(DicEdge));
    }
#else
    outfile.write((char*)ioEdges, num * sizeof(DicEdge));
#endif
}

#define MAX_STRING_LENGTH 200


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
        IncDec(int &ioCounter)
            : m_counter(ioCounter)
        {
            m_counter++;
        }

        ~IncDec()
        {
            m_counter--;
        }
    private:
        int &m_counter;
};

int current_rec = 0;
int max_rec = 0;
#endif

typedef boost::unordered_map<vector<DicEdge>, unsigned int> HashMap;

/* global variables */
HashMap global_hashmap;

wchar_t  global_stringbuf[MAX_STRING_LENGTH]; /* Space for current string */
wchar_t* global_endstring;                    /* Marks END of current string */
const wchar_t* global_input;
const wchar_t* global_endofinput;
#ifdef CHECK_RECURSION
map<int, vector<DicEdge> > global_mapfordepth;
#endif

/**
 * Makenode takes a prefix (as position relative to stringbuf) and
 * returns an index of the start node of a dawg that recognizes all
 * words beginning with that prefix.  String is a pointer (relative
 * to stringbuf) indicating how much of iPrefix is matched in the
 * input.
 * @param iPrefix: prefix to work on
 * @param outfile: stream where to write the nodes
 * @param ioHeaderInfo: information needed to build the final header, updated
 *      during the processing
 * @param iHeader: temporary header, used only to do the conversion between
 *      the (wide) chars and their corresponding internal code
 */
unsigned int makenode(const wchar_t *iPrefix, ostream &outfile,
                      DictHeaderInfo &ioHeaderInfo, const Header &iHeader)
{
#ifdef CHECK_RECURSION
    IncDec inc(current_rec);
    if (current_rec > max_rec)
        max_rec = current_rec;
#endif

#ifdef CHECK_RECURSION
    // Instead of creating a vector, try to reuse an existing one
    vector<DicEdge> &edges = global_mapfordepth[current_rec];
    edges.reserve(MAX_EDGES);
    edges.clear();
#else
    vector<DicEdge> edges;
    // Optimize allocation
    edges.reserve(MAX_EDGES);
#endif
    DicEdge newEdge;

    while (iPrefix == global_endstring)
    {
        // More edges out of node
        newEdge.ptr  = 0;
        newEdge.term = 0;
        newEdge.last = 0;
        try
        {
            newEdge.chr = iHeader.getCodeFromChar(*global_endstring++ = *global_input++);
        }
        catch (DicException &e)
        {
            // If an invalid character is found, be specific about the problem
            ostringstream oss;
            oss << "Error on line " << 1 + ioHeaderInfo.nwords
                << ", col " << global_endstring - global_stringbuf
                << ": " << e.what() << endl;
            throw DicException(oss.str());
        }
        edges.push_back(newEdge);

        // End of a word?
        if (*global_input == L'\n' || *global_input == L'\r')
        {
            ioHeaderInfo.nwords++;
            *global_endstring = L'\0';
            // Mark edge as word
            edges.back().term = 1;

            // Skip \r and/or \n
            while (global_input != global_endofinput &&
                   (*global_input == L'\n' || *global_input == L'\r'))
            {
                ++global_input;
            }
            // At the end of input?
            if (global_input == global_endofinput)
                break;

            global_endstring = global_stringbuf;
            while (*global_endstring == *global_input)
            {
                global_endstring++;
                global_input++;
            }
        }
        // Make dawg pointed to by this edge
        edges.back().ptr =
            makenode(iPrefix + 1, outfile, ioHeaderInfo, iHeader);
    }

    int numedges = edges.size();
    if (numedges == 0)
    {
        // Special node zero - no edges
        return 0;
    }

    // Mark the last edge
    edges.back().last = 1;

    HashMap::const_iterator itMap = global_hashmap.find(edges);
    if (itMap != global_hashmap.end())
    {
        ioHeaderInfo.edgessaved += numedges;
        ioHeaderInfo.nodessaved++;

        return itMap->second;
    }
    else
    {
        unsigned int node_pos = ioHeaderInfo.edgesused;
        global_hashmap[edges] = ioHeaderInfo.edgesused;
        ioHeaderInfo.edgesused += numedges;
        ioHeaderInfo.nodesused++;
        write_node(reinterpret_cast<uint32_t*>(&edges.front()),
                   numedges, outfile);

        return node_pos;
    }
}


void printUsage(const string &iBinaryName)
{
    cout << "Usage: " << iBinaryName << " [options]" << endl
         << _("Mandatory options:") << endl
         << _("  -d, --dicname <string>  Set the dictionary name and version") << endl
         << _("  -l, --letters <string>  Path to the file containing the letters (see below)") << endl
         << _("  -i, --input <string>    Path to the uncompressed dictionary file (encoded in UTF-8)") << endl
         << _("                          The words must be in alphabetical order, without duplicates") << endl
         << _("  -o, --output <string    Path to the generated compressed dictionary file") << endl
         << _("Other options:") << endl
         << _("  -h, --help              Print this help and exit") << endl
         << _("Example:") << endl
         << "  " << iBinaryName << _(" -d 'ODS 5.0' -l letters.txt -i ods5.txt -o ods5.dawg") << endl
         << endl
         << _("The file containing the letters (--letters switch) must be UTF-8 encoded.") << endl
         << _("Each line corresponds to one letter, and must contain 5 fields separated with ") << endl
         << _("one or more space(s).") << endl
         << _(" - 1st field: the letter itself") << endl
         << _(" - 2nd field: the points of the letter") << endl
         << _(" - 3rd field: the frequency of the letter (how many letters of this kind in the game)") << endl
         << _(" - 4th field: 1 if the letter is considered as a vowel in Scrabble game, 0 otherwise") << endl
         << _(" - 5th field: 1 if the letter is considered as a consonant in Scrabble game, 0 otherwise") << endl
         << _("Example for french:") << endl
         << _("A 1 9 1 0") << endl
         << _("[...]") << endl
         << _("Z 10 1 0 1") << endl
         << _("? 0 2 1 1") << endl;
}


int main(int argc, char* argv[])
{
#if HAVE_SETLOCALE
    // Set locale via LC_ALL
    setlocale(LC_ALL, "");
#endif

#if ENABLE_NLS
    // Set the message domain
#ifdef WIN32
    // Get the absolute path, as returned by GetFullPathName()
    char baseDir[MAX_PATH];
    GetFullPathName(argv[0], MAX_PATH, baseDir, NULL);
    char *pos = strrchr(baseDir, L'\\');
    if (pos)
        *pos = '\0';
    const string localeDir = baseDir + string("\\locale");
#else
    static const string localeDir = LOCALEDIR;
#endif
    bindtextdomain(PACKAGE, localeDir.c_str());
    textdomain(PACKAGE);
#endif

    static const struct option long_options[] =
    {
        {"help", no_argument, NULL, 'h'},
        {"dicname", required_argument, NULL, 'd'},
        {"letters", required_argument, NULL, 'l'},
        {"input", required_argument, NULL, 'i'},
        {"output", required_argument, NULL, 'o'},
        {0, 0, 0, 0}
    };
    static const char short_options[] = "hd:l:i:o:";

    bool found_d = false;
    bool found_l = false;
    bool found_i = false;
    bool found_o = false;
    string inFileName;
    string outFileName;
    DictHeaderInfo headerInfo;

    int res;
    int option_index = 1;
    try
    {
        while ((res = getopt_long(argc, argv, short_options,
                                  long_options, &option_index)) != -1)
        {
            switch (res)
            {
                case 'h':
                    printUsage(argv[0]);
                    exit(0);
                case 'd':
                    found_d = true;
                    headerInfo.dicName = convertToWc(optarg);
                    break;
                case 'l':
                    found_l = true;
                    readLetters(optarg, headerInfo);
                    break;
                case 'i':
                    found_i = true;
                    inFileName = optarg;
                    break;
                case 'o':
                    found_o = true;
                    outFileName = optarg;
                    break;
            }
        }

        // Check mandatory options
        if (!found_d || !found_l || !found_i || !found_o)
        {
            cerr << _("A mandatory option is missing") << endl;
            printUsage(argv[0]);
            exit(1);
        }

        struct stat stat_buf;
        if (stat(inFileName.c_str(), &stat_buf) < 0)
        {
            cerr << _("Cannot stat uncompressed dictionary ") << inFileName << endl;
            exit(1);
        }
        unsigned int dicsize = (unsigned int)stat_buf.st_size;

        ofstream outfile(outFileName.c_str(), ios::out | ios::binary | ios::trunc);
        if (!outfile.is_open())
        {
            cerr << _("Cannot open output file ") << outFileName << endl;
            exit(1);
        }

        clock_t startLoadTime = clock();
        // FIXME: not exception safe
        const wchar_t *uncompressed = load_uncompressed(inFileName, dicsize);
        clock_t endLoadTime = clock();

        global_input = uncompressed;
        global_endofinput = global_input + dicsize;

        headerInfo.dawg = true;
        Header tempHeader = skip_init_header(outfile, headerInfo);

        DicEdge specialnode = {0, 0, 0, 0};
        specialnode.last = 1;
        // Temporary variable to avoid a warning when compiling with -O2
        // (there is no warning with -O0... g++ bug?)
        DicEdge *tmpPtr = &specialnode;
        write_node(reinterpret_cast<uint32_t*>(tmpPtr), 1, outfile);

        /*
         * Call makenode with null (relative to stringbuf) prefix;
         * Initialize string to null; Put index of start node on output
         */
        DicEdge rootnode = {0, 0, 0, 0};
        global_endstring = global_stringbuf;
        clock_t startBuildTime = clock();
        rootnode.ptr = makenode(global_endstring, outfile, headerInfo, tempHeader);
        clock_t endBuildTime = clock();
        // Reuse the temporary variable
        tmpPtr = &rootnode;
        write_node(reinterpret_cast<uint32_t*>(tmpPtr), 1, outfile);

        fix_header(outfile, headerInfo);

        Header aHeader(headerInfo);
        aHeader.print();

        delete[] uncompressed;
        outfile.close();

        printf(_(" Load time: %.3f s\n"), 1.0 * (endLoadTime - startLoadTime) / CLOCKS_PER_SEC);
        printf(_(" Compression time: %.3f s\n"), 1.0 * (endBuildTime - startBuildTime) / CLOCKS_PER_SEC);
#ifdef CHECK_RECURSION
        printf(_(" Maximum recursion level reached: %d\n"), max_rec);
#endif
        return 0;
    }
    catch (std::exception &e)
    {
        cerr << e.what() << endl;
        return 1;
    }
}

