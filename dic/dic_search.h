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

/* $Id: dic_search.h,v 1.7 2005/04/27 17:35:03 afrab Exp $ */

/**
 *  \file dic_search.h
 *  \brief  Dictionary lookup functions
 *  \author Antoine Fraboulet
 *  \date   2002
 */

#ifndef _DIC_SEARCH_H_
#define _DIC_SEARCH_H_
#if defined(__cplusplus)
extern "C" 
  {
#endif 

    /**
     * number of results for Rack+1 search (Dic_search_7pl1)
     */
#define RES_7PL1_MAX 200

    /**
     * number of results for Extensions search (Dic_search_Racc)
     */
#define RES_RACC_MAX 100

    /**
     * number of results for Benjamin search (Dic_search_Benj)
     */
#define RES_BENJ_MAX 100

    /**
     * number of results for CrossWords search (Dic_search_Cros)
     */
#define RES_CROS_MAX 200

    /**
     * number of results for Regular Expression search (Dic_search_RegE)
     */
#define RES_REGE_MAX 200

    /**
     * Search for a word in the dictionnary
     * @param dic : dictionary
     * @param path : lookup word
     * @return 1 present, 0 error
     */
int  Dic_search_word(Dictionary dic, const char* path);

    /**
     * Search for all feasible word with "rack" plus one letter
     * @param dic : dictionary
     * @param rack : letters
     * @param wordlist : results
     */
void Dic_search_7pl1(Dictionary dic, const char* rack, char wordlist[DIC_LETTERS][RES_7PL1_MAX][DIC_WORD_MAX], int joker);

    /**
     * Search for all feasible word adding a letter in front or at the end
     * @param dic : dictionary
     * @param word : word
     * @param wordlist : results
     */
void Dic_search_Racc(Dictionary dic, const char* word, char wordlist[RES_RACC_MAX][DIC_WORD_MAX]);

    /**
     * Search for benjamins
     * @param dic : dictionary
     * @param rack : letters
     * @param wordlist : results
     */
void Dic_search_Benj(Dictionary dic, const char* word, char wordlist[RES_BENJ_MAX][DIC_WORD_MAX]);

    /**
     * Search for crosswords 
     * @param dic : dictionary
     * @param rack : letters
     * @param wordlist : results
     */
void Dic_search_Cros(Dictionary dic, const char* mask, char wordlist[RES_CROS_MAX][DIC_WORD_MAX]);

    /**
     * Search for words matching a regular expression 
     * @param dic : dictionary
     * @param re : regular expression
     * @param wordlist : results
     */
void Dic_search_RegE(Dictionary dic, const char* re, char wordlist[RES_REGE_MAX][DIC_WORD_MAX], struct search_RegE_list_t *list);

#if defined(__cplusplus)
  }
#endif 
#endif /* _DIC_SEARCH_H_ */
