/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *
 * $Id: debug.h,v 1.5 2005/03/27 21:45:04 ipkiss Exp $
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/

#ifndef _CONST_H_
#define _CONST_H_

/**********
 * General
 **********/

// XXX: Temporary
#define _DEBUG_

#ifdef _DEBUG_
#   include <iostream>
// Assertion macro: if the condition is not verified, print a message on stderr
// and stops execution, otherwise do nothing.
#   define ASSERT(cond, msg) \
    { \
        if (!cond) \
        { \
            cerr << "ASSERTION FAILED: " << msg << " (at " \
                 << __FILE__ << "#" << __LINE__ << ")\n"; \
            abort(); \
        } \
    }
#else
#   define ASSERT(cond, msg)
#endif

#endif
