/*****************************************************************************
 * Eliot
 * Copyright (C) 2010 Olivier Teulière
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

#include "config.h"
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "stacktrace.h"

#ifdef HAVE_EXECINFO_H
#   include <execinfo.h>
#endif
#ifdef HAVE_GCC_ABI_DEMANGLE
#   include <cxxabi.h>
#endif

using namespace std;


INIT_LOGGER(dic, StackTrace);


string StackTrace::GetStack()
{
#if defined HAVE_EXECINFO_H && defined(DEBUG)
    static const int MAX_FRAMES = 42;
    void *frames[MAX_FRAMES];
    // Get the frames
    int nb = backtrace(frames, MAX_FRAMES);
    // Get the corresponding symbols (ignoring the first frame)
    char **symbols = backtrace_symbols(frames + 1, nb - 1);
    // Demangle the symbols and build a nice stack trace
    ostringstream oss;
    for (int i = 0; i < nb - 1; ++i)
    {
        oss << "    at " << Demangle(symbols[i]) << endl;
    }
    free (symbols);

    return oss.str();
#else
    return "";
#endif
}


// See http://tombarta.wordpress.com/2008/08/01/c-stack-traces-with-gcc/
// and http://mykospark.net/2009/09/runtime-backtrace-in-c-with-name-demangling/
// for more details
string StackTrace::Demangle(char *symbol)
{
#if defined HAVE_GCC_ABI_DEMANGLE && defined(DEBUG)
    char temp1[200];
    char temp2[200];
    if (sscanf(symbol, "%199[^(]%*[^_]%199[^)+]", temp1, temp2) == 2)
    {
        // Try to demangle a C++ name
        int status;
        char *demangled = abi::__cxa_demangle(temp2, NULL, NULL, &status);
        if (demangled != NULL)
        {
            string result = temp1 + string(": ") + demangled;
            free(demangled);
            return result;
        }
        // Try to demangle a C name
        else if (sscanf(symbol, "%199s", temp2) == 1)
        {
            return temp1 + string(": ") + temp2;
        }
    }

    // Everything else failed, return the symbol
#endif
    return symbol;
}
