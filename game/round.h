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

/* $Id: round.h,v 1.2 2004/08/07 18:10:42 ipkiss Exp $ */

#ifndef _ROUND_H_
#define _ROUND_H_

#if defined(__cplusplus)
extern "C" {
#endif 

  /*************************
   * A Round is 
   * 
   *************************/

typedef struct tround* Round;

enum Tdirection {VERTICAL, HORIZONTAL};
typedef enum Tdirection Direction;

  /*************************
   * 
   * 
   *************************/

Round     Round_create             (void);
void      Round_init               (Round);
void      Round_destroy            (Round);
void      Round_copy               (Round dst, Round src);

  /*************************
   * 
   * 
   *************************/

void      Round_addrightfromboard  (Round,tile_t);
void      Round_removerighttoboard (Round,tile_t);
void      Round_addrightfromrack   (Round,tile_t,int);
void      Round_removerighttorack  (Round,tile_t,int);

  /*************************
   * 
   * 
   *************************/

void      Round_setword            (Round,tile_t*);
void      Round_setrow             (Round,int);
void      Round_setcolumn          (Round,int);
void      Round_setpoints          (Round,int);
void      Round_setdir             (Round,Direction);
void      Round_setbonus           (Round,int);
void      Round_setfromrack        (Round,int);
void      Round_setfromboard       (Round,int);
void      Round_setjoker           (Round,int);

  /*************************
   * 
   * 
   *************************/

int       Round_wordlen            (Round);
tile_t    Round_gettile            (Round,int);
int       Round_joker              (Round,int);
int       Round_playedfromrack     (Round,int);

  /*************************
   * 
   * 
   *************************/

int       Round_row                (Round);
int       Round_column             (Round);
int       Round_points             (Round);
int       Round_bonus              (Round);
Direction Round_dir                (Round);

  /*************************
   * 
   * 
   *************************/

#if defined(__cplusplus)
	   }
#endif 
#endif
