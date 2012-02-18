/*****************************************************************************
 * Eliot
 * Copyright (C) 2004-2007 Antoine Fraboulet & Olivier Teulière
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

#ifndef DEBUG_H_
#define DEBUG_H_

/**********
 * General
 **********/

#ifdef DEBUG
#   include <iostream>
#   include <cstdlib>

using std::cerr;
using std::endl;

// Assertion macro: if the condition is not verified, print a message on stderr
// and stops execution, otherwise do nothing.
#   define ASSERT(cond, msg) \
    { \
        if (!(cond)) \
        { \
            cerr << "ASSERTION FAILED: " << (msg) << " (at " \
                 << __FILE__ << "#" << __LINE__ << ")\n"; \
            abort(); \
        } \
    }
#else
#   define ASSERT(cond, msg)
#endif

#endif

