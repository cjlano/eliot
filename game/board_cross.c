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

/* $Id: board_cross.c,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#include <dic.h>
#include "tiles.h"
#include "bag.h"
#include "rack.h"
#include "round.h"
#include "results.h"
#include "board.h"
#include "board_internals.h" 

#include "debug.h"

/*
 * crosschecks
 * bitmap over an int, tiles are in reversed order
 * { 0-0 z y x w v u t s r q p o n m l k j i h g f e d c b a 0 }
 * CROSS_MASK is thus set to 0x7FFFFFE when we want to allow
 * every possible letter
 */


static unsigned int
Board_lookup(Dictionary d, unsigned int t, tile_t* s)
{
     unsigned int p;
begin:
     if (! *s)
          return t;
     if (! Dic_succ(d,t))
          return 0;
     p = Dic_succ(d,t);
     do {
          if (Dic_chr(d,p) == *s) {
               t = p;
               s++;
               goto begin;
          } else if (Dic_last(d,p)) {
               return 0;
          }
          p = Dic_next(d,p);
     } while (1);
     return 0;
}

static unsigned int
Board_checkout_tile(Dictionary d, tile_t* tiles, char* joker, int *points)
{
     unsigned int node,succ;
     unsigned int mask;
     tile_t* t = tiles;
     char* j = joker;

     mask = 0;
     *points = 0;

     /* points on the left part */
     while (t[-1]) {
       j--;
       t--;
       if (!(*j))
          (*points) += Tiles_points[*t];
     }
     
     /* tiles that can be played */
     node = Board_lookup(d,Dic_root(d),t);
     if (node == 0)
          return 0;
     
     for( succ = Dic_succ(d,node) ; succ ; succ = Dic_next(d,succ)) {
          if (Dic_word(d,Board_lookup(d,succ,tiles + 1)))
               mask |= (1 << Dic_chr(d,succ));
          if (Dic_last(d,succ))
               break;
     } 
     
     /* points on the right part */
     while (tiles[1]) {
       joker++;
       tiles++;
       if (!(*joker))
	 (*points) += Tiles_points[*tiles];
     }

     return mask;
}

static void
Board_check(Dictionary d, 
            tile_t tiles[BOARD_REALDIM][BOARD_REALDIM],
	    char joker[BOARD_REALDIM][BOARD_REALDIM],
            unsigned int  cross[BOARD_REALDIM][BOARD_REALDIM],
            int point[BOARD_REALDIM][BOARD_REALDIM])
{
     int i,j;
     
     for(i = 1; i <= BOARD_DIM; i++) {
          for(j = 1; j <= BOARD_DIM; j++) {
               point[j][i] = -1;
               if (tiles[i][j]) 
                    cross[j][i] = 0;
               else if (tiles[i][j - 1] || 
                        tiles[i][j + 1])
                    cross[j][i] =
                         Board_checkout_tile(d,
					     tiles[i] + j,
					     joker[i] + j,
                                             point[j] + i);
               else
                    cross[j][i] = CROSS_MASK;
          }
     }
}

void  
Board_buildcross(Dictionary d, Board b)
{
  Board_check(d,b->tiles_r,b->joker_r,b->cross_c,b->point_c);
  Board_check(d,b->tiles_c,b->joker_c,b->cross_r,b->point_r);
}
