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
 * $Id: dic_search.c,v 1.1 2004/04/08 09:43:06 afrab Exp $
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "dic_internals.h"
#include "dic.h"
#include "dic_search.h"


/****************************************/
/****************************************/

static Dawg_edge*
Dic_seek_edgeptr(Dictionary dic, const char* s, Dawg_edge *eptr)
{
  if (*s)
    {
      Dawg_edge *p = dic->dawg + eptr->ptr;
      do {
        if (p->chr == (unsigned)(*s & CHAR))    
          return Dic_seek_edgeptr (dic,s + 1, p); 
      } while (!(*p++).last);
      return dic->dawg;                       
    }
  else                                          
    return eptr;                                 
}


/****************************************/
/****************************************/


int
Dic_search_word(Dictionary dic, const char* word)
{
  Dawg_edge *e;
  e = Dic_seek_edgeptr(dic,word,dic->dawg + dic->root);
  return e->term;
}


/****************************************/
/****************************************/
/* global variables for Dic_search_word_by_len :
 *
 * a pointer to the structure is passed as a parameter
 * so that all the search_* variables appear to the functions
 * as global but the code remains re-entrant. 
 * Should be better to change the algorithm ...
 */

struct params_7plus1_t {
 Dictionary search_dic;
 int search_len;
 int search_wordlistlen;
 int search_wordlistlenmax;
 char search_wordtst[DIC_WORD_MAX];
 char search_letters[LETTERS];
 char (*search_wordlist)[RES_7PL1_MAX][DIC_WORD_MAX];
};

static void
Dic_search_word_by_len(struct params_7plus1_t *params, int i, Dawg_edge *edgeptr)
{
  /* depth first search in the dictionary */
  do {
    /* we use a static array and not a real list so we have to stop if 
     * the array is full */
    if (params->search_wordlistlen >= params->search_wordlistlenmax)
      break;

    /* the test is false only when reach the end-node */
    if (edgeptr->chr)                        
      {                                     

	/* is the letter available in search_letters */
	if (params->search_letters[edgeptr->chr])
	  {                         
	    params->search_wordtst[i] = edgeptr->chr + 'A' - 1;
	    params->search_letters[edgeptr->chr] --;
	    if (i == params->search_len)
	      {
		if ((edgeptr->term)
		  /* && (params->search_wordlistlen < params->search_wordlistlenmax) */)
		  strcpy((*params->search_wordlist)[params->search_wordlistlen++],params->search_wordtst);
	      }
	    else /* if (params->search_wordlistlen < params->search_wordlistlenmax) */
	      {
		Dic_search_word_by_len(params,i + 1, params->search_dic->dawg + edgeptr->ptr);
	      }
	    params->search_letters[edgeptr->chr] ++;
	    params->search_wordtst[i] = '\0';
	  }
	
	/* the letter is of course available if we have a joker available */
	if (params->search_letters[0])
	  {
	    params->search_wordtst[i] = edgeptr->chr + 'a' - 1;
	    params->search_letters[0] --;
	    if (i == params->search_len)
	      {
		if ((edgeptr->term)
		     /* && (params->search_wordlistlen < params->search_wordlistlenmax) */)
		  strcpy((*(params->search_wordlist))[params->search_wordlistlen++],params->search_wordtst);
	      }
	    else /* if (params->search_wordlistlen < params->search_wordlistlenmax) */
	      {
		Dic_search_word_by_len(params,i + 1,params->search_dic->dawg + edgeptr->ptr);
	      }
	    params->search_letters[0] ++;
	    params->search_wordtst[i] = '\0';
	  }
      } 
  } while (! (*edgeptr++).last);
}

void
Dic_search_7pl1(Dictionary dic, char* rack, 
		char buff[LETTERS][RES_7PL1_MAX][DIC_WORD_MAX], 
		int joker)
{
  int i,j,wordlen;
  char* r = rack;
  struct params_7plus1_t params;
  Dawg_edge *root_edge;

  for(i=0; i < LETTERS; i++)
    for(j=0; j < RES_7PL1_MAX; j++)
      buff[i][j][0] = '\0';

  for(i=0; i<LETTERS; i++) 
    params.search_letters[i] = 0; 

  if (dic == NULL || rack == NULL)
    return;

  /* 
   * the letters are verified and changed to the dic internal
   * representation (*r & CHAR)
   */
  for(wordlen=0; wordlen < DIC_WORD_MAX && *r; r++)
    {
      if (isalpha(*r))
	{
	  params.search_letters[(int)*r & CHAR]++;
          wordlen++;
	}
      else if (*r == '?')
	{
	  if (joker)
	    {
	      params.search_letters[0]++;
	      wordlen++;
	    }
	  else
	    {
	      strncpy(buff[0][0],"** joker **",DIC_WORD_MAX);
	      return;
	    }
	}
    }

  if (wordlen < 1)
    return;
  
  root_edge = dic->dawg + (dic->dawg[dic->root].ptr); 

  params.search_dic = dic;
  params.search_wordlistlenmax = RES_7PL1_MAX;
  
  /* search for all the words that can be done with the letters */
  params.search_len = wordlen - 1;
  params.search_wordtst[wordlen]='\0';
  params.search_wordlist = & buff[0];
  params.search_wordlistlen = 0; 
  Dic_search_word_by_len(&params,0,root_edge);

  /* search for all the words that can be done with the letters +1 */
  params.search_len = wordlen;
  params.search_wordtst[wordlen + 1]='\0';
  for(i='a'; i <= 'z'; i++)
    {
      params.search_letters[i & CHAR]++;

      params.search_wordlist = & buff[i & CHAR];
      params.search_wordlistlen = 0; 
      Dic_search_word_by_len(&params,0,root_edge);

      params.search_letters[i & CHAR]--;
    }
}

