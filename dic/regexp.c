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
 * $Id: regexp.c,v 1.1 2004/06/21 16:06:54 afrab Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "regexp.h"
#include "automaton.h"

#define MAX 32

#ifndef PDBG
#ifdef DEBUG
#define PDBG(x) { x ; }
#else
#define PDBG(x) { }
#endif
#endif

//////////////////////////////////////////////////
// position, annulable, PP, DP
// r   = root
// p   = current leaf position
// n   = current node number
// ptl = position to letter
//////////////////////////////////////////////////

void parcours(NODE* r, int *p, int *n, int ptl[])
{
  if (r == NULL)
    return;

  parcours(r->fg,p,n,ptl);
  parcours(r->fd,p,n,ptl);

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

//////////////////////////////////////////////////
// PosSuivante
//////////////////////////////////////////////////

void possuivante(NODE* r, int PS[])
{
  int pos;
  if (r == NULL)
    return;

  possuivante(r->fg,PS);
  possuivante(r->fd,PS);

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


//////////////////////////////////////////////////
//////////////////////////////////////////////////
#if 0
void print_node(NODE *n)
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

//////////////////////////////////////////////////
//////////////////////////////////////////////////

void print_PS(int PS[])
{
  int i;
  printf("** positions suivantes **\n");
  for(i=1; i <= PS[0]; i++)
    {
      printf("%02d: 0x%08x\n", i, PS[i]);
    }
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////

void print_ptl(int ptl[])
{
  int i;
  printf("** pos -> lettre: ");
  for(i=1; i <= ptl[0]; i++)
    {
      printf("%d=%c ",i,ptl[i]);
    }
  printf("\n");
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////

void print_tree_nodes(FILE* f, NODE* n)
{
  if (n == NULL) 
    return; 

  print_tree_nodes(f,n->fg);
  print_tree_nodes(f,n->fd);
  
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
  fprintf(f,"\\n annulable=%d\\n PP=0x%08x\\n DP=0x%08x\"];\n",n->annulable,n->PP,n->DP);
}

void print_tree_edges(FILE* f, NODE* n)
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

void print_tree(NODE* n)
{
  FILE* f;
  pid_t   pid; 
  char name[] = "tree";
  
  f=fopen(name,"w");
  fprintf(f,"digraph %s {\n",name);
  print_tree_nodes(f,n);
  print_tree_edges(f,n);
  fprintf(f,"fontsize=20;\n");
  fprintf(f,"}\n");
  fclose(f);

  pid = fork ();
  if (pid > 0) {
    wait(NULL);
  } else if (pid == 0) {
    execlp("dotty","dotty",name,NULL);
    printf("exec dotty failed\n");
    exit(1);
  }
}
#endif
//////////////////////////////////////////////////
//////////////////////////////////////////////////
#if 0
int main(int argc, char* argv[])
{
  int i,p,n;
  int ptl[MAX+1]; // mapping postition -> lettre
  int PS [MAX+1]; // Position Suivante [ 1 << (position-1)] = \cup { 1 << (p-1) | p \in position acceptée }
  automaton a,b;

  for(i=0; i < MAX; i++)
    {
      PS[i] = 0;
      ptl[i] = 0;
    }

  yyparse();

  n = 1;
  p = 1;
  parcours(root,&p,&n,ptl);
  PS [0] = p - 1;
  ptl[0] = p - 1;
  PDBG(printf("** regexp: nombre de terminaux: %d\n",PS[0]));
  PDBG(printf("** regexp: nombre de noeuds dans l'arbre: %d\n",n));
  PDBG(print_ptl(ptl));

  possuivante(root,PS);
  PDBG(print_tree(root));
  PDBG(print_PS(PS));

  automaton_build(&a,root->PP,ptl,PS);
  PDBG(printf("** auto: nombre d'états: %d\n",a.nstate));
  print_automaton(&a);
  
  automaton_minimize(&a,&b);
  PDBG(printf("** auto: nombre d'états: %d\n",b.nstate));
  PDBG(print_automaton(&b));

  automaton_destroy(&a);
  automaton_destroy(&b);
  return 0;
}
#endif
//////////////////////////////////////////////////
//////////////////////////////////////////////////

