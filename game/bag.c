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

/* $Id: bag.c,v 1.2 2004/08/07 18:10:42 ipkiss Exp $ */

#include <stdlib.h>
#include <string.h>
#include "tiles.h"
#include "bag.h"

#include "debug.h"

struct tbag {
     int tiles[TILES_NUMBER];
     int ntiles;
};


Bag
Bag_create(void)
{
  Bag b;
  b = (Bag) malloc(sizeof(struct tbag));
  Bag_init(b);
  return b;
}


void
Bag_init(Bag b)
{
  int i;
  b->ntiles = 0;
  for (i = 0; i < TILES_NUMBER; i++)
    {
      b->tiles[i] = Tiles_numbers[i];
      b->ntiles += Tiles_numbers[i];
    }
}


void
Bag_destroy(Bag b)
{
  if (b)
    free(b);
}


void
Bag_copy(Bag dest, Bag src)
{
  if (dest && src)
    memcpy(dest,src,sizeof(struct tbag));
}


int
Bag_in(Bag b, tile_t c)
{
  return b->tiles[c];
}


int
Bag_ntiles(Bag b)
{
  return b->ntiles;
}


int
Bag_nvowels(Bag b)
{
    int i, v;

    v = 0;
    for(i=1; i < TILES_NUMBER; i++)
    {
        if (Tiles_vowels[i])
            v += b->tiles[i];
    }
    return v;
}


int
Bag_nconsonants(Bag b)
{
    int i, c;

    c = 0;
    for(i=1; i < TILES_NUMBER; i++)
    {
        if (Tiles_consonants[i])
            c += b->tiles[i];
    }
    return c;
}


int
Bag_taketile(Bag b, tile_t t)
{
  if (Bag_in(b,(tile_t)t))
    {
      b->tiles[t] --;
      b->ntiles --;
    }
  else
    {
      return 1;
    }
  return 0;
}


int
Bag_replacetile(Bag b, tile_t t)
{
  if (b->tiles[t] < Tiles_numbers[t])
    {
      b->tiles[t]++;
      b->ntiles++;
    }
  else
    {
      return 1;
    }
  return 0;
}


tile_t
Bag_select_random(Bag b)
{
  int i,n;
  double max = (double)b->ntiles;

  n = (int)(max * rand() / (RAND_MAX + 1.0));
  for(i=1; i < TILES_NUMBER; i++)
    {
      if (n < b->tiles[i])
        return i;
      n -= b->tiles[i];
    }
  PDEBUG(1,"BAG:we should not come here\n")
  return 0;
}

