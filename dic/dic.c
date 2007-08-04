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
 *  \file   dic.c
 *  \brief  Dawg dictionary
 *  \author Antoine Fraboulet
 *  \date   2002
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "config.h"
#include "dic_internals.h"
#include "dic.h"

#define __UNUSED__ __attribute__((unused))

#if defined(WORDS_BIGENDIAN)
static uint32_t swap4(uint32_t v)
{
  uint32_t   r;
  uint8_t  *pv,*pr;

  pv = (uint8_t*)&v;
  pr = (uint8_t*)&r;
  
  pr[0] = pv[3];
  pr[1] = pv[2];
  pr[2] = pv[1];
  pr[3] = pv[0];

  return r;
}
#endif

static int
Dic_read_convert_header(Dict_header *header, FILE* file)
{

  if (fread(header,sizeof(Dict_header),1,file) != 1)
    return 1;

#if defined(WORDS_BIGENDIAN)
  header->root       = swap4(header->root);
  header->nwords     = swap4(header->nwords);
  header->nodesused  = swap4(header->nodesused);
  header->edgesused  = swap4(header->edgesused);
  header->nodessaved = swap4(header->nodessaved);
  header->edgessaved = swap4(header->edgessaved);
#else

#endif
  return 0;
}

int
Dic_check_header(Dict_header *header, const char *path)
{
  int r;
  FILE* file;
  if ((file = fopen(path,"rb")) == NULL)
    return 1;
  
  r = Dic_read_convert_header(header,file);
  fclose(file);

  return r || strcmp(header->ident,_COMPIL_KEYWORD_);
}

static void
Dic_convert_data_to_arch(Dictionary __UNUSED__ dic)
{
#if defined(WORDS_BIGENDIAN)
  int i;
  uint32_t* p;
  p = (uint32_t*)dic->dawg;
  for(i=0; i < (dic->nedges + 1); i++)
    {
      p[i] = swap4(p[i]);
    }
#endif
}

int
Dic_load(Dictionary *dic, const char* path)
{
  FILE* file;
  Dict_header header;


  *dic = NULL;
  if ((file = fopen(path,"rb")) == NULL)
    return 1;

  Dic_read_convert_header(&header,file);

  if ((*dic = (Dictionary) malloc(sizeof(struct _Dictionary))) == NULL)
    return 3;

  if (((*dic)->dawg = (Dawg_edge*)malloc((header.edgesused + 1)*sizeof(Dawg_edge))) == NULL)
    {
      free(*dic);
      *dic = NULL;
      return 4;
    }

  if (fread((*dic)->dawg,sizeof(Dawg_edge),header.edgesused + 1,file) !=
      (header.edgesused + 1))
    {
      free((*dic)->dawg);
      free(*dic);
      *dic = NULL;
      return 5;
    }

  (*dic)->root   = header.root;
  (*dic)->nwords = header.nwords;
  (*dic)->nnodes = header.nodesused;
  (*dic)->nedges = header.edgesused;

  Dic_convert_data_to_arch(*dic);

  fclose(file);
  return 0;
}


int
Dic_destroy(Dictionary dic)
{
  if (dic != NULL)
    {
      if (dic->dawg != NULL)
        free(dic->dawg);
      else
        {
          free(dic);
          return 2;
        }
      free(dic);
    }
  else
    return 1;

  return 0;
}


dic_elt_t
Dic_next(Dictionary d, dic_elt_t e)
{
     if (! Dic_last(d,e))
          return e+1;
     return 0;
}


dic_elt_t
Dic_succ(Dictionary d, dic_elt_t e)
{
  return (d->dawg[e]).ptr;
}


dic_elt_t
Dic_root(Dictionary d)
{
  return d->root;
}


dic_code_t
Dic_chr(Dictionary d, dic_elt_t e)
{
  return (dic_code_t)(d->dawg[e]).chr;
}


int
Dic_last(Dictionary d, dic_elt_t e)
{
  return (d->dawg[e]).last;
}


int
Dic_word(Dictionary d, dic_elt_t e)
{
  return (d->dawg[e]).term;
}

unsigned int
Dic_lookup(Dictionary d, dic_elt_t root, dic_code_t* s)
{
    unsigned int p;
begin:
    if (! *s)
        return root;
    if (! Dic_succ(d, root))
        return 0;
    p = Dic_succ(d, root);
    do
    {
        if (Dic_chr(d, p) == *s)
        {
            root = p;
            s++;
            goto begin;
        }
        else if (Dic_last(d, p))
        {
            return 0;
        }
        p = Dic_next(d, p);
    } while (1);

    return 0;
}

/* **************************************************************************** */
/* **************************************************************************** */
/* **************************************************************************** */
/* **************************************************************************** */

char
Dic_char(Dictionary d, dic_elt_t e)
{
  char c = (d->dawg[e]).chr;
  if (c)
    return c + 'A' - 1;
  else
    return 0;
}

unsigned int
Dic_char_lookup(Dictionary d, dic_elt_t root, char* s)
{
    unsigned int p;
begin:
    if (! *s)
        return root;
    if (! Dic_succ(d, root))
        return 0;
    p = Dic_succ(d, root);
    do
    {
        if (Dic_char(d, p) == *s)
        {
            root = p;
            s++;
            goto begin;
        }
        else if (Dic_last(d, p))
        {
            return 0;
        }
        p = Dic_next(d, p);
    } while (1);

    return 0;
}
