/* Eliot                                                                     */
/* Copyright (C) 1999  Antoine Fraboulet                                     */
/*                                                                           */
/* This file is part of Eliot.                                               */
/*                                                                           */
/* Eliot is free software; you can redistribute it and/or modify             */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* Eliot is distributed in the hope that it will be useful,                  */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

/**
 *  \file   regexp.h
 *  \brief  Regular Expression functions
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
#define NODE_PLUS   5

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

    /**
     * maximum number of accepted terminals in regular expressions
     */
#define REGEXP_MAX 32

    /**
     * special terminals that should not appear in the dictionary
     */
#define RE_EPSILON     (DIC_LETTERS + 0)
#define RE_FINAL_TOK   (DIC_LETTERS + 1)
#define RE_ALL_MATCH   (DIC_LETTERS + 2)
#define RE_VOWL_MATCH  (DIC_LETTERS + 3)
#define RE_CONS_MATCH  (DIC_LETTERS + 4)
#define RE_USR1_MATCH  (DIC_LETTERS + 5)
#define RE_USR2_MATCH  (DIC_LETTERS + 6)

    /**
     * number of lists for regexp letter match \n
     * 0 : all tiles                           \n
     * 1 : vowels                              \n
     * 2 : consonants                          \n
     * 3 : user defined 1                      \n
     * 4 : user defined 2                      \n
     * x : lists used during parsing           \n
     */
#define DIC_SEARCH_REGE_LIST (REGEXP_MAX)

    /**
     * Structure used for Dic_search_RegE \n
     * this structure is used to explicit letters list that will be matched
     * against special tokens in the regular expression search
     */
struct search_RegE_list_t {
  /** maximum length for results */
  int minlength;
  /** maximum length for results */
  int maxlength;
  /** special symbol associated with the list */
  char symbl[DIC_SEARCH_REGE_LIST];
  /** 0 or 1 if list is valid */
  int  valid[DIC_SEARCH_REGE_LIST];
  /** 0 or 1 if letter is present in the list */
  char letters[DIC_SEARCH_REGE_LIST][DIC_LETTERS];
};

#define RE_LIST_ALL_MATCH  0
#define RE_LIST_VOYL_MATCH 1
#define RE_LIST_CONS_MATCH 2
#define RE_LIST_USER_BEGIN 3
#define RE_LIST_USER_END   4

    /**
     * Create a node for the syntactic tree used for
     * parsing regular expressions                    \n
     * The fonction is called by bison grammar rules
     */
NODE* regexp_createNODE(int type,char v,NODE *fg,NODE *fd);

    /**
     * delete regexp syntactic tree
     */
void  regexp_delete_tree(NODE * root);

    /**
     * Computes positions, first positions (PP), last position (DP)
     * and translation table 'position to letter' (ptl)
     * @param p : max position found in the tree (must be initialized to 1)
     * @param n : number of nodes in the tree (must be initialized to 1)
     * @param ptl : position to letter translation table
     */
void  regexp_parcours(NODE* r, int *p, int *n, int ptl[]);

    /**
     * Computes 'next position' table used for building the
     * automaton
     * @param r : root node of the syntactic tree
     * @param PS : next position table, PS[0] must contain the
     * number of terminals contained in the regular expression
     */
void  regexp_possuivante(NODE* r, int PS[]);

#define MAX_REGEXP_ERROR_LENGTH 500

struct regexp_error_report_t {
  int pos1;
  int pos2;
  char msg[MAX_REGEXP_ERROR_LENGTH];
};

#include <stdio.h>

void  regexp_print_letter(FILE* f, char l);
void  regexp_print_letter2(FILE* f, char l);
void  regexp_print_PS(int PS[]);
void  regexp_print_ptl(int ptl[]);
void  regexp_print_tree(NODE* n, char* name, int detail);

#if defined(__cplusplus)
  }
#endif
#endif /* _TREE_H_ */

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
