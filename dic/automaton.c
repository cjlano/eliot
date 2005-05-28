/* Eliot                                                                     */
/* Copyright (C) 2005  Antoine Fraboulet                                     */
/*                                                                           */
/* This file is part of Eliot.                                               */
/*                                                                           */
/* Eliot is free software; you can redistribute it and/or modify             */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* Elit is distributed in the hope that it will be useful,                   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/*
 * $Id: automaton.c,v 1.8 2005/05/28 20:59:14 afrab Exp $
 */

#include "config.h"
#include <assert.h>
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
#include "alist.h"
#include "automaton.h"

#ifdef DEBUG_AUTOMATON__
#define DMSG(a) a
#else
#define DMSG(a)
#endif

#define MAX_TRANSITION_LETTERS 256

typedef struct automaton_state_t *astate;
typedef struct Automaton_t       *Automaton;

/* ************************************************** *
   exported functions for static automata
 * ************************************************** */
 
automaton automaton_build          (int init_state, int *ptl, int *PS, struct search_RegE_list_t *list);
void      automaton_delete         (automaton a);
int       automaton_get_nstate     (automaton a);
int       automaton_get_init       (automaton a);
int       automaton_get_accept     (automaton a, int state);
int       automaton_get_next_state (automaton a, int start, char l);
void      automaton_dump           (automaton a, char* filename);
     

/* ************************************************** *
   static functions for dynamic automata
 * ************************************************** */

static Automaton s_automaton_create         ();
static void      s_automaton_delete         (Automaton a);

static alist     s_automaton_id_create      (int id);
static char*     s_automaton_id_to_str      (alist id);

static astate    s_automaton_state_create   (alist id);

static void      s_automaton_add_state      (Automaton a, astate s);
static astate    s_automaton_get_state      (Automaton a, alist id);

static Automaton s_automaton_PS_to_NFA      (int init_state, int *ptl, int *PS);
static Automaton s_automaton_NFA_to_DFA     (Automaton a, struct search_RegE_list_t *list);
static automaton s_automaton_finalize       (Automaton a);
#ifdef DEBUG_AUTOMATON
static void      s_automaton_dump           (Automaton a, char* filename);
#endif

/* ************************************************** *
   data types
 * ************************************************** */

struct automaton_state_t {
  alist    id;                     // alist of int
  int      accept;
  int      id_static;
  astate   next[MAX_TRANSITION_LETTERS];
};

struct Automaton_t {
  int      nstates;
  astate   init_state;
  alist    states;                 // alist of alist of int
};

struct automaton_t {
  int   nstates;
  int   init;
  int  *accept;
  int **trans;
};

/* ************************************************** *
   exported functions for static automata
 * ************************************************** */

automaton 
automaton_build(int init_state, int *ptl, int *PS, struct search_RegE_list_t *list)
{
  Automaton nfa,dfa;
  automaton final;

  nfa = s_automaton_PS_to_NFA(init_state,ptl,PS);
  DMSG(printf("\n non deterministic automaton OK \n\n"));
  DMSG(s_automaton_dump(nfa,"auto_nfa"));

  dfa = s_automaton_NFA_to_DFA(nfa, list);
  DMSG(printf("\n deterministic automaton OK \n\n"));
  DMSG(s_automaton_dump(dfa,"auto_dfa"));

  final = s_automaton_finalize(dfa);
  DMSG(printf("\n final automaton OK \n\n"));
  DMSG(automaton_dump(final,"auto_fin"));

  s_automaton_delete(nfa);
  s_automaton_delete(dfa);
  return final;
}

void 
automaton_delete(automaton a)
{
  int i;
  free(a->accept);
  for(i=0; i <= a->nstates; i++)
    free(a->trans[i]);
  free(a->trans);
  free(a);
}

inline int
automaton_get_nstates(automaton a)
{
  return a->nstates;
}

inline int
automaton_get_init(automaton a)
{
  return a->init;
}

inline int
automaton_get_accept(automaton a, int state)
{
  return a->accept[state];
}

inline int
automaton_get_next_state(automaton a, int state, char l)
{
  return a->trans[state][(int)l];
}

