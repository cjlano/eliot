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

/* $Id: board.c,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#include <string.h>
#include <stdlib.h>
#include "dic.h"
#include "tiles.h"
#include "round.h"
#include "bag.h"
#include "rack.h"
#include "results.h"
#include "board.h"
#include "board_internals.h"

#define oo 0
#define __ 1
#define T2 2
#define T3 3
#define W2 2
#define W3 3


const int Board_tile_multipliers[BOARD_REALDIM][BOARD_REALDIM] = 
{
  { oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo },
  { oo,__,__,__,T2,__,__,__,__,__,__,__,T2,__,__,__,oo },
  { oo,__,__,__,__,__,T3,__,__,__,T3,__,__,__,__,__,oo },
  { oo,__,__,__,__,__,__,T2,__,T2,__,__,__,__,__,__,oo },
  { oo,T2,__,__,__,__,__,__,T2,__,__,__,__,__,__,T2,oo },
  { oo,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,oo },
  { oo,__,T3,__,__,__,T3,__,__,__,T3,__,__,__,T3,__,oo },
  { oo,__,__,T2,__,__,__,T2,__,T2,__,__,__,T2,__,__,oo },
  { oo,__,__,__,T2,__,__,__,__,__,__,__,T2,__,__,__,oo },
  { oo,__,__,T2,__,__,__,T2,__,T2,__,__,__,T2,__,__,oo },
  { oo,__,T3,__,__,__,T3,__,__,__,T3,__,__,__,T3,__,oo },
  { oo,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,oo },
  { oo,T2,__,__,__,__,__,__,T2,__,__,__,__,__,__,T2,oo },
  { oo,__,__,__,__,__,__,T2,__,T2,__,__,__,__,__,__,oo },
  { oo,__,__,__,__,__,T3,__,__,__,T3,__,__,__,__,__,oo },
  { oo,__,__,__,T2,__,__,__,__,__,__,__,T2,__,__,__,oo },
  { oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo }
};


const int Board_word_multipliers[BOARD_REALDIM][BOARD_REALDIM] = 
{
  { oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo },
  { oo,W3,__,__,__,__,__,__,W3,__,__,__,__,__,__,W3,oo },
  { oo,__,W2,__,__,__,__,__,__,__,__,__,__,__,W2,__,oo },
  { oo,__,__,W2,__,__,__,__,__,__,__,__,__,W2,__,__,oo },
  { oo,__,__,__,W2,__,__,__,__,__,__,__,W2,__,__,__,oo },
  { oo,__,__,__,__,W2,__,__,__,__,__,W2,__,__,__,__,oo },
  { oo,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,oo },
  { oo,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,oo },
  { oo,W3,__,__,__,__,__,__,W2,__,__,__,__,__,__,W3,oo },
  { oo,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,oo },
  { oo,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,oo },
  { oo,__,__,__,__,W2,__,__,__,__,__,W2,__,__,__,__,oo },
  { oo,__,__,__,W2,__,__,__,__,__,__,__,W2,__,__,__,oo },
  { oo,__,__,W2,__,__,__,__,__,__,__,__,__,W2,__,__,oo },
  { oo,__,W2,__,__,__,__,__,__,__,__,__,__,__,W2,__,oo },
  { oo,W3,__,__,__,__,__,__,W3,__,__,__,__,__,__,W3,oo },
  { oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo }
};


Board
Board_create(void)
{
  Board b;
  b = (Board) malloc(sizeof(struct tboard));
  Board_init(b);
  return b;
}


void 
Board_init(Board b)
{
  int i, j;
  memset(b, 0, sizeof(struct tboard));
  for (i = 1; i <= BOARD_DIM; i++)
    for (j = 1; j <= BOARD_DIM; j++) 
      {
	b->joker_r[i][j] = 0;
	b->joker_c[i][j] = 0;
	b->cross_r[i][j] = CROSS_MASK;
	b->cross_c[i][j] = CROSS_MASK;
	b->point_r[i][j] = -1;
	b->point_c[i][j] = -1;
      }
}


void 
Board_destroy(Board b)
{
  if (b)
    free(b);
}


tile_t 
Board_tile(Board b, int row, int column)
{
  return b->tiles_r[row][column];
}


int
Board_joker(Board b, int row, int column)
{
  return b->joker_r[row][column];
}


int 
Board_vacant(Board b, int row, int column)
{
  if (row < 1 || row > BOARD_DIM ||
      column < 1 || column > BOARD_DIM)
    return 0;
  return b->tiles_r[row][column] == 0;
}


void 
Board_addround(Dictionary d, Board b, Round r)
{
  tile_t t;
  int row, column, i;
     
  row = Round_row(r);
  column = Round_column(r);
  if (Round_dir(r) == HORIZONTAL) 
    {
      for (i = 0; i < Round_wordlen(r); i++)
	if (!b->tiles_r[row][column + i]) 
	  {
	    t = Round_gettile(r,i);
	    b->tiles_r[row][column + i] = t;
	    b->joker_r[row][column + i] = Round_joker(r,i);
	    b->tiles_c[column + i][row] = t;
	    b->joker_c[column + i][row] = Round_joker(r,i);
	  }
    } 
  else 
    {
      for (i = 0; i < Round_wordlen(r); i++)
	if (!b->tiles_r[row + i][column]) 
	  {
	    t = Round_gettile(r,i);
	    b->tiles_r[row + i][column] = t;
	    b->joker_r[row + i][column] = Round_joker(r,i);
	    b->tiles_c[column][row + i] = t;
	    b->joker_c[column][row + i] = Round_joker(r,i);
	  }
    }
  Board_buildcross(d, b);
#ifdef DEBUG
  Board_check_double(b);
#endif
}


void 
Board_removeround(Dictionary d, Board b, Round r)
{
  int row, column, i;

  row = Round_row(r);
  column = Round_column(r);
  if (Round_dir(r) == HORIZONTAL) 
    {
      for (i = 0; i < Round_wordlen(r); i++)
	if (Round_playedfromrack(r,i)) 
	  {
	    b->tiles_r[row][column + i] = 0;
	    b->joker_r[row][column + i] = 0;
	    b->tiles_c[column + i][row] = 0;
	    b->joker_c[column + i][row] = 0;
	  }
    } 
  else 
    {
      for (i = 0; i < Round_wordlen(r); i++)
	if (Round_playedfromrack(r,i)) 
	  {
	    b->tiles_r[row + i][column] = 0;
	    b->joker_r[row + i][column] = 0;
	    b->tiles_c[column][row + i] = 0;
	    b->joker_c[column][row + i] = 0;
	  }
    }
  Board_buildcross(d, b);
#ifdef DEBUG
  Board_check_double(b);
#endif
}


void
Board_testround(Board b, Round r)
{
  tile_t t;
  int row, column, i;

  row = Round_row(r);
  column = Round_column(r);
  if (Round_dir(r) == HORIZONTAL) 
    {
      for (i = 0; i < Round_wordlen(r); i++)
	if (!b->tiles_r[row][column + i]) 
	  {
	    t = Round_gettile(r,i);
	    b->tiles_r[row][column + i] = t;
	    b->joker_r[row][column + i] = Round_joker(r,i);
	    b->tests_r[row][column + i] = 1;

	    b->tiles_c[column + i][row] = t;
	    b->joker_c[column + i][row] = Round_joker(r,i);
	  }
    } 
  else 
    {
      for (i = 0; i < Round_wordlen(r); i++)
	if (!b->tiles_r[row + i][column]) 
	  {
	    t = Round_gettile(r,i);
	    b->tiles_r[row + i][column] = t;
	    b->joker_r[row + i][column] = Round_joker(r,i);
	    b->tests_r[row + i][column] = 1;

	    b->tiles_c[column][row + i] = t;
	    b->joker_c[column][row + i] = Round_joker(r,i);
	  }
    }
}


void
Board_removetestround(Board b)
{
  int i,j;
  for(i=1; i <= BOARD_DIM; i++)
    for(j=1; j <= BOARD_DIM; j++)
      {
	if (b->tests_r[i][j])
	  {
	    b->tiles_r[i][j] = 0;
	    b->tests_r[i][j] = 0;
	    b->joker_r[i][j] = 0;

	    b->tiles_c[j][i] = 0;
	    b->joker_c[j][i] = 0;
	  }
      }
}


char
Board_gettestchar(Board b,int r,int c)
{
  return b->tests_r[r][c];
}

int   
Board_getwordmultiplier(Board b,int r,int c)
{
  b++; /* unused */
  if (r < BOARD_MIN || r > BOARD_MAX ||
      c < BOARD_MIN || c > BOARD_MAX)
    return 0;
  return Board_word_multipliers[r][c];
}

int   
Board_getlettermultiplier(Board b, int r, int c)
{
  b++; /* unused */
  if (r < BOARD_MIN || r > BOARD_MAX ||
      c < BOARD_MIN || c > BOARD_MAX)
    return 0;
  return Board_tile_multipliers[r][c];
}

#ifdef DEBUG
void
Board_check_double(Board b)
{
  int r,c;
  for(r=BOARD_MIN; r <= BOARD_MAX; r++)
    for(c=BOARD_MIN; c <= BOARD_MAX; c++)
      {
	if (b->tiles_r[r][c] != b->tiles_c[c][r])
	  printf("tiles diff %d %d\n",r,c);

	if (b->cross_r[r][c] != b->cross_c[c][r])
	  printf("cross diff %d %d\n",r,c);

	if (b->point_r[r][c] != b->point_c[c][r])
	  printf("point diff %d %d\n",r,c);

	if (b->joker_r[r][c] != b->joker_c[c][r])
	  printf("joker diff %d %d\n",r,c);
      }
  printf("**\n");
}
#endif
