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

/* $Id: game.c,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "dic.h"
#include "tiles.h"
#include "bag.h"
#include "rack.h"
#include "round.h"
#include "pldrack.h"
#include "results.h"
#include "board.h"
#include "game.h"
#include "gameio.h"
#include "game_internals.h"

#include "debug.h"

/*********************************************************
 *********************************************************/

Game  
Game_create(Dictionary dic)
{
  int i;
  Game g;

  g = (Game)malloc(sizeof(struct tgame));
  if (g == NULL)
    return NULL;

  g->dic           = dic;
  g->bag           = Bag_create();
  g->board         = Board_create();
  g->searchresults = Results_create();
    
  for(i=0; i < PLAYEDRACK_MAX; i++) 
    {
      g->playedrounds[i] = Round_create();
      g->playedracks[i]  = Playedrack_create();
    }
  
  Game_init(g);
  return g;
}


void  
Game_destroy(Game g)
{
     int i;
     if (g) {
          Bag_destroy(g->bag);
          Board_destroy(g->board);
          Results_destroy(g->searchresults);
          for(i=0; i < PLAYEDRACK_MAX; i++) {
               Round_destroy(g->playedrounds[i]);
               Playedrack_destroy(g->playedracks[i]);
          }
          free(g);
     }
}

/*********************************************************
 *********************************************************/

void  
Game_init(Game g)
{
  int i;
  Bag_init(g->bag);
  Board_init(g->board);
  Results_init(g->searchresults);
  for(i=0; i < PLAYEDRACK_MAX; i++) 
    {
      Round_init(g->playedrounds[i]);
      Playedrack_init(g->playedracks[i]);
    }
  g->points  = 0;
  g->nrounds = 0;
}


int
Game_load(Game game, FILE* fin)
{
  return Game_read_game(fin,game);
}


void
Game_save(Game game, FILE* fout)
{
  Game_print_game(fout,game);
}


/*********************************************************
 *********************************************************/


Dictionary
Game_getdic(Game g)
{
  return g->dic;
}



void
Game_setdic(Game g, Dictionary d)
{
  g->dic = d;
}


/*********************************************************
 *********************************************************/

int
Game_search(Game game)
{
  Rack rack;
  if (game->dic == NULL)
    return 1;
  
  Game_removetestplay(game);
  Results_init(game->searchresults);

  rack = Rack_create();
  Playedrack_getrack(game->playedracks[game->nrounds],rack);
  if (game->nrounds)
    Board_search(game->dic,game->board,
		 rack,
		 game->searchresults);
  else
    Board_search_first(game->dic,game->board, 
		       rack,
		       game->searchresults);
  Rack_destroy(rack);
  return 0;
}

/* 
   This function returns a rack "dest" with the unplayed tiles from
   the current round. 
   03 sept 2000 : We have to sort the tiles according to the new rules 
*/
static void
Game_restfromround(Playedrack dest, Playedrack source, Round round)
{
     tile_t i;
     Rack r;
     
     r = Rack_create();
     Playedrack_getrack(source,r);
     
     /* we remove the played tiles from rack r */
     for(i=0; i < Round_wordlen(round); i++) 
       {
          if (Round_playedfromrack(round,i)) 
	    {
	      if (Round_joker(round,i))
		Rack_remove(r,(tile_t)JOKER_TILE);
	      else
		Rack_remove(r,Round_gettile(round,i));
	    }
       }
      
     Playedrack_init(dest);
     Playedrack_setold(dest,r);
     Rack_destroy(r);
}


int
Game_play(Game game, int n) 
{
  /*
   * We remove tiles from the bag only when they are played 
   * on the board. When going back in the game, we must only
   * replace played tiles. 
   * We test a rack when it is set but tiles are left in
   * the bag.
   */

  int i;
  Round round;
  
  if (game->dic == NULL)
    return 1;
  
  if ((round = Results_get(game->searchresults,n))==NULL)
    return 2;
  
  Round_copy(game->playedrounds[game->nrounds],round);
  Board_addround(game->dic,game->board,round);
  game->points += Round_points(round);
  Game_restfromround(game->playedracks[game->nrounds + 1],
		     game->playedracks[game->nrounds],
		     round);
  for(i=0; i < Round_wordlen(round); i++) {
    if (Round_playedfromrack(round,i)) {
      if (Round_joker(round,i))
	Bag_taketile(game->bag,(tile_t)JOKER_TILE);
      else
	Bag_taketile(game->bag,Round_gettile(round,i));
    }
  }
  game->nrounds++;
  Results_init(game->searchresults);
  return 0;
}


