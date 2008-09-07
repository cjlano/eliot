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

/**
 *  \file   listdic.c
 *  \brief  Program used to list a dictionary
 *  \author Antoine Fraboulet
 *  \date   1999
 */

#include "config.h"

#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstddef>

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
#include "dic.h"

using namespace std;


template <typename DAWG_EDGE>
static void print_dic_rec(ostream &out, const Dictionary &iDic, wchar_t *buf, wchar_t *s, DAWG_EDGE i)
{
    if (i.term)  /* edge points at a complete word */
    {
        *s = '\0';
        out << convertToMb(buf) << endl;
    }
    if (i.ptr)
    {           /* Compute index: is it non-zero ? */
        const DAWG_EDGE *p = reinterpret_cast<const DAWG_EDGE*>(iDic.getEdgeAt(i.ptr));
        do
        {                         /* for each edge out of this node */
            *s = iDic.getHeader().getCharFromCode(p->chr);
            print_dic_rec(out, iDic, buf, s + 1, *p);
        }
        while (!(*p++).last);
    }
}


template <typename DAWG_EDGE>
void print_dic_list(const Dictionary &iDic)
{
    static wchar_t buf[80];
    print_dic_rec(cout, iDic, buf, buf, *reinterpret_cast<const DAWG_EDGE*>(iDic.getEdgeAt(iDic.getRoot())));
}


template <typename DAWG_EDGE>
static void print_node_hex(const Dictionary &dic, int i)
{
    union edge_t
    {
        DAWG_EDGE e;
        uint32_t  s;
    } ee;

    ee.e = *reinterpret_cast<const DAWG_EDGE*>(dic.getEdgeAt(i));

    printf("0x%04lx %08x |%4d ptr=%8d t=%d l=%d chr=%2d (%c)\n",
           (unsigned long)i*sizeof(ee), (unsigned int)(ee.s),
           i, ee.e.ptr, ee.e.term, ee.e.last, ee.e.chr, ee.e.chr +'a' -1);
}


template <typename DAWG_EDGE>
void print_dic_hex(const Dictionary &iDic)
{
    printf(_("offset binary   | structure\n"));
    printf("------ -------- | --------------------\n");
    for (unsigned int i = 0; i < (iDic.getHeader().getNbEdgesUsed() + 1); i++)
        print_node_hex<DAWG_EDGE>(iDic, i);
}


void usage(const string &iName)
{
    printf(_("usage: %s [-a|-h|-l|-x] dictionary\n"), iName.c_str());
    printf(_("  -a: print all\n"));
    printf(_("  -h: print header\n"));
    printf(_("  -l: print dictionary word list\n"));
    printf(_("  -x: print dictionary in hex\n"));
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

    int arg_count;
    int option_print_all      = 0;
    int option_print_header   = 0;
    int option_print_dic_hex  = 0;
    int option_print_dic_list = 0;

    if (argc < 3)
    {
        usage(argv[0]);
        exit(1);
    }

    arg_count = 1;
    while (argv[arg_count][0] == '-')
    {
        switch (argv[arg_count][1])
        {
            case 'a': option_print_all = 1; break;
            case 'h': option_print_header = 1; break;
            case 'l': option_print_dic_list = 1; break;
            case 'x': option_print_dic_hex = 1; break;
            default: usage(argv[0]); exit(2); break;
        }
        arg_count++;
    }

    try
    {
        Dictionary dic(argv[arg_count]);

        if (option_print_header || option_print_all)
        {
            dic.getHeader().print();
        }
        if (option_print_dic_hex || option_print_all)
        {
            if (dic.getHeader().getVersion() == 0)
                print_dic_hex<DicEdgeOld>(dic);
            else
                print_dic_hex<DicEdge>(dic);
        }
        if (option_print_dic_list || option_print_all)
        {
            if (dic.getHeader().getVersion() == 0)
                print_dic_list<DicEdgeOld>(dic);
            else
                print_dic_list<DicEdge>(dic);
        }
        return 0;
    }
    catch (std::exception &e)
    {
        cerr << e.what() << endl;
        return 1;
    }
}
