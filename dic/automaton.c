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
 * $Id: automaton.c,v 1.1 2004/06/20 20:13:59 afrab Exp $
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "automaton.h"

#ifndef PDBG
#ifdef DEBUG
#define PDBG(x) { x ; }
#else
#define PDBG(x) { }
#endif
#endif

int 
automaton_build(automaton* aa, int init_state, int *ptl, int *PS)
{
  int  i,l,pos,letter,ens;
  int  state,plist;
  int  *state_list;
  char used_letter[256];
  automaton a;

  a = (automaton)malloc(sizeof(struct _automaton));
  *aa = a;
  
  a->nterm  = PS[0];
  a->nstate = 1;
  a->init   = init_state;
  a->accept = (int*) calloc(1 << PS[0], sizeof(int));   // #{etats}
  a->marque = (int*) calloc(1 << PS[0], sizeof(int));   // #{etats}
  a->Dtrans = (int**)calloc(1 << PS[0], sizeof(int*));  // #{etats} * #{lettres}
  for(i=0; i < (1 << PS[0]); i++)
    {
      a->Dtrans[i] = (int*)calloc(256, sizeof(int));
    }
  
  state_list = (int*)calloc(1 << PS[0], sizeof(int));   // #{etats} 

  /* 1: init_state = root->PP */
  plist = 0;
  state_list[plist++] = a->init;
  /* 2: while \exist state \in state_list */
  while (plist)
    {
      state = state_list[--plist];
      PDBG(printf("** traitement état 0x%08x\n",state));
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
		  PDBG(printf("  adding %x -%c> %x (queue %x)\n",state,letter,ens,ens));
		  if (ens != state)
		    {
		      a->nstate = a->nstate + 1;
		    }
		}
	      /* 6: */
	      if (ens && a->marque[ens] == 1)
		{
		  a->Dtrans[state][letter] = ens;
		  PDBG(printf("  adding %x -%c> %x\n",state,letter,ens));
		}
	      a->marque[state] = 1;
	      used_letter[letter] = 1;
	    }
	}
    }

  PDBG(printf("** accept : "));
  for(i=0; i < (1 << PS[0]); i++)
    {
      if (a->marque[i] && (i & (1 << (PS[0] - 1))))
	{
	  a->accept[i] = 1;
	  PDBG(printf("%x ",i));
	}
    }
  PDBG(printf("\n"));

  free(state_list);
  return 0;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////

void 
automaton_delete(automaton a)
{
  int i;
  free(a->accept);
  free(a->marque);
  for(i=0; i < a->nstate; i++)
    {
      free(a->Dtrans[i]);
    }
  free(a->Dtrans);
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////

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

//////////////////////////////////////////////////
//////////////////////////////////////////////////

static void 
print_automaton_nodes(FILE* f, automaton a)
{
  int i;
  for(i=0; i < (1 << a->nterm); i++)
    {
      if (a->marque[i])
	{
	  fprintf(f,"\t%d [label = \"%x\"",i,i);
	  if (a->init == i)
	    {
	      fprintf(f,", style = filled, color=lightgrey");
	    }
	  if (a->accept[i])
 	    {
	      fprintf(f,", shape = doublecircle");
	    }
	  fprintf(f,"];\n");
	}
    }
  fprintf(f,"\n");
}

static void 
print_automaton_edges(FILE* f, automaton a)
{
  int i,j;
  for(i=0; i < (1 << a->nterm); i++)
    {
      if (a->marque[i])
	{
	  for(j=0; j < 255; j++)
	    {
	      if (a->Dtrans[i][j])
		{
		  fprintf(f,"\t%d -> %d [label = \"%c\"];\n",i,a->Dtrans[i][j],j);
		}
	    }
	}
    }
}

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

  pid = fork ();
  if (pid > 0) {
    wait(NULL);
  } else if (pid == 0) {
    execlp("dotty","dotty",filename,NULL);
    printf("exec dotty failed\n");
    exit(1);
  }
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////

