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

/* $Id: dic_search.h,v 1.2 2004/06/19 18:43:35 afrab Exp $ */

#ifndef _DIC_SEARCH_H_
#define _DIC_SEARCH_H_

#if defined(__cplusplus)
extern "C" 
  {
#endif 

#define DIC_WORD_MAX 16
#define RES_7PL1_MAX 200
#define RES_RACC_MAX 100
#define RES_BENJ_MAX 100
#define RES_CROS_MAX 200
#define RES_REGE_MAX 200

int  Dic_search_word(Dictionary,const char*);
void Dic_search_7pl1(Dictionary,char* rack, char wordlist[LETTERS][RES_7PL1_MAX][DIC_WORD_MAX], int joker);
void Dic_search_Racc(Dictionary,char* word, char wordlist[RES_RACC_MAX][DIC_WORD_MAX]);
void Dic_search_Benj(Dictionary,char* word, char wordlist[RES_BENJ_MAX][DIC_WORD_MAX]);
void Dic_search_Cros(Dictionary,char* mask, char wordlist[RES_CROS_MAX][DIC_WORD_MAX]);
void Dic_search_RegE(Dictionary,char* mask, char wordlist[RES_CROS_MAX][DIC_WORD_MAX]);

#if defined(__cplusplus)
  }
#endif 
#endif
