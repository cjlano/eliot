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

/* $Id: board.h,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#ifndef _BOARD_H_
#define _BOARD_H_

#if defined(__cplusplus)
extern "C" {
#endif 

  /*************************
   * 
   * 
   * 
   *************************/

typedef struct tboard *Board;

  /*************************
   * general routines
   *************************/

Board  Board_create          (void);
void   Board_init            (Board);
void   Board_destroy         (Board);

  /*************************
   * 
   * 
   *************************/

tile_t Board_tile            (Board,int,int);
int    Board_joker           (Board,int,int);
  /*int    Board_score           (Board,Round);*/
int    Board_vacant          (Board,int,int);
void   Board_addround        (Dictionary,Board,Round);
void   Board_removeround     (Dictionary,Board,Round);

  /*************************
   * 
   * 
   *************************/

void   Board_testround       (Board,Round);
void   Board_removetestround (Board);
char   Board_gettestchar     (Board,int,int);

  /*************************
   * 
   * board_search.c 
   *************************/

void   Board_search          (Dictionary,Board,Rack,Results);
void   Board_search_first    (Dictionary,Board,Rack,Results);

  /*************************
   * 
   * board_cross.c 
   *************************/

void   Board_buildcross      (Dictionary, Board);

  /*************************
   * 
   * 
   *************************/

int    Board_getwordmultiplier  (Board,int,int);
int    Board_getlettermultiplier(Board,int,int);

  /*************************
   * 
   * 
   *************************/

#if defined(__cplusplus)
	   }
#endif 
#endif
