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

/**
 *  \file   regexpmain.c
 *  \brief  Program used to test regexp
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#include "config.h"

#include <exception>
#include <iostream>
#include <cstring> // For memset

#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
#else
#   define _(String) String
#endif

#include "dic.h"
#include "header.h"
#include "regexp.h"
#include "encoding.h"


void init_letter_lists(const Dictionary &iDic, struct search_RegE_list_t *iList)
{
    memset(iList, 0, sizeof(*iList));
    iList->minlength = 1;
    iList->maxlength = 15;
    iList->valid[0] = true; // all letters
    iList->symbl[0] = RE_ALL_MATCH;
    iList->valid[1] = true; // vowels
    iList->symbl[1] = RE_VOWL_MATCH;
    iList->valid[2] = true; // consonants
    iList->symbl[2] = RE_CONS_MATCH;
    iList->letters[0][0] = false;
    iList->letters[1][0] = false;
    iList->letters[2][0] = false;
    const wstring &allLetters = iDic.getHeader().getLetters();
    for (size_t i = 1; i <= allLetters.size(); ++i)
    {
        iList->letters[0][i] = true;
        iList->letters[1][i] = iDic.getHeader().isVowel(i);
        iList->letters[2][i] = iDic.getHeader().isConsonant(i);
    }

    iList->valid[3] = false; // user defined list 1
    iList->symbl[3] = RE_USR1_MATCH;
    iList->valid[4] = false; // user defined list 2
    iList->symbl[4] = RE_USR2_MATCH;
}


void usage(const char *iBinaryName)
{
    cerr << _("usage: %s dictionary") << iBinaryName << endl;
    cerr << _("   dictionary: path to eliot dawg dictionary") << endl;
}


int main(int argc, char* argv[])
{
#if HAVE_SETLOCALE
    // Set locale via LC_ALL
    setlocale(LC_ALL, "");
#endif

#if ENABLE_NLS
    // Set the message domain
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

    if (argc != 2)
    {
        usage(argv[0]);
        return 0;
    }

    try
    {
        Dictionary dic(argv[1]);

        struct search_RegE_list_t regList;
        string line;
        cout << "**************************************************************" << endl;
        cout << "**************************************************************" << endl;
        cout << _("enter a regular expression:") << endl;
        while (getline(cin, line))
        {
            if (line == "")
                break;

            /* Automaton */
            init_letter_lists(dic, &regList);
            vector<wstring> wordList;
            dic.searchRegExp(convertToWc(line), wordList, &regList);

            cout << _("result:") << endl;
            vector<wstring>::const_iterator it;
            for (it = wordList.begin(); it != wordList.end(); it++)
            {
                cerr << convertToMb(*it) << endl;
            }
            cout << "**************************************************************" << endl;
            cout << "**************************************************************" << endl;
            cout << _("enter a regular expression:") << endl;
        }

        return 0;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown exception taken" << endl;
        return 1;
    }
}
