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
 *  \file   regexpmain.c
 *  \brief  Program used to test regexp
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dic.h"
#include "regexp.h"
#include "dic_search.h"

/********************************************************/
/********************************************************/
/********************************************************/

const unsigned int all_letter[DIC_LETTERS] =
  {
    /*                      1  1 1 1 1 1 1 1 1 1 2 2 2  2  2  2  2 */
    /* 0 1 2 3 4  5 6 7 8 9 0  1 2 3 4 5 6 7 8 9 0 1 2  3  4  5  6 */
    /* x A B C D  E F G H I J  K L M N O P Q R S T U V  W  X  Y  Z */
       0,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1, 1, 1, 1, 1
  };

const unsigned int vowels[DIC_LETTERS] =
  {
    /* x A B C D  E F G H I J  K L M N O P Q R S T U V  W  X  Y  Z */
       0,1,0,0,0, 1,0,0,0,1,0, 0,0,0,0,1,0,0,0,0,0,1,0, 0, 0, 1, 0
  };

const unsigned int consonants[DIC_LETTERS] =
  {
    /* x A B C D  E F G H I J  K L M N O P Q R S T U V  W  X  Y  Z */
       0,0,1,1,1, 0,1,1,1,0,1, 1,1,1,1,0,1,1,1,1,1,0,1, 1, 1, 1, 1
  };

void init_letter_lists(struct search_RegE_list_t *list)
{
  int i;
  memset (list,0,sizeof(*list));
  list->minlength = 1;
  list->maxlength = 15;
  list->valid[0] = 1; // all letters
  list->symbl[0] = RE_ALL_MATCH;
  list->valid[1] = 1; // vowels
  list->symbl[1] = RE_VOWL_MATCH;
  list->valid[2] = 1; // consonants
  list->symbl[2] = RE_CONS_MATCH;
  for(i=0; i < DIC_LETTERS; i++)
    {
      list->letters[0][i] = all_letter[i];
      list->letters[1][i] = vowels[i];
      list->letters[2][i] = consonants[i];
    }
  list->valid[3] = 0; // user defined list 1
  list->symbl[3] = RE_USR1_MATCH;
  list->valid[4] = 0; // user defined list 2
  list->symbl[4] = RE_USR2_MATCH;
}

/********************************************************/
/********************************************************/
/********************************************************/
void
usage(int argc, char* argv[])
{
  fprintf(stderr,"usage: %s dictionary\n",argv[0]);
  fprintf(stderr,"   dictionary : path to dawg eliot dictionary\n");
}

int main(int argc, char* argv[])
{
  int i;
  Dictionary dic;
  char wordlist[RES_REGE_MAX][DIC_WORD_MAX];
  char er[200];
  strcpy(er,".");
  struct search_RegE_list_t list;

  if (argc < 2)
    {
      usage(argc,argv);
    }

  if (Dic_load(&dic,argv[1]))
    {
      fprintf(stdout,"impossible de lire le dictionnaire\n");
      return 1;
    }

  while (strcmp(er,""))
    {
      fprintf(stdout,"**************************************************************\n");
      fprintf(stdout,"**************************************************************\n");
      fprintf(stdout,"entrer une ER:\n");
      fgets(er,sizeof(er),stdin);
      /* strip \n */
      er[strlen(er) - 1] = '\0';
      if (strcmp(er,"") == 0)
	break;

      /* automaton */
      init_letter_lists(&list);
      Dic_search_RegE(dic,er,wordlist,&list);

      fprintf(stdout,"résultat:\n");
      for(i=0; i<RES_REGE_MAX && wordlist[i][0]; i++)
	{
	  fprintf(stderr,"%s\n",wordlist[i]);
	}
    }

  Dic_destroy(dic);
  return 0;
}
