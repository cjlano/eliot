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
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <getopt.h>

#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
#else
#   define _(String) String
#endif
#ifdef WIN32
#   include <windows.h>
#endif

#include "compdic.h"
#include "dic_exception.h"
#include "encoding.h"
#include "header.h"

using namespace std;

// Useful shortcut
#define fmt(a) boost::format(a)


void readLetters(const string &iFileName, CompDic &ioBuilder)
{
    ifstream in(iFileName.c_str());
    if (!in.is_open())
        throw DicException((fmt(_("Could not open file '%1%'")) % iFileName).str());

    // Use a more friendly type name
    typedef boost::tokenizer<boost::char_separator<wchar_t>,
            std::wstring::const_iterator,
            std::wstring> Tokenizer;

    int lineNb = 1;
    string line;
    while (getline(in, line))
    {
        // Ignore empty lines
        if (line == "" || line == "\r" || line == "\n")
            continue;

        // If there is a BOM in the file, remove it from the first line
        if (ioBuilder.getLettersCount() == 0 && line.size() >= 3 &&
            (uint8_t)line[0] == 0xEF &&
            (uint8_t)line[1] == 0xBB &&
            (uint8_t)line[2] == 0xBF)
        {
            line = line.substr(3);
        }

        // Convert the line to a wstring
        const wstring &wline = readFromUTF8(line, "readLetters (1)");
        // Split the lines on space characters
        boost::char_separator<wchar_t> sep(L" ");
        Tokenizer tok(wline, sep);
        Tokenizer::iterator it;
        vector<wstring> tokens(tok.begin(), tok.end());

        // We expect at least 5 fields on the line
        if (tokens.size() < 5)
        {
            ostringstream ss;
            ss << fmt(_("readLetters: Not enough fields "
                        "in %1% (line %2%)")) % iFileName % lineNb;
            throw DicException(ss.str());
        }

        // The first field is a single character
        wstring letter = tokens[0];
        if (letter.size() != 1)
        {
            ostringstream ss;
            ss << fmt(_("readLetters: Invalid letter at line %1% "
                        "(only one character allowed)")) % lineNb;
            throw DicException(ss.str());
        }

        vector<wstring> inputs;
        if (tokens.size() > 5)
        {
            inputs = vector<wstring>(tokens.begin() + 5, tokens.end());
        }
        ioBuilder.addLetter(letter[0], wtoi(tokens[1].c_str()),
                            wtoi(tokens[2].c_str()), wtoi(tokens[3].c_str()),
                            wtoi(tokens[4].c_str()), inputs);

        ++lineNb;
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
         << _("  -o, --output <string>   Path to the generated compressed dictionary file") << endl
         << _("Other options:") << endl
         << _("  -h, --help              Print this help and exit") << endl
         << _("Example:") << endl
         << "  " << iBinaryName << _(" -d 'ODS 5.0' -l letters.txt -i ods5.txt -o ods5.dawg") << endl
         << endl
         << _("The file containing the letters (--letters switch) must be UTF-8 encoded.") << endl
         << _("Each line corresponds to one letter, and must contain at least 5 fields separated with "
              "one or more space(s).") << endl
         << _(" - 1st field: the letter itself, as stored in the input file (single character)") << endl
         << _(" - 2nd field: the points of the letter") << endl
         << _(" - 3rd field: the frequency of the letter (how many letters of this kind in the game)") << endl
         << _(" - 4th field: 1 if the letter is considered as a vowel in Scrabble game, 0 otherwise") << endl
         << _(" - 5th field: 1 if the letter is considered as a consonant in Scrabble game, 0 otherwise") << endl
         << _(" - 6th field (optional): display string for the letter (default: the letter itself)") << endl
         << _(" - other fields (optional): input strings for the letter, in addition to the display string") << endl
         << endl
         << _("Example for french:") << endl
         << "A 1 9 1 0" << endl
         << "[...]" << endl
         << "Z 10 1 0 1" << endl
         << "? 0 2 1 1" << endl
         << endl
         << _("Example for catalan:") << endl
         << "A 1 12 1 0" << endl
         << "[...]" << endl
         // TRANSLATORS: the first "L.L" must be translated "L·L",
         // and the last one translated "ĿL"
         << _("W 10 1 0 1 L.L L.L L-L L.L") << endl
         << "X 10 1 0 1" << endl
         << "Y 10 1 0 1 NY" << endl
         << "[...]" << endl;
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
    string dicName;
    string inFileName;
    string outFileName;
    CompDic builder;

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
                    dicName = optarg;
                    break;
                case 'l':
                    found_l = true;
                    readLetters(optarg, builder);
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

        // Generate the dictionary
        const Header &header =
            builder.generateDawg(inFileName, outFileName, dicName);

        // Print the header
        header.print();

        cout << fmt(_(" Load time: %1% s")) % builder.getLoadTime() << endl;
        cout << fmt(_(" Compression time: %1% s")) % builder.getBuildTime() << endl;
#ifdef CHECK_RECURSION
        cout << fmt(_(" Maximum recursion level reached: %1%")) % builder.getMaxRecursion() << endl;
#endif
        return 0;
    }
    catch (std::exception &e)
    {
        cerr << fmt(_("Exception caught: %1%")) % e.what() << endl;
        return 1;
    }
}

