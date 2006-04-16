/* Eliot                                                                     */
/* Copyright (C) 1999  Antoine Fraboulet                                     */
/*                                                                           */
/* This file is part of Eliot.                                               */
/*                                                                           */
/* Eliot is free software; you can redistribute it and/or modify             */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* Eliot is distributed in the hope that it will be useful,                  */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

/**
 *  \file   dic_internals.h
 *  \brief  Internal dictionary structures
 *  \author Antoine Fraboulet
 *  \date   2002
 */

#ifndef _DIC_INTERNALS_H_
#define _DIC_INTERNALS_H_
#if defined(__cplusplus)
extern "C"
  {
#endif

#include <stdint.h>
#include "config.h"

/**
 * bit masking for ascii characters \n
 * ('a' & CHAR) == ('A' & CHAR) == 1
 */
#define DIC_CHAR_MASK    0x1F

/**
 * keyword included in dictionary headers
 * implies little endian storage on words
 */
#define _COMPIL_KEYWORD_ "_COMPILED_DICTIONARY_"

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

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) _Dawg_edge {
  uint32_t
    chr  :  5,
    fill :  1,
    last :  1,
    term :  1,
    ptr  : 24;
};
#else
struct __attribute__ ((packed)) _Dawg_edge {
  uint32_t
    ptr  : 24,
    term :  1,
    last :  1,
    fill :  1,
    chr  :  5;
};
#endif

typedef struct _Dawg_edge Dawg_edge;


struct _Dict_header {
  char ident[sizeof(_COMPIL_KEYWORD_)];
  char unused_1;
  char unused_2;
  int root;
  int nwords;
  unsigned int edgesused;
  unsigned int nodesused;
  unsigned int nodessaved;
  unsigned int edgessaved;
};


struct _Dictionary
{
  Dawg_edge *dawg;
  unsigned int root;
  int nwords;
  int nnodes;
  int nedges;
};

#if defined(__cplusplus)
  }
#endif
#endif /* _DIC_INTERNALS_H */

