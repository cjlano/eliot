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
 * $Id: automaton.c,v 1.6 2005/04/19 20:18:32 afrab Exp $
 */
#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef HAVE_SYS_WAIT_H
#   include <sys/wait.h>
#endif
#include <unistd.h>
#include "dic.h"
#include "regexp.h"
#include "automaton.h"

#ifndef PDBG
#ifdef DEBUG_RE2
#   define PDBG(s) s
#else
#   define PDBG(s) 
#endif
#endif

automaton
automaton_build(int init_state, int *ptl, int *PS)
{
  /* int init_state; == root->PP           */
  /* int *ptl; mapping postition -> lettre */
  /* int *PS;  Position Suivante [ 1 << (position-1)] = \cup { 1 << (p-1) | p \in position acceptée } */

  int  i,l,pos,letter,ens;
  int  state,plist;
  int  *state_list;
  char used_letter[256];
  automaton a;

  a = (automaton)malloc(sizeof(struct _automaton));
  
  a->nterm  = PS[0];
  a->nstate = 1;
  a->init   = init_state;
  a->accept = (int*) calloc(1 << PS[0], sizeof(int));   // #{states}
  a->marque = (int*) calloc(1 << PS[0], sizeof(int));   // #{states}
  a->Dtrans = (int**)calloc(1 << PS[0], sizeof(int*));  // #{states} * #{letters}
  a->callocsize = 1 << PS[0];

  for(i=0; i < (1 << PS[0]); i++)
    {
      // 256 different letters max
      a->Dtrans[i] = (int*)calloc(256, sizeof(int));    
    }

  state_list = (int*)calloc(1 << PS[0], sizeof(int));   // #{states} 

  /* 1: init_state = root->PP */
  plist = 0;
  state_list[plist++] = a->init;
  /* 2: while \exist state \in state_list */
  while (plist)
    {
      state = state_list[--plist];
      PDBG(fprintf(stdout,"** traitement état 0x%08x\n",state));
      memset(used_letter,0,sizeof(used_letter));
      /* 3: \foreach l in \sigma | l \neq # */
      for(l=1; l < PS[0]; l++) 
	{
	  letter = ptl[l];
	  if (used_letter[letter] == 0)
	    {
	      /* 4: int ens = \cup { PS(pos) | pos \in state \wedge pos == l } */
	      ens = 0;
	      for(pos = 1; pos <= PS[0]; pos++)
		{
		  if (ptl[pos] == letter && (state & (1 << (pos - 1))))
		    ens |= PS[pos];
		}
	      /* 5: */
	      if (ens && a->marque[ens] == 0)
		{
		  state_list[plist++] = ens;
		  a->Dtrans[state][letter] = ens;
		  PDBG(fprintf(stdout,"  adding %x +",state));
		  PDBG(regexp_print_letter(stdout,letter));
		  PDBG(fprintf(stdout,"> %x (queue %x)\n",ens,ens));
		  if (ens != state)
		    {
		      a->nstate = a->nstate + 1;
		    }
		}
	      /* 6: */
	      if (ens && a->marque[ens] == 1)
		{
		  a->Dtrans[state][letter] = ens;
		  PDBG(fprintf(stdout,"  adding %x -",state));
		  PDBG(regexp_print_letter(stdout,letter));
		  PDBG(fprintf(stdout,"> %x\n",ens));
		}
	      a->marque[state] = 1;
	      used_letter[letter] = 1;
	    }
	}
    }

  PDBG(fprintf(stdout,"** accept : "));
  for(i=0; i < (1 << PS[0]); i++)
    {
      if (a->marque[i] && (i & (1 << (PS[0] - 1))))
	{
	  a->accept[i] = 1;
	  PDBG(fprintf(stdout,"%x ",i));
	}
    }
  PDBG(fprintf(stdout,"\n"));

  free(state_list);
  return a;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////

void 
automaton_delete(automaton a)
{
  int i;
  free(a->accept);
  free(a->marque);
  for(i=0; i < a->callocsize; i++)
    {
      free(a->Dtrans[i]);
    }
  free(a->Dtrans);
  free(a);
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////

#if 0
void 
automaton_minimize(automaton a, automaton b)
{
  int i;
  b->nterm  = a->nterm;
  b->nstate = a->nstate;
  b->init   = a->init;
  b->accept = (int*) calloc(a->nstate, sizeof(int ));  // #{etats}
  b->marque = (int*) calloc(a->nstate, sizeof(int ));  // #{etats}
  b->Dtrans = (int**)calloc(a->nstate, sizeof(int*));  // #{etats} * #{lettres}
  for(i=0; i < a->nstate; i++)
    {
      b->Dtrans[i] = (int*)calloc(256, sizeof(int));
    }
  
  /* NOT DONE */
}
#endif 

//////////////////////////////////////////////////
//////////////////////////////////////////////////

#ifdef DEBUG_RE
static void 
print_automaton_nodes(FILE* f, automaton a)
{
  int state;
  for(state=0; state < (1 << a->nterm); state++)
    {
      if (a->marque[state])
	{
	  fprintf(f,"\t%d [label = \"%x\"",state,state);
	  if (a->init == state)
	    {
	      fprintf(f,", style = filled, color=lightgrey");
	    }
	  if (a->accept[state])
 	    {
	      fprintf(f,", shape = doublecircle");
	    }
	  fprintf(f,"];\n");
	}
    }
  fprintf(f,"\n");
}
#endif

#ifdef DEBUG_RE
static void 
print_automaton_edges(FILE* f, automaton a)
{
  int state,letter;
  for(state=0; state < (1 << a->nterm); state++)
    {
      if (a->marque[state]) 
	{
	  for(letter=0; letter < 255; letter++)
	    {
	      if (a->Dtrans[state][letter])
		{
		  fprintf(f,"\t%d -> %d [label = \"",state,a->Dtrans[state][letter]);
		  regexp_print_letter(f,letter);
		  fprintf(f,"\"];\n");
		}
	    }
	}
    }
}
#endif

#ifdef DEBUG_RE
void 
automaton_dump(automaton a, char* filename)
{
  FILE* f;
  pid_t   pid;

  f=fopen(filename,"w");
  fprintf(f,"digraph automaton {\n");
  print_automaton_nodes(f,a);
  print_automaton_edges(f,a);
  fprintf(f,"fontsize=20;\n");
  fprintf(f,"}\n");
  fclose(f);

#ifdef HAVE_SYS_WAIT_H
  pid = fork ();
  if (pid > 0) {
    wait(NULL);
  } else if (pid == 0) {
    execlp("dotty","dotty",filename,NULL);
    printf("exec dotty failed\n");
    exit(1);
  }
#endif
}
#endif

//////////////////////////////////////////////////
//////////////////////////////////////////////////