/****************************************/
/****************************************/

void
Dic_search_Racc(Dictionary dic, char* word, char wordlist[RES_RACC_MAX][DIC_WORD_MAX])
{
  /* search_racc will try to add a letter in front and at the end of a word */

  int i,wordlistlen;
  Dawg_edge *edge;
  char wordtst[DIC_WORD_MAX];

  for(i=0; i < RES_RACC_MAX; i++)
    wordlist[i][0] = 0;

  if (dic == NULL || wordlist == NULL)
    return;

  /* let's try for the front */
  wordlistlen = 0;
  strcpy(wordtst+1,word);
  for(i='a'; i <= 'z'; i++)
    {
      wordtst[0] = i;
      if (Dic_search_word(dic,wordtst) && wordlistlen < RES_RACC_MAX)
	strcpy(wordlist[wordlistlen++],wordtst);
    }
  
  /* add a letter at the end */
  for(i=0; word[i]; i++)
    wordtst[i] = word[i];

  wordtst[i  ] = '\0';
  wordtst[i+1] = '\0';
  
  edge = Dic_seek_edgeptr(dic,word,dic->dawg + dic->root);

  /* points to what the next letter can be */
  edge = dic->dawg + edge->ptr;
  
  if (edge != dic->dawg)
    {
      do {
	  if (edge->term && wordlistlen < RES_RACC_MAX)
	    {
	      wordtst[i] = edge->chr + 'a' - 1;
	      strcpy(wordlist[wordlistlen++],wordtst);
	    }
      } while (!(*edge++).last);
    }
}

/****************************************/
/****************************************/


void
Dic_search_Benj(Dictionary dic, char* word, char wordlist[RES_BENJ_MAX][DIC_WORD_MAX])
{
  int i,wordlistlen;
  char wordtst[DIC_WORD_MAX];
  Dawg_edge *edge0,*edge1,*edge2,*edgetst;

  for(i=0; i < RES_BENJ_MAX; i++)
    wordlist[i][0] = 0;

  if (dic == NULL || word == NULL)
    return;
  
  wordlistlen = 0;

  strcpy(wordtst+3,word);
  edge0 = dic->dawg + (dic->dawg[dic->root].ptr);
  do {
    wordtst[0] = edge0->chr + 'a' - 1;
    edge1 = dic->dawg + edge0->ptr;
    do {
      wordtst[1] = edge1->chr + 'a' - 1;
      edge2  = dic->dawg + edge1->ptr;
      do {
	wordtst[2] = edge2->chr + 'a' - 1;
	edgetst = Dic_seek_edgeptr(dic,word,edge2);
	if (edgetst->term && wordlistlen < RES_BENJ_MAX)
	  strcpy(wordlist[wordlistlen++],wordtst);
      } while (!(*edge2++).last);
    } while (!(*edge1++).last);
  } while (!(*edge0++).last);
}


/****************************************/
/****************************************/

struct params_cross_t {
 Dictionary dic;
 int wordlen;
 int wordlistlen;
 int wordlistlenmax;
 char mask[DIC_WORD_MAX];
};


void
Dic_search_cross_rec(struct params_cross_t *params, char wordlist[RES_CROS_MAX][DIC_WORD_MAX], Dawg_edge *edgeptr)
{
  Dawg_edge *current = params->dic->dawg + edgeptr->ptr;

  if (params->mask[params->wordlen] == '\0' && edgeptr->term)
    {
      if (params->wordlistlen < params->wordlistlenmax)
	strcpy(wordlist[params->wordlistlen++],params->mask);
    }
  else if (params->mask[params->wordlen] == '.')
    {
      do
	{
	  params->mask[params->wordlen] = current->chr + 'a' - 1;
	  params->wordlen ++;
	  Dic_search_cross_rec(params,wordlist,current);
	  params->wordlen --;
	  params->mask[params->wordlen] = '.';
	}
      while (!(*current++).last);
    }
  else 
    {
      do
	{
	  if (current->chr == (params->mask[params->wordlen] & CHAR))
	    {
	      params->wordlen ++;
	      Dic_search_cross_rec(params,wordlist,current);
	      params->wordlen --;
	      break;
	    }
	}
      while (!(*current++).last);
    }
}



void
Dic_search_Cros(Dictionary dic, char* mask, char wordlist[RES_CROS_MAX][DIC_WORD_MAX])
{
  int  i;
  struct params_cross_t params;

  for(i=0; i < RES_CROS_MAX; i++)
    wordlist[i][0] = 0;

  if (dic == NULL || mask == NULL)
    return;

  for(i=0; i < DIC_WORD_MAX && mask[i]; i++)
    {
      if (isalpha(mask[i]))
	params.mask[i] = (mask[i] & CHAR) + 'A' - 1;
      else
	params.mask[i] = '.';
    }
  params.mask[i] = '\0';

  params.dic            = dic;
  params.wordlen        = 0;
  params.wordlistlen    = 0;
  params.wordlistlenmax = RES_CROS_MAX;
  Dic_search_cross_rec(&params, wordlist, dic->dawg + dic->root);
}

/****************************************/
/****************************************/

