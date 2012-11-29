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

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <boost/foreach.hpp>
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

#include "header.h"
#include "encoding.h"
#include "dic_internals.h"
#include "listdic.h"
#include "dic.h"

using namespace std;


static void printHeader(const Dictionary &iDic)
{
    iDic.getHeader().print(cout);
}


static void printLetters(const Dictionary &iDic)
{
    const Header &header = iDic.getHeader();
    const wstring &letters = header.getLetters();
    for (unsigned i = 1; i <= letters.size(); ++i)
    {
        // Main data
        wstring wlett(1, letters[i - 1]);
        cout << ufw(wlett) << " "
             << (int) header.getPoints(i) << " "
             << (int) header.getFrequency(i) << " "
             << (header.isVowel(i) ? "1" : "0") << " "
             << (header.isConsonant(i) ? "1" : "0");

        // Display and input strings
        map<wchar_t, vector<wstring> >::const_iterator it =
            header.getDisplayInputData().find(letters[i - 1]);
        if (it != header.getDisplayInputData().end())
        {
            BOOST_FOREACH(const wstring &input, it->second)
            {
                cout << " " << ufw(input);
            }
        }

        cout << endl;
    }
}


static void printWords(const Dictionary &iDic)
{
    ListDic::printWords(cout, iDic);
}


static void printHexa(const Dictionary &iDic)
{
    union edge_t
    {
        DicEdge e;
        uint32_t s;
    } ee;

    printf(_("offset binary   | structure\n"));
    printf("------ -------- | --------------------\n");
    for (unsigned int i = 0; i < (iDic.getHeader().getNbEdgesUsed() + 1); i++)
    {
        ee.e = *reinterpret_cast<const DicEdge*>(iDic.getEdgeAt(i));

        printf("0x%04lx %08x |%4d ptr=%8d t=%d l=%d chr=%2d (%lc)\n",
               (unsigned long)i*sizeof(ee), (unsigned int)(ee.s),
               i, ee.e.ptr, ee.e.term, ee.e.last, ee.e.chr,
               ee.e.chr == 0 ? L'-' : iDic.getHeader().getCharFromCode(ee.e.chr));
    }
}


static void printUsage(const string &iBinaryName)
{
    cout << "Usage: " << iBinaryName << " [-e|-l|-w|-x]] -d <dawg_file>" << endl
         << _("Mandatory options:") << endl
         << _("  -d, --dictionary <string>  Dictionary file (.dawg) to use") << endl
         << _("Output options:") << endl
         << _("  -e, --header            Print the dictionary header") << endl
         << _("  -l, --letters           Print letters information, in a format") << endl
         << _("                          suitable for the 'compdic' program") << endl
         << _("  -w, --words             Print all the words stored in the dictionary") << endl
         << _("  -x, --hexa              Print data as hexadecimal (for debugging)") << endl
         << _("Other options:") << endl
         << _("  -h, --help              Print this help and exit") << endl
         << endl
         << _("If no output option is specified, --header is used implicitly.") << endl
         << _("Example: ") << iBinaryName << " -w -d ods.dawg" << endl;
}


int main(int argc, char *argv[])
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
        {"dictionary", required_argument, NULL, 'd'},
        {"header", no_argument, NULL, 'e'},
        {"letters", no_argument, NULL, 'l'},
        {"words", no_argument, NULL, 'w'},
        {"hexa", no_argument, NULL, 'x'},
        {0, 0, 0, 0}
    };
    static const char short_options[] = "hd:elwx";

    bool dicSpecified = false;
    bool shouldPrintHeader = false;
    bool shouldPrintLetters = false;
    bool shouldPrintWords = false;
    bool shouldPrintHexa = false;
    string dicPath;

    int res;
    int option_index = 1;
    while ((res = getopt_long(argc, argv, short_options,
                              long_options, &option_index)) != -1)
    {
        switch (res)
        {
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'd':
                dicSpecified = true;
                dicPath = optarg;
                break;
            case 'e':
                shouldPrintHeader = true;
                break;
            case 'l':
                shouldPrintLetters = true;
                break;
            case 'w':
                shouldPrintWords = true;
                break;
            case 'x':
                shouldPrintHexa = true;
                break;
        }
    }

    // Check mandatory options
    if (!dicSpecified)
    {
        cerr << _("A mandatory option is missing") << endl;
        printUsage(argv[0]);
        exit(1);
    }

    // The default is to print the header
    if (!shouldPrintHeader && !shouldPrintLetters &&
        !shouldPrintWords && !shouldPrintHexa)
    {
        shouldPrintHeader = true;
    }

    try
    {
        // Load the dictionary
        Dictionary dic(dicPath);

        if (shouldPrintHeader)
            printHeader(dic);
        if (shouldPrintLetters)
            printLetters(dic);
        if (shouldPrintWords)
            printWords(dic);
        if (shouldPrintHexa)
            printHexa(dic);

        return 0;
    }
    catch (std::exception &e)
    {
        cerr << e.what() << endl;
        return 1;
    }
}

