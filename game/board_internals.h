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

/* $Id: board_internals.h,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#ifndef _BOARD_INTERNALS_H_
#define _BOARD_INTERNALS_H_

#define BOARD_MIN 1
#define BOARD_MAX 15
#define BOARD_DIM 15
#define BOARD_REALDIM (BOARD_DIM + 2)

#define CROSS_MASK 0x7FFFFFE

struct tboard {
  tile_t tiles_r[BOARD_REALDIM][BOARD_REALDIM];
  tile_t tiles_c[BOARD_REALDIM][BOARD_REALDIM];

  unsigned int cross_r[BOARD_REALDIM][BOARD_REALDIM];
  unsigned int cross_c[BOARD_REALDIM][BOARD_REALDIM];

  int point_r[BOARD_REALDIM][BOARD_REALDIM];
  int point_c[BOARD_REALDIM][BOARD_REALDIM];

  char joker_r[BOARD_REALDIM][BOARD_REALDIM];
  char joker_c[BOARD_REALDIM][BOARD_REALDIM];

  char tests_r[BOARD_REALDIM][BOARD_REALDIM];
};

extern const int Board_tile_multipliers[BOARD_REALDIM][BOARD_REALDIM];
extern const int Board_word_multipliers[BOARD_REALDIM][BOARD_REALDIM];

#ifdef DEBUG
void Board_check_double(Board b);
#endif

#endif
