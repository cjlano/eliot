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

/* $Id: rack.c,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tiles.h"
#include "bag.h"
#include "rack.h"

struct track {
     int tiles[TILES_NUMBER];
     int ntiles;
};


Rack 
Rack_create(void)
{
     Rack r;
     r = (Rack)malloc(sizeof(struct track));
     if (r)
       Rack_init(r);
     return r;
}


void 
Rack_init(Rack r)
{
     memset(r,0,sizeof(struct track));
}


void 
Rack_destroy(Rack r)
{
     if (r)
	  free(r);
}


void
Rack_copy(Rack dest, Rack source)
{
     memcpy(dest,source,sizeof(struct track));
}


int 
Rack_empty(Rack r)
{
     return r->ntiles == 0;
}


int
Rack_ntiles(Rack r)
{
     return r->ntiles;
}


int
Rack_in(Rack rack, tile_t tile)
{
     return rack->tiles[tile];
}


void
Rack_remove(Rack rack, tile_t tile)
{
     rack->tiles[tile]--;
     rack->ntiles --;
}


void
Rack_add(Rack rack, tile_t tile)
{
     rack->tiles[tile]++;
     rack->ntiles ++;
}


