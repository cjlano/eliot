/* Eliot                                                                     */
/* Copyright (C) 1999-2004 Eliot                                             */
/* Antoine Fraboulet <antoine.fraboulet@free.fr>                             */
/* Olivier Teuliere  <ipkiss@via.ecp.fr>                                     */
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

/* $Id: game.h,v 1.2 2004/08/07 18:10:42 ipkiss Exp $ */

#ifndef _GAME_H_
#define _GAME_H_

#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif

  /*************************
   * Ident string used to identify saved Eliot games
   *************************/

#define IDENT_STRING "Eliot"

  /*************************
   * Dimensions of the board, the tiles placed on
   * the board can be accessed via Game_getboardchar()
   *************************/

#define BOARD_MIN 1
#define BOARD_MAX 15

  /*************************
   * Dimensions of the strings that are used to return
   * values to the GUI
   *************************/

#define WORD_SIZE_MAX               16
#define RACK_SIZE_MAX               10
#define COOR_SIZE_MAX                6

  /*************************
   * Functions to create and destroy a game
   * the dictionary does not belong to the
   * game (ie: it won't be destroyed by Game_destroy)
   *
   * The dictionary can be set to NULL and changed
   * afterwards by Game_setdic
   *************************/

typedef struct tgame* Game;

Game       Game_create           (Dictionary);
void       Game_destroy          (Game);

  /*************************
   * handling games
   * Game_init will set up a new (empty) game
   *
   * Game_load return :
   *  0 : ok
   *  1 : bad identstring
   * Game_load might need some more work to be robust enough to
   * handling "hand written" files
   *************************/

void       Game_init             (Game);
int        Game_load             (Game,FILE*);
void       Game_save             (Game,FILE*);

  /*************************
   * Dictionary associated with the game
   * The dictionary can be changed during a
   * game without problem
   *************************/

Dictionary Game_getdic           (Game);
void       Game_setdic           (Game,Dictionary);

  /*************************
   * Playing the game
   * the int parameter should be 0 <= int < Game_getnrounds
   *
   * testplay will place a temporary word on the board for
   * preview purpose
   * return value is
   *  0 : ok
   *  1 : dictionary is set to NULL
   *  2 : not enough played rounds for the request
   *************************/

int  Game_training_search        (Game);
int  Game_training_play          (Game,const char[COOR_SIZE_MAX],const char*);
int  Game_training_playresult    (Game,int);
int  Game_back                   (Game,int);
int  Game_testplay               (Game,int);
int  Game_removetestplay         (Game);

  /*************************
   * int coordinates have to be
   * BOARD_MIN <= int <= BOARD_MAX
   *
   * Game_getboardchar returns an upper case letter
   * for normal tiles and a lower case letter for jokers.
   *
   * Game_getboardcharattr tells the attributes of the tile
   *   0 : normal played tile
   *   1 : joker tile
   *   2 : test tile for preview purpose
   * attributes can be combined with the or (|) operator
   *************************/

#define ATTR_NORMAL 0
#define ATTR_JOKER  1
#define ATTR_TEST   2

char Game_getboardchar           (Game,int,int);
int  Game_getboardcharattr       (Game,int,int);

int  Game_getboardwordmultiplier  (Game,int,int);
int  Game_getboardlettermultiplier(Game,int,int);

  /*************************
   * Set the rack for searching
   *
   * The int parameter is a boolean, if this parameter
   * set the rack will check that there are at least
   * 2 vowels and 2 consonants before the round 15.
   *
   * The setrackmanual parameter string has to contain
   * 'a' <= char <= 'z' or 'A' <= char <= 'Z' or '?'
   *
   * return value
   *    0 : the rack has been set
   *    1 : the bag does not contain enough tiles
   *    2 : the rack check was set on and failed
   *    3 : the rack cannot be completed (Game_*_setrackrandom only)
   *************************/

#define RACK_MAX 7
typedef enum {RACK_ALL, RACK_NEW} set_rack_mode;

int  Game_training_setrackmanual (Game,int,const char*);
int  Game_training_setrackrandom (Game,int,set_rack_mode);
int  Game_freegame_setrackrandom (Game,int,int,set_rack_mode);
int  Game_duplicate_setrackrandom(Game,int,int,set_rack_mode);

  /*************************
   * Get the number of tiles available in the bag.
   * The parameter has to be
   * 'a' <= char <= 'z' or 'A' <= char <= 'Z' or '?'
   *************************/

int  Game_getcharinbag           (Game,char);

  /*************************
   * Functions to access already played words
   * The int parameter should be 0 <= int < getnrounds
   *************************/

int  Game_getnrounds             (Game);
void Game_getplayedrack          (Game,int,char [RACK_SIZE_MAX]);
void Game_getplayedword          (Game,int,char [WORD_SIZE_MAX]);
void Game_getplayedfirstcoord    (Game,int,char [COOR_SIZE_MAX]);
void Game_getplayedsecondcoord   (Game,int,char [COOR_SIZE_MAX]);
void Game_getplayedcoord         (Game,int,char [COOR_SIZE_MAX]);
int  Game_getplayedpoints        (Game,int);
int  Game_getplayedbonus         (Game,int);

  /*************************
   * Functions to access the current search results
   * The int parameter should be 0 <= int < getnresults
   *************************/

int  Game_getnresults            (Game);
void Game_getsearchedword        (Game,int,char [WORD_SIZE_MAX]);
void Game_getsearchedfirstcoord  (Game,int,char [COOR_SIZE_MAX]);
void Game_getsearchedsecondcoord (Game,int,char [COOR_SIZE_MAX]);
void Game_getsearchedcoord       (Game,int,char [COOR_SIZE_MAX]);
int  Game_getsearchedpoints      (Game,int);
int  Game_getsearchedbonus       (Game,int);

  /*************************
   * Functions to access players.
   * The int parameter should be 0 <= int < getnplayers
   *************************/

int  Game_getnplayers            (Game);
void Game_addhumanplayer         (Game);
void Game_addaiplayer            (Game);
int  Game_getplayerpoints        (Game,int);

int  Game_currplayer             (Game);
void Game_prevplayer             (Game);
void Game_nextplayer             (Game);

  /*************************
   * Training mode
   *************************/

int  Game_training_start         (Game);
void Game_training_endturn       (Game);

  /*************************
   * Free game mode
   *************************/

int  Game_freegame_start         (Game);
int  Game_freegame_play          (Game,const char[COOR_SIZE_MAX],const char*);
int  Game_freegame_endturn       (Game);
int  Game_freegame_pass          (Game,const char*,int);

  /*************************
   * Duplicate mode
   *************************/

int  Game_duplicate_start        (Game);
int  Game_duplicate_play         (Game,const char[COOR_SIZE_MAX],const char*);
void Game_duplicate_endturn      (Game);
int  Game_duplicate_setplayer    (Game,int);

#if defined(__cplusplus)
   }
#endif
#endif /* _GAME_H */
