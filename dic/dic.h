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
 *  \file   dic.h
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
typedef unsigned char dic_code_t;

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
dic_code_t Dic_chr (Dictionary dic, dic_elt_t elt);

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
unsigned int Dic_lookup(Dictionary dic, dic_elt_t root, dic_code_t* pattern);

    /**
     * Dic_char returns the character associated with an element
     * (in the range ['A'-'Z']), or the null character ('\0').
     * @returns ASCII code for the character
     */
char   Dic_char (Dictionary dic, dic_elt_t elt);

    /**
     * Find the dictionary element matching the pattern starting
     * from the given root node by walking the dictionary tree
     * @params dic : valid dictionary
     * @params root : starting dictionary node for the search
     * @params pattern : string made of uppercase characters in the range
     * ['A'-'Z']. The pattern must be null ('\0') terminated
     * @returns 0 if the string cannot be matched otherwise returns the
     * element that results from walking the dictionary according to the
     * pattern
     */
unsigned int Dic_char_lookup(Dictionary dic, dic_elt_t root, char* pattern);

#if defined(__cplusplus)
  }
#endif
#endif /* _DIC_H_ */

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
