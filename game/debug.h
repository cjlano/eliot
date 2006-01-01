/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
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

#ifndef _CONST_H_
#define _CONST_H_

/**********
 * General
 **********/

#ifdef DEBUG
#   include <iostream>
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

#ifdef DEBUG
#  define debug(x...) { fprintf(stderr,x); }
#else
#  define debug(x...)
#endif

#endif

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
