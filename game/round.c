/* Eliot                                                                     */
/* Copyright (C) 1999  Antoine Fraboulet                                     */
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

/* $Id: round.c,v 1.2 2004/08/07 18:10:42 ipkiss Exp $ */

#include <stdlib.h>
#include <string.h>
#include "tiles.h"
#include "round.h"

#define ROUND_INTERNAL_MAX 20

#define FROMBOARD 0x1
#define FROMRACK  0x2
#define JOKER     0x4


struct tround {
  tile_t word       [ROUND_INTERNAL_MAX];
  char   tileorigin [ROUND_INTERNAL_MAX];
  
  int wordlen;
  int row,column;
  Direction dir;
  int points;
  int bonus;
};


Round
Round_create(void)
{
  Round r;
  r = (Round)malloc(sizeof(struct tround));
  if (r)
    Round_init(r);
  return r;
}


void
Round_init(Round r)
{
  memset(r->word,0,sizeof(tile_t)*ROUND_INTERNAL_MAX);
  memset(r->tileorigin,0,sizeof(char)*ROUND_INTERNAL_MAX);
  r->wordlen   = 0;
  r->row       = 1;
  r->column    = 1;
  r->dir       = HORIZONTAL;
  r->points    = 0;
  r->bonus     = 0;
}


void
Round_copy(Round dest, Round source)
{
     memcpy(dest,source,sizeof(struct tround));
}


void
Round_destroy(Round r)
{
     if (r) {
	  free(r);
     }
}


void
Round_setword(Round r, tile_t* c)
{
     strncpy((char*)r->word,(char*)c,ROUND_INTERNAL_MAX);
     r->wordlen = strlen(r->word);
}


void 
Round_setrow(Round r, int row)
{
     r->row = row;
}


void 
Round_setcolumn(Round r, int c)
{
     r->column = c;
}


void
Round_setpoints(Round r, int p)
{
     r->points = p;
}


void Round_setdir(Round r, Direction d)
{
     r->dir = d;
}


void Round_setbonus(Round r, int b)
{
     r->bonus = b;
}


void Round_setfromrack(Round r, int c)
{
    r->tileorigin[c] &= ~FROMBOARD;
    r->tileorigin[c] |= FROMRACK;
}


void Round_setfromboard(Round r, int c)
{
    r->tileorigin[c] &= ~FROMRACK;
    r->tileorigin[c] |= FROMBOARD;
}


void Round_setjoker(Round r, int c)
{
    r->tileorigin[c] |= JOKER;
}


tile_t
Round_gettile(Round r, int n)
{
     return r->word[n];
}


int 
Round_joker(Round r, int c)
{
     return r->tileorigin[c] & JOKER;
}


int 
Round_playedfromrack(Round r, int c)
{
     return r->tileorigin[c] & FROMRACK;
}


int 
Round_wordlen(Round r)
{
     return r->wordlen;
}


int 
Round_row(Round r)
{
     return r->row;
}


int 
Round_column(Round r)
{
     return r->column;
}


int
Round_points(Round r)
{
     return r->points;
}


Direction 
Round_dir(Round r)
{
     return r->dir;
}


int
Round_bonus(Round r)
{
     return r->bonus;
}


void 
Round_addrightfromboard(Round r, tile_t c)
{
  r->word[r->wordlen] = c;
  r->tileorigin[r->wordlen++] = FROMBOARD;
}


void 
Round_removerighttoboard(Round r, tile_t c)
{
  c ++; /* unused */
  r->wordlen--;
  r->word[r->wordlen] = '\0';
  r->tileorigin[r->wordlen] = 0;
}


void 
Round_addrightfromrack(Round r, tile_t c, int j)
{
     r->word[r->wordlen] = c;
     r->tileorigin[r->wordlen] = FROMRACK;
     if (j) {
       /* c = JOKER_TILE; */
	 r->tileorigin[r->wordlen] |= JOKER;
     }
     r->wordlen++;
}


void 
Round_removerighttorack(Round r, tile_t c, int j)
{
  r->wordlen--;
  r->word[r->wordlen] = '\0';
  r->tileorigin[r->wordlen] = 0;
  if (j) 
    {
      c = JOKER_TILE;
    }
}

