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

/* $Id: rack.h,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#ifndef _RACK_H_
#define _RACK_H_

#if defined(__cplusplus)
extern "C" {
#endif 

  /*************************
   * A rack is a set of tiles, no more.
   * tiles have to be in the bag for the
   * rack to be valid.
   *************************/

typedef struct track* Rack;
  
  /*************************
   * general routines
   *************************/

Rack Rack_create  (void);
void Rack_init    (Rack r);
void Rack_destroy (Rack r);
void Rack_copy    (Rack dst, Rack src);

  /*************************
   * Rack_empty return 1 if the rack is
   * empty, 0 otherwise (ntiles == 0)
   *************************/

int  Rack_ntiles    (Rack r);
int  Rack_empty     (Rack r);

  /*************************
   * 
   * 
   *************************/

int  Rack_in        (Rack r,tile_t t);
void Rack_add       (Rack r,tile_t t);
void Rack_remove    (Rack r,tile_t t);

  /*************************
   * 
   * 
   *************************/

#if defined(__cplusplus)
	   }
#endif 
#endif
