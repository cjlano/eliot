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

/* $Id: dic.h,v 1.7 2005/04/26 07:37:55 afrab Exp $ */

/**
 *  \file dic.h
 *  \brief  Dawg dictionary
 *  \author Antoine Fraboulet
 *  \date   2002
 */

#ifndef _DIC_H_
#define _DIC_H_
#if defined(__cplusplus)
extern "C" 
  {
#endif 
  
/** 
 * different letters in the dictionary     
 */
#define DIC_LETTERS  27

/** 
 * max length of words (including last \0) 
 */        
#define DIC_WORD_MAX 16

typedef struct _Dictionary* Dictionary;
typedef unsigned int dic_elt_t;

    /**
     * Dictionary creation and loading from a file
     * @param dic : pointer to a dictionary
     * @param path : compressed dictionary path
     * @return 0 ok, 1 error
     */
int    Dic_load   (Dictionary* dic,const char* path);

    /**
     * Destroy a dictionary
     */
int    Dic_destroy(Dictionary dic);

    /**
     * Dic_chr returns the character code associated with an element,
     * codes may range from 0 to 31. 0 is the null character.
     * @returns code for the encoded character
     */
char   Dic_chr (Dictionary dic, dic_elt_t elt);

    /**
     * Returns a boolean to show if there is another available
     * character in the current depth (a neighbor in the tree)
     * @returns 0 or 1 (true)
     */
int    Dic_last(Dictionary dic, dic_elt_t elt);

    /**
     * Returns a boolean to show if we are at the end of a word
     * (see Dic_next)
     * @returns 0 or 1 (true)
     */
int    Dic_word(Dictionary dic, dic_elt_t elt);

    /** 
     * Returns the root of the dictionary
     * @returns root element
     */
dic_elt_t Dic_root(Dictionary dic);

    /**
     * Returns the next available neighbor (see Dic_last)
     * @returns next dictionary element at the same depth
     */
dic_elt_t Dic_next(Dictionary dic, dic_elt_t elt);

    /**
     * Returns the first element available at the next depth
     * in the dictionary
     * @params dic : dictionary
     * @params elt : current dictionary element
     * @returns next element (successor)
     */
dic_elt_t Dic_succ(Dictionary dic, dic_elt_t elt);

    /**
     * Find the dictionary element matching the pattern starting
     * from the given root node by walking the dictionary tree
     * @params dic : valid dictionary
     * @params root : starting dictionary node for the search
     * @params pattern : string encoded according to the dictionary codes,
     * the pattern must be null ('\0') terminated
     * @returns 0 if the string cannot be matched otherwise returns the 
     * element that results from walking the dictionary according to the
     * pattern
     */
unsigned int Dic_lookup(Dictionary dic, dic_elt_t root, char* pattern);

#if defined(__cplusplus)
  }
#endif 
#endif /* _DIC_H_ */
