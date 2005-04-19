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

/* $Id: regexp.h,v 1.5 2005/04/19 16:26:51 afrab Exp $ */

/**
 *  \file regexp.h
 *  \brief  Regular Expression fonctions
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#ifndef _TREE_H_
#define _TREE_H_
#if defined(__cplusplus)
extern "C" 
  {
#endif 

#define NODE_TOP    0
#define NODE_VAR    1
#define NODE_OR     2
#define NODE_AND    3
#define NODE_STAR   4

typedef struct node {
  int              type;
  char             var; 
  struct node      *fg; 
  struct node      *fd; 
  int numero;
  int position;
  int annulable;
  int PP;
  int DP;
} NODE;

/* max regexp length */
#define REGEXP_MAX 32 

/** special letters that should not appear in the dictionary */
#define RE_ALL_MATCH   (DIC_LETTERS + 1)
#define RE_VOWL_MATCH  (DIC_LETTERS + 2)
#define RE_CONS_MATCH  (DIC_LETTERS + 3)
#define RE_USR1_MATCH  (DIC_LETTERS + 4)
#define RE_USR2_MATCH  (DIC_LETTERS + 5)

NODE* regexp_createNODE(int type,char v,NODE *fg,NODE *fd);
void  regexp_delete_tree(NODE * root);

void  regexp_parcours(NODE* r, int *p, int *n, int ptl[]);
void  regexp_possuivante(NODE* r, int PS[]);

#ifdef DEBUG_RE
#include <stdio.h>

void  regexp_print_letter(FILE* f, char l);
void  regexp_print_PS(int PS[]);
void  regexp_print_ptl(int ptl[]);
void  regexp_print_tree(NODE* n, char* name, int detail);
#endif

#if defined(__cplusplus)
  }
#endif 
#endif /* _TREE_H_ */