int
Game_back(Game game, int n)
{
     int i,j;
     Round lastround;

     if (game->dic == NULL)
	  return 1;

     for(i=0 ; i < n; i++) {
          if (game->nrounds) {
               game->nrounds--;
               lastround = game->playedrounds[game->nrounds];
               game->points -= Round_points(lastround);
               Board_removeround(game->dic,game->board,lastround);
               for(j=0; j < Round_wordlen(lastround); j++) {
                    if (Round_playedfromrack(lastround,j)) {
                         if (Round_joker(lastround,j))
                              Bag_replacetile(game->bag,JOKER_TILE);
                         else
                              Bag_replacetile(game->bag,Round_gettile(lastround,j));
                    }
               }
               Round_init(lastround);
          }
     }
     return 0;
}

int 
Game_testplay(Game game, int n)
{
  Round round;
  
  if ((round = Results_get(game->searchresults,n))==NULL)
    return 2;
  Board_testround(game->board,round);
  return 0;
}

int 
Game_removetestplay(Game game)
{
  Board_removetestround(game->board);
  return 0;
}

/*********************************************************
 *********************************************************/

int   
Game_getpoints(Game g)
{
     return g->points;
}

char
Game_getboardchar(Game g, int r, int c)
{
     tile_t t;
     char l = 0;
     t = Board_tile(g->board,r,c);
     if (t)
       {
          l = codetochar(t);
          if (Board_joker(g->board,r,c))
               l = tolower(l);
       }
     return l;
}

int
Game_getboardcharattr(Game g,int r,int c)
{
  int t = Board_gettestchar(g->board,r,c);
  int j = Board_joker(g->board,r,c);
  return  (t << 1) | j;
}

int  
Game_getboardwordmultiplier(Game g,int r,int c)
{
  return Board_getwordmultiplier(g->board,r,c);
}

int  
Game_getboardlettermultiplier(Game g,int r,int c)
{ 
  return Board_getlettermultiplier(g->board,r,c);
}

/*********************************************************
 *********************************************************/

int 
Game_setrack_random(Game game, int check, set_rack_mode mode)
{
     int i,c,v,min,nold;
     tile_t l;
     Bag b;
     Playedrack p;

/* create a copy of the bag in which we can do everything we want */
     b = Bag_create();
     Bag_copy(b,game->bag);

     if (Bag_ntiles(b) == 0)
         return 1;

     v = Bag_nvowels(b);
     c = Bag_nconsonants(b);
     min = 0;

     if (check)
     {
         if (v == 0 || c == 0)
             return 1;
         if (Bag_nvowels(b) > 1 && Bag_nconsonants(b) > 1
             && Game_getnrounds(game) < 15)
             min = 2;
         else
             min = 1;
     }

     p = game->playedracks[game->nrounds];
     nold = Playedrack_nold(p);

/* "complement" with an empty rack is equivalent to ALL */

     if (mode == RACK_ALL || nold == 0)
       {
	  Playedrack_init(p);
	  for(i=0; (Bag_ntiles(b) != 0) && (i < RACK_MAX); i++)
            {
              l = Bag_select_random(b);
	      Bag_taketile(b,l);
	      Playedrack_addold(p,l);
            }
       }
     else
       {
          tile_t ttmp[RACK_MAX];
          int oldc = 0;
          int oldv = 0;

/* we flush the "new" part of the rack */

	  Playedrack_resetnew(p);

/* "old" part must be taken into account in the bag */

	  Playedrack_getoldtiles(p,ttmp);
	  for(i=0; ttmp[i] ; i++)
            {
               if (Tiles_consonants[ttmp[i]])
                   oldc++;
               if (Tiles_vowels[ttmp[i]])
                   oldv++;
               Bag_taketile(b,ttmp[i]);
            }

          /* make sure we can complete the rack */
          if (check)
            {
              /* RACK_MAX - nold is the number of letters to add */
              if (min > oldc + RACK_MAX - nold ||
                  min > oldv + RACK_MAX - nold)
                  return 3;
            }

/* take new tiles from the bag */

	  for(i=nold; (Bag_ntiles(b) != 0) && (i < RACK_MAX); i++)
            {
              l = Bag_select_random(b);
              Bag_taketile(b,l);
              Playedrack_addnew(p,l);
            }
       }

     Bag_destroy(b);

     if (check)
       return Playedrack_check_rack(p, min) ? 0 : 2;
     else
       return 0; /* everything is ok */
}

