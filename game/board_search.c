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

/* $Id: board_search.c,v 1.1 2004/04/08 09:43:06 afrab Exp $  */

#include <stdio.h>
#include <string.h>
#include <dic.h>
#include "tiles.h"
#include "bag.h"
#include "rack.h"
#include "round.h"
#include "results.h"
#include "board.h"
#include "board_internals.h"

#include "debug.h"

/*
 * computes the score of a word, coordinates may be changed to reflect
 * the real direction of the word 
 */
static void
Boardsearch_evalmove(
         tile_t tiles[BOARD_REALDIM][BOARD_REALDIM],
         int points[BOARD_REALDIM][BOARD_REALDIM],
	 char joker[BOARD_REALDIM][BOARD_REALDIM],
         Results results, Round word)
{
     int i,pts,ptscross;
     int l,t,fromrack;
     int len,row,col,wordmul;
     
     fromrack = 0;
     pts      = 0;
     ptscross = 0;
     wordmul  = 1;

     len = Round_wordlen(word);

     row = Round_row(word);
     col = Round_column(word);

     for(i=0; i < len; i++) 
       {
	 if (tiles[row][col+i]) 
	   {
	     if (! joker[row][col+i])
	       pts += Tiles_points[Round_gettile(word,i)];
	   }
	 else 
	   {
	     if (! Round_joker(word,i))
	       l = Tiles_points[Round_gettile(word,i)] *
		 Board_tile_multipliers[row][col+i];
	     else
	       l = 0;
	     pts += l;
	     wordmul *= Board_word_multipliers[row][col+i];
	     
	     t = points[row][col+i];
	     if (t >= 0)
	       ptscross += (t + l) * Board_word_multipliers[row][col+i];
	     fromrack++;
          }
       }
     pts = ptscross + pts * wordmul + 50 * (fromrack == 7);
     Round_setbonus(word,fromrack == 7);
     Round_setpoints(word,pts);

     if (Round_dir(word) == VERTICAL) {
          Round_setrow(word,col);
          Round_setcolumn(word,row);
     }
     Results_addsorted(results,word);
     if (Round_dir(word) == VERTICAL) {
          Round_setrow(word,row);
          Round_setcolumn(word,col);
     }
}


static void
ExtendRight(Dictionary dic, 
            tile_t tiles[BOARD_REALDIM][BOARD_REALDIM],
            unsigned int  cross[BOARD_REALDIM][BOARD_REALDIM],
            int points[BOARD_REALDIM][BOARD_REALDIM],
	    char joker[BOARD_REALDIM][BOARD_REALDIM],
            Rack rack, Round partialword,
            Results results, unsigned int n, int row, int column, int anchor)
{
     tile_t l;
     unsigned int succ;

     if (! tiles[row][column]) {

          if (Dic_word(dic,n) && column > anchor) 
               Boardsearch_evalmove(tiles,points,joker,results,partialword);
          
          for(succ = Dic_succ(dic,n); succ ; succ = Dic_next(dic,succ)) {
               l = Dic_chr(dic,succ);
               if (cross[row][column] & (1 << l)) {
                    if (Rack_in(rack,l)) {
                         Rack_remove(rack,l);
                         Round_addrightfromrack(partialword,l,0);
                         ExtendRight(dic,tiles,cross,points,joker,
                                     rack,partialword,results,
				     succ,row,column + 1,anchor);
                         Round_removerighttorack(partialword,l,0);
                         Rack_add(rack,l);
                    }
                    if (Rack_in(rack,(tile_t)JOKER_TILE)) {
                         Rack_remove(rack,(tile_t)JOKER_TILE);
                         Round_addrightfromrack(partialword,l,1);
                         ExtendRight(dic,tiles,cross,points,joker,
                                     rack,partialword,results,
				     succ,row,column + 1,anchor);
                         Round_removerighttorack(partialword,l,1);
                         Rack_add(rack,JOKER_TILE);
                    }
               }
          }
     } else {
          l = tiles[row][column];
          for(succ = Dic_succ(dic,n); succ ; succ = Dic_next(dic,succ)) {
               if (Dic_chr(dic,succ) == l) {
                    Round_addrightfromboard(partialword,l);
                    ExtendRight(dic,tiles,cross,points,joker,
                                rack,partialword,
                                results,succ,row,column + 1,anchor);
                    Round_removerighttoboard(partialword,l);
               }
          }
     }
}


