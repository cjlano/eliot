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

/* $Id: game_internals.h,v 1.3 2004/08/07 18:10:42 ipkiss Exp $ */

#ifndef _GAMESTRUCT_H
#define _GAMESTRUCT_H

#if defined(__cplusplus)
extern "C" {
#endif

/* Number of players. */
#define PLAYERS_MAX 10

typedef enum
{
    TRAINING,
    FREEGAME,
    DUPLICATE
} GameMode;

struct tgame {
   Dictionary dic;

   Bag        bag;
   Board      board;

   Playedrack playedracks [PLAYEDRACK_MAX];
   Round      playedrounds[PLAYEDRACK_MAX];

   int nrounds;
   int points;

   Player     players[PLAYERS_MAX];
   int nplayers;
   int currplayer;

   /* Game mode */
   GameMode mode;
};

#if defined(__cplusplus)
   }
#endif
#endif
