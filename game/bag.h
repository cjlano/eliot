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

/* $Id: bag.h,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#ifndef _BAG_H_
#define _BAG_H_

#if defined(__cplusplus)
extern "C" {
#endif 

  /*************************
   * A bag stores the set of free 
   * tiles for the game
   *************************/

typedef struct tbag *Bag;

  /*************************
   * Bag general routines
   *************************/

Bag   Bag_create  (void);
void  Bag_init    (Bag b);
void  Bag_destroy (Bag b);
void  Bag_copy    (Bag dst, Bag src);

  /*************************
   * take or replace a tile in the bag
   * return value :
   * 0 : Ok
   * 1 : an error occured (not enough or too many tiles 
   *       of that type are in the bag)
   *************************/

int   Bag_taketile      (Bag b, tile_t t);
int   Bag_replacetile   (Bag b, tile_t t);

  /*************************
   * Returns how many 'tile_t' tiles are available
   *************************/

int   Bag_in            (Bag b, tile_t t);

  /*************************
   * Returns how many tiles/vowels/consonants are available
   * Warning: Bag_nvowels(b) + Bag_nconsonants(b) != Bag_ntiles(b),
   * because of the jokers and the 'Y'.
   *************************/

int   Bag_ntiles        (Bag b);
int   Bag_nvowels       (Bag b);
int   Bag_nconsonants   (Bag b);

  /*************************
   * return a random available tile 
   * the tile is not taken out of the
   * bag.
   * returns 0 on failure
   *************************/

tile_t Bag_select_random (Bag b);

#if defined(__cplusplus)
	   }
#endif 
#endif


