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

/* $Id: regexpmain.c,v 1.4 2005/04/19 16:26:51 afrab Exp $ */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dic.h"
#include "dic_search.h"
#include "regexp.h"

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

int main(int argc, char* argv[])
{
  int i;
  Dictionary dic;
  char wordlist[RES_REGE_MAX][DIC_WORD_MAX];
  char er[200];
  strcpy(er,".");
  struct search_RegE_list_t list;

  if (Dic_load(&dic,"/home/antoine/projets/eliot/cvs/web/download/ods4.dawg"))
    {
      fprintf(stdout,"impossible de lire le dictionnaire\n");
      return 1;
    }

  list.valid[0] = 1; // all letters
  list.symbl[0] = RE_ALL_MATCH;
  list.valid[1] = 1; // vowels
  list.symbl[1] = RE_VOWL_MATCH;
  list.valid[2] = 1; // consonants
  list.symbl[2] = RE_CONS_MATCH;
  for(i=0; i < DIC_LETTERS; i++)
    {
      list.letters[0][i] = all_letter[i];
      list.letters[1][i] = vowels[i];
      list.letters[2][i] = consonants[i];
    }  

  list.valid[3] = 0; // user defined list 1
  list.symbl[3] = RE_USR1_MATCH;
  list.valid[4] = 0; // user defined list 2
  list.symbl[5] = RE_USR2_MATCH;

  while (strcmp(er,""))
    {
      fprintf(stdout,"\nentrer une ER:\n");
      fgets(er,sizeof(er),stdin);
      /* strip \n */
      er[strlen(er) - 1] = '\0';
      /* automaton */
      Dic_search_RegE(dic,er,wordlist,&list);
      fprintf(stdout,"résultat:\n");
      for(i=0; i<RES_REGE_MAX && wordlist[i][0]; i++)
	{
	  fprintf(stdout,"  %03d : %s\n",i,wordlist[i]);
	}
    }

  Dic_destroy(dic);
  return 0;
}
