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
 * $Id: automaton.h,v 1.3 2005/04/09 19:16:09 afrab Exp $
 */

#ifndef _DIC_AUTOMATON_H_
#define _DIC_AUTOMATON_H_

struct _automaton {
  int nterm;
  int nstate;
  int init;
  int **Dtrans;  
  int *accept; 
  int *marque;
};

typedef struct _automaton* automaton;

automaton automaton_build (int init_state, int *ptl, int *PS);
void      automaton_delete(automaton a);
void      automaton_dump  (automaton a, char* filename);

#endif