static int 
Game_rackinbag(Rack r, Bag b)
{
  int i;
  for(i=0; i < TILES_NUMBER; i++)
    if (Rack_in(r,i) > Bag_in(b,i))
      return 0;
  return 1;
}


int
Game_setrack_manual(Game game, int check, const char *t)
{
     int i,min,ret;
     tile_t l;
     Playedrack p;
     Rack rack;

     p = game->playedracks[game->nrounds];
     
     if (t == NULL || t[0] == 0)
       {
	  Playedrack_init(p);
	  return 0;
       }
	 
     Playedrack_init(p);
     for(i=0; t[i] != 0 && t[i] != '+'; i++)
       {
	  if ((l = chartocode(t[i]))==0)
            {
	       return 1;
            }
	  Playedrack_addold(p,l);
       }
 
     if (t[i] == '+')
       {
	  for(i++; t[i] ; i++)
            {
	       if ((l = chartocode(t[i])) == 0)
                 {
		    return 1;
                 }
	       Playedrack_addnew(p,l);
            }
       }

     rack = Rack_create(); 
     Playedrack_getrack(p,rack); 
     if ((ret = Game_rackinbag(rack,game->bag)) == 0)
       {
	  Playedrack_init(p); 
	  Rack_destroy(rack); 
	  return 1;
       } 
     Rack_destroy(rack); 

     if (check)
       {
         if (Bag_nvowels(game->bag) > 1 && Bag_nconsonants(game->bag) > 1
             && Game_getnrounds(game) < 15)
             min = 2;
         else
             min = 1;
         return Playedrack_check_rack(p, min) ? 0 : 2;
       }
     else
       return 0; /* everything is ok */
}

/*********************************************************
 *********************************************************/


int
Game_getcharinbag(Game g, char c)
{
     return Bag_in(g->bag,chartocode(c));
}


/*********************************************************
 *********************************************************/


int
Game_getnrounds(Game g)
{
     return g->nrounds;
}



void
Game_getplayedrack(Game g, int num, char buff[RACK_SIZE_MAX])
{
     int i,l = 0;
     Playedrack p;
     tile_t bt[RACK_SIZE_MAX];
     buff[0] = 0;
     if (num < 0 || num > g->nrounds) 
          return;
     p = g->playedracks[num];
     Playedrack_getoldtiles(p, bt);
     for(i=0; bt[i]; i++)
          buff[l++] = codetochar(bt[i]);
     Playedrack_getnewtiles(p, bt);

     if (i && bt[0])
          buff[l++] = '+';

     for(i=0; bt[i]; i++)
          buff[l++] = codetochar(bt[i]);
     buff[l] = 0;
}



void
Game_getplayedword(Game g, int num, char buff[WORD_SIZE_MAX])
{
     Round r;
     char c;
     int i,l = 0;
     buff[0] = 0;
     if (num < 0 || num >= g->nrounds) 
          return;
     r = g->playedrounds[num];
     for(i=0; i < Round_wordlen(r); i++) {
          c = codetochar(Round_gettile(r,i));
          if (Round_joker(r,i))
               c = tolower(c);
          buff[l++] = c;
     }
     buff[i] = 0;
}