void 
automaton_dump(automaton a, char* filename)
{
  int i,l;
  FILE* f;
  pid_t   pid;
  if (a == NULL)
    return ;
  f=fopen(filename,"w");
  fprintf(f,"digraph automaton {\n");
  for(i=1; i<=a->nstates; i++)
    {
      fprintf(f,"\t%d [label = \"%d\"",i,i);
      if (i == a->init)
	fprintf(f,", style = filled, color=lightgrey");
      if (a->accept[i])
	fprintf(f,", shape = doublecircle");
      fprintf(f,"];\n");
    }
  fprintf(f,"\n");
  for(i=1; i<=a->nstates; i++)
    for(l=0; l < MAX_TRANSITION_LETTERS; l++)
      if (a->trans[i][l])
	{
	  fprintf(f,"\t%d -> %d [label = \"",i,a->trans[i][l]);
	  regexp_print_letter(f,l);
	  fprintf(f,"\"];\n");
	}
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

/* ************************************************** *
 * ************************************************** *
 * ************************************************** */

void
state_delete_fun(void* ps)
{
  astate s = ps;
  alist_delete(s->id);
  free(s);
}

static Automaton
s_automaton_create()
{  
  Automaton a; 
  a = (Automaton)malloc(sizeof(struct Automaton_t));
  a->nstates      = 0;
  a->init_state   = NULL;
  a->states       = alist_create();
  alist_set_delete(a->states,state_delete_fun);
  return a;
}


static void
s_automaton_delete(Automaton a)
{
  alist_delete(a->states);
  free(a);
}

static alist
s_automaton_id_create(int id)
{
  alist a = alist_create();
  alist_add(a,(void*)id);
  return a;
}

static char* s_automaton_id_to_str(alist id)
{
  static char s[250];
  memset(s,0,sizeof(s));
  alist_elt ptr;
  for(ptr = alist_get_first(id); ptr ; ptr = alist_get_next(id,ptr))
    {
      char tmp[50];
      sprintf(tmp,"%d ",(int)alist_elt_get_value(ptr));
      strcat(s,tmp);
    }
  return s;
}

static astate
s_automaton_state_create(alist id)
{
  astate s;
  s = (astate)malloc(sizeof(struct automaton_state_t));
  s->id      = id;
  s->accept  = 0;
  memset(s->next,0,sizeof(astate)*MAX_TRANSITION_LETTERS);
  DMSG(printf("** state %s creation\n",s_automaton_id_to_str(id)));
  return s;
}

static void 
s_automaton_add_state(Automaton a, astate s)
{ 
  a->nstates ++;
  alist_add(a->states,(void*)s);
  DMSG(printf("** state %s added to automaton\n",s_automaton_id_to_str(s->id)));
}

static astate
s_automaton_get_state(Automaton a, alist id)
{
  astate s;
  alist_elt ptr;
  for(ptr = alist_get_first(a->states) ;  ptr ; ptr = alist_get_next(a->states,ptr))
    {
      s = alist_elt_get_value(ptr);
      if (alist_equal(s->id,id))
	{
	  //DMSG(printf("** get state %s ok\n",s_automaton_id_to_str(s->id)));
	  return s;
	}
    } 
  return NULL;
}

/* ************************************************** *
 * ************************************************** *
 * ************************************************** */

Automaton
s_automaton_PS_to_NFA(int init_state_id, int *ptl, int *PS)
{
  int p;
  int maxpos = PS[0];
  Automaton nfa = NULL;
  alist temp_id;
  alist_elt ptr;
  astate temp_state,current_state;
  alist L;
  char used_letter[MAX_TRANSITION_LETTERS];

  nfa = s_automaton_create();
  L   = alist_create();

  /* 1: init_state = root->PP */
  temp_id          = s_automaton_id_create(init_state_id);
  temp_state       = s_automaton_state_create(temp_id);
  nfa->init_state  = temp_state;
  s_automaton_add_state(nfa,temp_state);
  alist_add(L,temp_state);
  /* 2: while \exist state \in state_list */
  while (! alist_is_empty(L))
    {
      current_state = (astate)alist_pop_first_value(L);
      DMSG(printf("** current state = %s\n",s_automaton_id_to_str(current_state->id)));
      memset(used_letter,0,sizeof(used_letter));
      /* 3: \foreach l in \sigma | l \neq # */
      for(p=1; p < maxpos; p++) 
	{
	  int current_letter = ptl[p];
	  if (used_letter[current_letter] == 0)
	    {
	      /* 4: int set = \cup { PS(pos) | pos \in state \wedge pos == l } */
	      int pos, ens = 0;
	      for(pos = 1; pos <= maxpos; pos++)
		{
		  if (ptl[pos] == current_letter && 
		      (int)alist_elt_get_value(alist_get_first(current_state->id)) & (1 << (pos - 1)))
		    ens |= PS[pos];
		}
	      /* 5: transition from current_state to temp_state */
	      if (ens)
		{
		  temp_id    = s_automaton_id_create(ens);
		  temp_state = s_automaton_get_state(nfa,temp_id);
		  if (temp_state == NULL)
		    {
		      temp_state = s_automaton_state_create(temp_id);
		      s_automaton_add_state     (nfa,temp_state);
		      current_state->next[current_letter] = temp_state;
		      alist_add(L,temp_state);
		    }
		  else 
		    {
		      alist_delete(temp_id);
		      current_state->next[current_letter] = temp_state;
		    }
		}
	      used_letter[current_letter] = 1;
	    }
	}
    }

  alist_delete(L);

  for(ptr = alist_get_first(nfa->states); ptr ; ptr = alist_get_next(nfa->states,ptr))
    {
      astate s = (astate)alist_elt_get_value(ptr);
      if ((int)alist_elt_get_value(alist_get_first(s->id)) & (1 << (maxpos - 1)))
	s->accept = 1;
    }

  return nfa;
}

/* ************************************************** *
 * ************************************************** *
 * ************************************************** */

static alist
s_automaton_successor(alist S, int letter, Automaton nfa, struct search_RegE_list_t *list)
{
  alist R,r;
  alist_elt ptr;
  R = alist_create();                                       /* R = \empty */
                                                            /* \forall y \in S */
  for(ptr = alist_get_first(S); ptr ; ptr = alist_get_next(S,ptr))
    {
      int i;
      alist t, Ry; astate y,z;

      i = (int)alist_elt_get_value(ptr);
      t = s_automaton_id_create(i);
      assert(y = s_automaton_get_state(nfa,t));
      alist_delete(t);

      Ry = alist_create();                                 /* Ry = \empty             */

      if ((z = y->next[letter]) != NULL)                   /* \delta (y,z) = l        */
	{
	  r = s_automaton_successor(z->id,RE_EPSILON,nfa, list);    
	  alist_insert(Ry,r);
	  alist_delete(r);
	  alist_insert(Ry,z->id);                          /* Ry = Ry \cup succ(z)    */
	}

#if 0
      if ((z = y->next[RE_EPSILON]) != NULL)               /* \delta (y,z) = \epsilon */
	{
	  r = s_automaton_successor(z->id,letter,nfa, list);    
	  alist_insert(Ry,r);                              /* Ry = Ry \cup succ(z)    */
	  alist_delete(r);
	}
#endif

      if (letter < RE_FINAL_TOK)
	{
	  for(i = 0 ; i < DIC_SEARCH_REGE_LIST ; i++)
	    if (list->valid[i])
	      {
		if (list->letters[i][letter] && (z = y->next[(int)list->symbl[i]]) != NULL)
		  {
		    DMSG(printf("*** letter "));
		    DMSG(regexp_print_letter(stdout,letter));
		    DMSG(printf("is in "));
		    DMSG(regexp_print_letter(stdout,i));

		    r = s_automaton_successor(z->id,RE_EPSILON,nfa, list);    
		    alist_insert(Ry,r);
		    alist_delete(r);
		    alist_insert(Ry,z->id);
		  }
	      }
	}

      //      if (alist_is_empty(Ry))                              /* Ry = \empty             */
      //	return Ry;

      alist_insert(R,Ry);                                  /* R = R \cup Ry           */
      alist_delete(Ry);
    }

  return R;
}

static void
s_automaton_node_set_accept(astate s, Automaton nfa)
{
  void* idx;
  alist_elt ptr;

  DMSG(printf("=== setting accept for node (%s) :",s_automaton_id_to_str(s->id)));
  for(ptr = alist_get_first(nfa->states) ; ptr ; ptr = alist_get_next(nfa->states,ptr))
    {
      astate ns = (astate)alist_elt_get_value(ptr);
      idx = alist_elt_get_value(alist_get_first(ns->id));
      DMSG(printf("%s ",s_automaton_id_to_str(ns->id)));
      if (ns->accept && alist_is_in(s->id,idx))
	{
	  DMSG(printf("(ok) ")); 
	  s->accept = 1;
	}
    }
  DMSG(printf("\n"));
}

static Automaton 
s_automaton_NFA_to_DFA(Automaton nfa, struct search_RegE_list_t *list)
{
  Automaton dfa = NULL;
  alist temp_id;
  alist_elt ptr;
  astate temp_state, current_state;
  alist L;
  int letter;

  dfa = s_automaton_create();
  L   = alist_create();

  temp_id         = alist_clone(nfa->init_state->id);
  temp_state      = s_automaton_state_create(temp_id);
  dfa->init_state = temp_state;
  s_automaton_add_state(dfa,temp_state);
  alist_add(L,temp_state);
  while (! alist_is_empty(L))
    {
      current_state = (astate)alist_pop_first_value(L);
      DMSG(printf("** current state = %s\n",s_automaton_id_to_str(current_state->id)));
      for(letter = 1; letter < DIC_LETTERS; letter++)
	{
	  //	  DMSG(printf("*** start successor of %s\n",s_automaton_id_to_str(current_state->id)));

	  temp_id = s_automaton_successor(current_state->id,letter,nfa,list);

	  if (! alist_is_empty(temp_id))
	    {
	      
	      DMSG(printf("*** successor of %s for ",s_automaton_id_to_str(current_state->id)));
	      DMSG(regexp_print_letter(stdout,letter));
	      DMSG(printf(" = %s\n", s_automaton_id_to_str(temp_id)));

	      temp_state = s_automaton_get_state(dfa,temp_id);

	      //	  DMSG(printf("*** automaton get state -%s- ok\n",s_automaton_id_to_str(temp_id)));
	      
	      if (temp_state == NULL)
		{
		  temp_state = s_automaton_state_create(temp_id);
		  s_automaton_add_state(dfa,temp_state);
		  current_state->next[letter] = temp_state;
		  alist_add(L,temp_state);
		}
	      else
		{
		  alist_delete(temp_id);
		  current_state->next[letter] = temp_state;
		}
	    }
	  else
	    {
	      alist_delete(temp_id);
	    }
	} 
    }

  for(ptr = alist_get_first(dfa->states) ; ptr ; ptr = alist_get_next(dfa->states,ptr))
    {
      astate s = (astate)alist_elt_get_value(ptr);
      s_automaton_node_set_accept(s,nfa);
    }
  
  alist_delete(L);
  return dfa;
}

/* ************************************************** *
 * ************************************************** *
 * ************************************************** */

static automaton 
s_automaton_finalize(Automaton a)
{
  int i,l;
  automaton fa = NULL; 
  alist_elt ptr;
  astate s;

  if (a == NULL)
    return NULL;

  /* creation */
  fa = (automaton)malloc(sizeof(struct automaton_t));
  fa->nstates = a->nstates;
  fa->accept  = (int*) malloc((fa->nstates + 1)*sizeof(int));
  memset(fa->accept,0,(fa->nstates + 1)*sizeof(int));
  fa->trans   = (int**)malloc((fa->nstates + 1)*sizeof(int*));
  for(i=0; i <= fa->nstates; i++)
    {
      fa->trans[i] = (int*)malloc(MAX_TRANSITION_LETTERS * sizeof(int));
      memset(fa->trans[i],0,MAX_TRANSITION_LETTERS * sizeof(int));
    }

  /* create new id for states */
  for(i = 1 , ptr = alist_get_first(a->states); ptr ; ptr = alist_get_next(a->states,ptr), i++)
    {
      s = (astate)alist_elt_get_value(ptr);
      s->id_static = i;
    }

  /* build new automaton */
  for(ptr = alist_get_first(a->states); ptr ; ptr = alist_get_next(a->states,ptr))
    {
      s = (astate)alist_elt_get_value(ptr);
      i = s->id_static;

      if (s == a->init_state) 
	fa->init = i;
      if (s->accept == 1)    
	fa->accept[i] = 1;

      for(l=0; l < MAX_TRANSITION_LETTERS; l++)
	if (s->next[l])
	  fa->trans[i][l] = s->next[l]->id_static;
    }

  return fa;
}


/* ************************************************** *
 * ************************************************** *
 * ************************************************** */

static void 
s_automaton_print_nodes(FILE* f, Automaton a)
{
  char * sid;
  astate s;
  alist_elt ptr;
  for(ptr = alist_get_first(a->states) ; ptr != NULL ; ptr = alist_get_next(a->states,ptr))
    {
      s = alist_elt_get_value(ptr);
      sid = s_automaton_id_to_str(s->id);
      fprintf(f,"\t\"%s\" [label = \"%s\"",sid,sid);
      if (s == a->init_state)
	    {
	      fprintf(f,", style = filled, color=lightgrey");
	    }
      if (s->accept)
	{
	  fprintf(f,", shape = doublecircle");
	}
      fprintf(f,"];\n");
    }
  fprintf(f,"\n");
}

static void 
s_automaton_print_edges(FILE* f, Automaton a)
{
  int letter;
  char * sid;
  astate s;
  alist_elt ptr;
  for(ptr = alist_get_first(a->states) ; ptr != NULL ; ptr = alist_get_next(a->states,ptr))
    {
      s = (astate)alist_elt_get_value(ptr);
      for(letter=0; letter < 255; letter++)
	{
	  if (s->next[letter])
	    {
	      sid = s_automaton_id_to_str(s->id);
	      fprintf(f,"\t\"%s\" -> ",sid);
	      sid = s_automaton_id_to_str(s->next[letter]->id);
	      fprintf(f,"\"%s\" [label = \"",sid);
	      regexp_print_letter(f,letter);
	      fprintf(f,"\"];\n");
	    }
	}
    }
}

static void 
s_automaton_dump(Automaton a, char* filename)
{
  FILE* f;
  pid_t   pid;

  if (a == NULL)
    return;
  f=fopen(filename,"w");
  fprintf(f,"digraph automaton {\n");
  s_automaton_print_nodes(f,a);
  s_automaton_print_edges(f,a);
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

/* ************************************************** *
 * ************************************************** *
 * ************************************************** */

