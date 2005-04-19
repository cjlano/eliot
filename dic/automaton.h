/* Eliot                                                                     */
/* Copyright (C) 1999  antoine.fraboulet                                     */
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
/*
 * $Id: automaton.h,v 1.4 2005/04/19 16:26:51 afrab Exp $
 */

#ifndef _DIC_AUTOMATON_H_
#define _DIC_AUTOMATON_H_

struct _automaton {
  int nterm;       /* (1 << nterm) == index of maximum state */
  int nstate;      /* number of states                       */
  int init;        /* index of initial state                 */
  int **Dtrans;    /* Dtrans[state][letter] == next state    */
  int *accept;     /* accept[state] == 1 -> accept state     */
  int *marque;     /* marque[state] == 1 -> valid state      */
};

typedef struct _automaton* automaton;

automaton automaton_build (int init_state, int *ptl, int *PS);
void      automaton_delete(automaton a);

#ifdef DEBUG_RE
void      automaton_dump  (automaton a, char* filename);
#endif
#endif
