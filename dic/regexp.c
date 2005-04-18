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
 * $Id: regexp.c,v 1.4 2005/04/18 17:40:36 afrab Exp $
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_WAIT_H
#   include <sys/wait.h>
#endif
#include <unistd.h>

#include "regexp.h"
#include "automaton.h"


#ifndef PDBG
#ifdef DEBUG
#define PDBG(x) { x ; }
#else
#define PDBG(x) { }
#endif
#endif

NODE* regexp_createNODE(int type,char v,NODE *fg,NODE *fd)
{
  NODE *x;
  x=(NODE *)malloc(sizeof(NODE));
  x->type      = type;
  x->var       = v;
  x->fd        = fd;
  x->fg        = fg;
  x->numero    = 0;
  x->position  = 0;
  x->annulable = 0;
  x->PP        = 0;
  x->DP        = 0;
  return x;
}

NODE *regexp_createNODE_AllMatch()
{
  return regexp_createNODE(NODE_VAR,'A',NULL,NULL);
}

NODE *regexp_createNODE_ConsMatch()
{
  return regexp_createNODE(NODE_VAR,'C',NULL,NULL);
}

NODE *regexp_createNODE_VoylMatch()
{
  return regexp_createNODE(NODE_VAR,'V',NULL,NULL);
}

void regexp_delete_tree(NODE *root)
{
  if (root == NULL)
    return;
  regexp_delete_tree(root->fg);
  regexp_delete_tree(root->fd);
  free(root);
}

static void print_node(NODE *n);

/*////////////////////////////////////////////////
// position, annulable, PP, DP
// r   = root
// p   = current leaf position
// n   = current node number
// ptl = position to letter
////////////////////////////////////////////////*/

void regexp_parcours(NODE* r, int *p, int *n, int ptl[])
{
  if (r == NULL)
    return;

  regexp_parcours(r->fg,p,n,ptl);
  regexp_parcours(r->fd,p,n,ptl);

  switch (r->type)
    {
    case NODE_VAR:
      r->position = *p;
      ptl[*p] = r->var;
      *p = *p + 1;
      r->annulable = 0;
      r->PP = 1 << (r->position - 1);
      r->DP = 1 << (r->position - 1);
      break;
    case NODE_OR:
      r->position = 0;
      r->annulable = r->fg->annulable || r->fd->annulable;
      r->PP = r->fg->PP | r->fd->PP;
      r->DP = r->fg->DP | r->fd->DP;
      break;
    case NODE_AND:
      r->position = 0;
      r->annulable = r->fg->annulable && r->fd->annulable;
      r->PP = (r->fg->annulable) ? (r->fg->PP | r->fd->PP) : r->fg->PP;
      r->DP = (r->fd->annulable) ? (r->fg->DP | r->fd->DP) : r->fd->DP;
      break;
    case NODE_STAR:
      r->position = 0;
      r->annulable = 1;
      r->PP = r->fg->PP;
      r->DP = r->fg->DP;
      break;
    }

  r->numero = *n;
  *n = *n + 1;
  
  PDBG(print_node(r));
}

/*////////////////////////////////////////////////
// PosSuivante
////////////////////////////////////////////////*/

void regexp_possuivante(NODE* r, int PS[])
{
  int pos;
  if (r == NULL)
    return;

  regexp_possuivante(r->fg,PS);
  regexp_possuivante(r->fd,PS);

  switch (r->type)
    {
    case NODE_AND:
      for(pos=1; pos <= PS[0]; pos++)
	{
	  if (r->fg->DP & (1 << (pos-1)))
	    PS[pos] |= r->fd->PP;
	}
      break;
    case NODE_STAR:
      for(pos=1; pos <= PS[0]; pos++)
	{
	  if (r->DP & (1 << (pos-1)))
	    PS[pos] |= r->PP;
	}
      break;
    }
}


/*////////////////////////////////////////////////
////////////////////////////////////////////////*/
static void print_node(NODE *n)
{
  if (n == NULL)
    return;
  
  switch (n->type)
    {
    case NODE_VAR:
      printf("%c (%d)",n->var,n->position);
      break;
    case NODE_OR:
      printf("OR");
      break;
    case NODE_AND:
      printf("AND");
      break;
    case NODE_STAR:
      printf("STAR");
      break;
    }
  printf("\tannulable: %d",n->annulable);
  printf(" PP: 0x%08x",n->PP);
  printf(" DP: 0x%08x\n",n->DP);
}

/*////////////////////////////////////////////////
////////////////////////////////////////////////*/

void regexp_print_PS(int PS[])
{
  int i;
  printf("** positions suivantes **\n");
  for(i=1; i <= PS[0]; i++)
    {
      printf("%02d: 0x%08x\n", i, PS[i]);
    }
}

/*////////////////////////////////////////////////
////////////////////////////////////////////////*/

void regexp_print_ptl(int ptl[])
{
  int i;
  printf("** pos -> lettre: ");
  for(i=1; i <= ptl[0]; i++)
    {
      printf("%d=%c ",i,ptl[i]);
    }
  printf("\n");
}

/*////////////////////////////////////////////////
////////////////////////////////////////////////*/

static void print_tree_nodes(FILE* f, NODE* n, int detail)
{
  if (n == NULL) 
    return; 

  print_tree_nodes(f,n->fg,detail);
  print_tree_nodes(f,n->fd,detail);
  
  fprintf(f,"%d [ label=\"",n->numero);
  switch (n->type)
    {
    case NODE_VAR:
      fprintf(f,"%c (%d)",n->var,n->position);
      break;
    case NODE_OR:
      fprintf(f,"OR");
      break;
    case NODE_AND:
      fprintf(f,"AND");
      break;
    case NODE_STAR:
      fprintf(f,"*");
      break;
    }
  if (detail)
    {
      fprintf(f,"\\n annulable=%d\\n PP=0x%08x\\n DP=0x%08x",
	      n->annulable,n->PP,n->DP);
    }
  fprintf(f,"\"];\n");
}

static void print_tree_edges(FILE* f, NODE* n)
{
  if (n == NULL)
    return;

  print_tree_edges(f,n->fg);
  print_tree_edges(f,n->fd);
  
  switch (n->type)
    {
    case NODE_OR:
      fprintf(f,"%d -> %d;",n->numero,n->fg->numero);
      fprintf(f,"%d -> %d;",n->numero,n->fd->numero);
      break;
    case NODE_AND:
      fprintf(f,"%d -> %d;",n->numero,n->fg->numero);
      fprintf(f,"%d -> %d;",n->numero,n->fd->numero);
      break;
    case NODE_STAR:
      fprintf(f,"%d -> %d;",n->numero,n->fg->numero);
      break;
    }
}

void regexp_print_tree(NODE* n, char* name, int detail)
{
  FILE* f;
  pid_t   pid; 
  
  f=fopen(name,"w");
  fprintf(f,"digraph %s {\n",name);
  print_tree_nodes(f,n,detail);
  print_tree_edges(f,n);
  fprintf(f,"fontsize=20;\n");
  fprintf(f,"}\n");
  fclose(f);

#ifdef HAVE_SYS_WAIT_H
  pid = fork ();
  if (pid > 0) {
    wait(NULL);
  } else if (pid == 0) {
    execlp("dotty","dotty",name,NULL);
    printf("exec dotty failed\n");
    exit(1);
  }
#endif
}

