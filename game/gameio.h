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

/* $Id: gameio.h,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#ifndef _GAMEIO_H
#define _GAMEIO_H

#if defined(__cplusplus)
extern "C" {
#endif 

void Game_print_board(FILE*,Game);
void Game_print_board_joker(FILE*,Game);
void Game_print_board_point(FILE*,Game);
void Game_print_nonplayed(FILE*,Game);
void Game_print_playedrack(FILE*,Game,int);
void Game_print_searchresults(FILE*,Game,int);
  
int  Game_read_game(FILE*,Game);
void Game_print_game(FILE*,Game);


#if defined(__cplusplus)
   }
#endif 
#endif /* _GAME_H */
