/* Eliot                                                                     */
/* Copyright (C) 1999  antoine.fraboulet                                     */
/* antoine.fraboulet@free.fr                                                 */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*
 * $Id: dic_internals.h,v 1.1 2004/04/08 09:43:06 afrab Exp $
 */
#ifndef _DIC_INTERNALS_H
#define _DIC_INTERNALS_H


/* bit masking for ascii characters */
/* 'a' & CHAR == 'A' & CHAR == 1    */
#define CHAR    0x1F


#define _COMPIL_KEYWORD_ "_COMPILED_DICTIONARY_"

/*

   structure of a compressed dictionary

   ----------------
   header
   ----------------
   specialnode (0)
   +
   + nodes
   +
   firstnode (= root)
   ----------------

*/

typedef struct _Dawg_edge { 
   unsigned int ptr  : 24; 
   unsigned int term : 1;  
   unsigned int last : 1;  
   unsigned int fill : 1;  
   unsigned int chr  : 5;  
} Dawg_edge;    

typedef struct _Dict_header {
  char ident[sizeof(_COMPIL_KEYWORD_)];
  char unused_1;
  char unused_2;
  int root;
  int nwords;
  unsigned int edgesused;
  unsigned int nodesused;
  unsigned int nodessaved;
  unsigned int edgessaved;
} Dict_header;

struct _Dictionary
{
  Dawg_edge *dawg; 
  unsigned int root;
  int nwords;
  int nnodes;
  int nedges;
};

#endif
