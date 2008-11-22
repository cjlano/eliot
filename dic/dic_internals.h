/*****************************************************************************
 * Eliot
 * Copyright (C) 2002-2007 Antoine Fraboulet
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

#ifndef _DIC_INTERNALS_H_
#define _DIC_INTERNALS_H_

#include <stdint.h>
#include "config.h"

/**
 *  structure of a compressed dictionary \n
 *  \n
 *  ----------------    \n
 *  header              \n
 *  ----------------    \n
 *  specialnode (0)     \n
 *  +                   \n
 *  + nodes             \n
 *  +                   \n
 *  firstnode (= root)  \n
 *  ----------------
 */

struct __attribute__ ((packed)) DicEdge
{
    public:
      uint32_t
        ptr : 24,
        term:  1,
        last:  1,
        chr :  6;
      bool operator==(const DicEdge &iOther) const
      {
          return memcmp(this, &iOther, sizeof(*this)) == 0;
      }
};

#endif /* _DIC_INTERNALS_H */