void 
Game_getplayedfirstcoord (Game g,int num,char buff[COOR_SIZE_MAX])
{
     Round r;
     buff[0] = 0;
     if (num < 0 || num >= g->nrounds) 
          return;
     r = g->playedrounds[num];
     if (Round_dir(r) == HORIZONTAL)
          sprintf(buff,"%c",Round_row(r) + 'A' - 1);
     else
          sprintf(buff,"%d",Round_column(r));
}



void 
Game_getplayedsecondcoord(Game g,int num,char buff[COOR_SIZE_MAX])
{
     Round r;
     buff[0] = 0;
     if (num < 0 || num >= g->nrounds) 
          return;
     r = g->playedrounds[num];
     if (Round_dir(r) == HORIZONTAL)
          sprintf(buff,"%d",Round_column(r));
     else
          sprintf(buff,"%c",Round_row(r) + 'A' - 1);
}



void
Game_getplayedcoord(Game g, int num, char buff[COOR_SIZE_MAX])
{
     char c1[COOR_SIZE_MAX];
     char c2[COOR_SIZE_MAX];
     Game_getplayedfirstcoord(g,num,c1);
     Game_getplayedsecondcoord(g,num,c2);
     sprintf(buff,"%2s%2s",c1,c2);
}



int  
Game_getplayedpoints(Game g, int num)
{
     if (num < 0 || num >= g->nrounds) 
          return 0;
     return Round_points(g->playedrounds[num]);
}



int
Game_getplayedbonus(Game g, int num)
{
     if (num < 0 || num >= g->nrounds) 
          return 0;
     return Round_bonus(g->playedrounds[num]);
}

/*********************************************************
 *********************************************************/

int
Game_getnresults(Game g)
{
  return Results_in(g->searchresults);
}



void 
Game_getsearchedword(Game g, int num ,char buff[WORD_SIZE_MAX])
{
     Round r;
     char c; 
     int i,l = 0;
     buff[0] = 0;
     if (num < 0 || num >= Results_in(g->searchresults)) 
          return;
     r = Results_get(g->searchresults,num);
     for(i=0; i < Round_wordlen(r); i++) {
          c = codetochar(Round_gettile(r,i));
          if (Round_joker(r,i))
               c = tolower(c);
          buff[l++] = c;
     }
     buff[i] = 0;
}



void 
Game_getsearchedfirstcoord(Game g,int num,char buff[COOR_SIZE_MAX])
{
     Round r;
     buff[0] = 0;
     if (num < 0 || num >= Results_in(g->searchresults)) 
          return;
     r = Results_get(g->searchresults,num);
     if (Round_dir(r) == HORIZONTAL)
          sprintf(buff,"%c",Round_row(r) + 'A' - 1);
     else
          sprintf(buff,"%d",Round_column(r));
}



void 
Game_getsearchedsecondcoord(Game g, int num, char buff[COOR_SIZE_MAX])
{
     Round r;
     buff[0] = 0;
     if (num < 0 || num >= Results_in(g->searchresults)) 
          return;
     r = Results_get(g->searchresults,num);
     if (Round_dir(r) == HORIZONTAL)
          sprintf(buff,"%d",Round_column(r));
     else
          sprintf(buff,"%c",Round_row(r) + 'A' - 1);
}



void
Game_getsearchedcoord(Game g, int num, char buff[COOR_SIZE_MAX])
{
     char c1[COOR_SIZE_MAX];
     char c2[COOR_SIZE_MAX];
     Game_getsearchedfirstcoord(g,num,c1);
     Game_getsearchedsecondcoord(g,num,c2);
     sprintf(buff,"%2s%2s",c1,c2);
}



int  
Game_getsearchedpoints(Game g, int num)
{
     if (num < 0 || num >= Results_in(g->searchresults)) 
          return 0;
     return Round_points(Results_get(g->searchresults,num));
}



int
Game_getsearchedbonus(Game g, int num)
{
     if (num < 0 || num >= Results_in(g->searchresults)) 
          return 0;
     return Round_bonus(Results_get(g->searchresults,num));
}
