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
 * $Id: regexpmain.c,v 1.1 2005/04/09 19:16:09 afrab Exp $
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "regexp.h"
#include "automaton.h"

#ifndef PDBG
#ifdef DEBUG
#define PDBG(x) { x ; }
#else
#define PDBG(x) { }
#endif
#endif

NODE* root;

int main(int argc, char* argv[])
{
  int i,p,n;
  int ptl[REGEXP_MAX+1]; // mapping postition -> lettre
  int PS [REGEXP_MAX+1]; // Position Suivante [ 1 << (position-1)] = \cup { 1 << (p-1) | p \in position acceptée }
  automaton a;

  for(i=0; i < REGEXP_MAX; i++)
    {
      PS[i] = 0;
      ptl[i] = 0;
    }

  yyparse(argv[1]);

  n = 1;
  p = 1;
  regexp_parcours(root,&p,&n,ptl);
  PS [0] = p - 1;
  ptl[0] = p - 1;
  PDBG(printf("** regexp: nombre de terminaux: %d\n",PS[0]));
  PDBG(printf("** regexp: nombre de noeuds dans l'arbre: %d\n",n));
  PDBG(regexp_print_ptl(ptl));

  regexp_possuivante(root,PS);
  PDBG(regexp_print_tree(root));
  PDBG(regexp_print_PS(PS));

  a = automaton_build(root->PP,ptl,PS);
  PDBG(printf("** auto: nombre d'états: %d\n",a->nstate));
  automaton_dump(a,"regexp.auto");
  
  automaton_delete(a);
  return 0;
}
