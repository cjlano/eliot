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

/* $Id: game_internals.h,v 1.2 2004/07/04 18:50:30 ipkiss Exp $ */

#ifndef _GAMESTRUCT_H
#define _GAMESTRUCT_H

#if defined(__cplusplus)
extern "C" {
#endif 

/* Number of played words.
 * There are only 102 tiles, so we should be safe even if only one tile is
 * played at each turn... */
#define PLAYEDRACK_MAX 102

struct tgame {
   Dictionary dic;

   Bag        bag;
   Board      board;
   Results    searchresults;

   Playedrack playedracks [PLAYEDRACK_MAX];
   Round      playedrounds[PLAYEDRACK_MAX];

   int nrounds;
   int points;
};

#if defined(__cplusplus)
   }
#endif 
#endif 
