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

/* $Id: tiles.h,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#ifndef _TILES_H_
#define _TILES_H_

#if defined(__cplusplus)
extern "C" {
#endif 

  /*************************
   * A ttile is the internal representation
   * used within the game library to 
   * handles letters
   *************************/

typedef unsigned char tile_t;

  /*************************
   * Zero + 26 letters + joker
   *************************/

#define TILES_NUMBER 28
#define JOKER_TILE   27

extern const int Tiles_vowels    [TILES_NUMBER];
extern const int Tiles_consonants[TILES_NUMBER];
extern const int Tiles_numbers   [TILES_NUMBER];
extern const int Tiles_points    [TILES_NUMBER];

tile_t chartocode(char  c);
char   codetochar(tile_t t);

  /*************************
   * 
   * 
   *************************/

#if defined(__cplusplus)
	   }
#endif 
#endif