static void
LeftPart(Dictionary dic, 
         tile_t tiles[BOARD_REALDIM][BOARD_REALDIM],
         unsigned int cross[BOARD_REALDIM][BOARD_REALDIM],
         int points[BOARD_REALDIM][BOARD_REALDIM],
	 char joker[BOARD_REALDIM][BOARD_REALDIM],
         Rack rack, Round partialword,
         Results results, int n, int row, int anchor, int limit)
{
     tile_t l;
     int succ;

     ExtendRight(dic,tiles,cross,points,joker,rack,
                 partialword,results,n,row,anchor,anchor);

     if (limit > 0) {
          for(succ = Dic_succ(dic,n); succ ; succ = Dic_next(dic,succ)) {
               l = Dic_chr(dic,succ);
               if (Rack_in(rack,l)) {
                    Rack_remove(rack,l);
                    Round_addrightfromrack(partialword,l,0);
                    Round_setcolumn(partialword,Round_column(partialword) - 1);
                    LeftPart(dic,tiles,cross,points,joker,
                             rack,partialword,results,
                             succ,row,anchor,limit - 1);
                    Round_setcolumn(partialword,Round_column(partialword) + 1);
                    Round_removerighttorack(partialword,l,0);
                    Rack_add(rack,l);
               }
               if (Rack_in(rack,JOKER_TILE)) {
                    Rack_remove(rack,JOKER_TILE);
                    Round_addrightfromrack(partialword,l,1);
                    Round_setcolumn(partialword,Round_column(partialword) - 1);
                    LeftPart(dic,tiles,cross,points,joker,
                             rack,partialword,results,
                             succ,row,anchor,limit - 1);
                    Round_setcolumn(partialword,Round_column(partialword) + 1);
                    Round_removerighttorack(partialword,l,1);
                    Rack_add(rack,JOKER_TILE);
               }
          }
     } 
}

static void
Board_search_aux(Dictionary dic,
                 tile_t tiles[BOARD_REALDIM][BOARD_REALDIM],
                 unsigned int cross[BOARD_REALDIM][BOARD_REALDIM],
                 int points[BOARD_REALDIM][BOARD_REALDIM],
		 char joker[BOARD_REALDIM][BOARD_REALDIM],
                 Rack rack,Results results, Direction dir)
{
     int row,column,lastanchor;
     Round partialword;
     
     partialword = Round_create();
     for(row = 1; row <= BOARD_DIM; row++) {
          Round_init(partialword);
          Round_setdir(partialword,dir);
          Round_setrow(partialword,row);
          lastanchor = 0;
          for(column = 1; column <= BOARD_DIM; column++) {
               if (! tiles[row][column] &&
                   (tiles[row][column - 1] || tiles[row][column + 1] ||
                    tiles[row - 1][column] || tiles[row + 1][column])) 
               {
                    if (tiles[row][column - 1]) {
                         Round_setcolumn(partialword,lastanchor + 1);
                         ExtendRight(dic,tiles,cross,points,joker,
                                     rack,partialword,results,
                                     Dic_root(dic),row,lastanchor + 1,column);
                    } else {
                         Round_setcolumn(partialword,column);
                         LeftPart(dic,tiles,cross,points,joker,
                                  rack,partialword,results,
                                  Dic_root(dic),row,column,column -
                                  lastanchor - 1);
                    }
                    lastanchor = column;
               }
          }
     }
     Round_destroy(partialword);
}

void
Board_search(Dictionary dic,Board board, Rack rack,Results results)
{
     Board_search_aux(dic,board->tiles_r,board->cross_r,
                      board->point_r,board->joker_r,
		      rack,results,HORIZONTAL);

     Board_search_aux(dic,board->tiles_c,board->cross_c,
                      board->point_c,board->joker_c,
		      rack,results,VERTICAL);
}

void
Board_search_first(Dictionary dic, Board board, Rack rack, Results results)
{
     Round partialword;
     int row = 8,column = 8;

     partialword = Round_create();
     Round_setrow(partialword,row);
     Round_setcolumn(partialword,column);
     Round_setdir(partialword,HORIZONTAL);

     LeftPart(dic,board->tiles_r,board->cross_r,board->point_r,board->joker_r,
              rack,partialword,results,Dic_root(dic),row,column,
              Rack_ntiles(rack) - 1);

     Round_destroy(partialword);
}
