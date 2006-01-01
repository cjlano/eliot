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
/* Eliot is distributed in the hope that it will be useful,                  */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

/**
 *  \file   alist.c
 *  \brief  List type used by automaton
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#include <stdlib.h>
#include "alist.h"


struct alist_elt_t {
  void* info;
  alist_elt next;
};

struct alist_t {
  int size;
  void (*delete_function)(void*);
  alist_elt start;
};


void*
alist_elt_get_value(alist_elt e)
{
  return e->info;
}

alist_elt
alist_elt_create(void* info)
{
  alist_elt e;
  e = (alist_elt)malloc(sizeof(struct alist_elt_t));
  e->info = info;
  e->next = NULL;
  return e;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

alist
alist_create()
{
  alist l;
  l                  = (alist)malloc(sizeof(struct alist_t));
  l->size            = 0;
  l->start           = NULL;
  l->delete_function = NULL;
  return l;
}

alist
alist_clone(alist l)
{
  alist t;
  alist_elt ptr;
  t = alist_create();
  for(ptr = alist_get_first(l); ptr ; ptr = alist_get_next(l,ptr))
    {
      alist_add(t,alist_elt_get_value(ptr));
    }
  return t;
}

void
alist_set_delete (alist l, void (*f)(void*))
{
  l->delete_function = f;
}

static void
alist_delete_rec(alist_elt e, void (*delete_function)(void*))
{
  if (e != NULL)
    {
      alist_delete_rec(e->next, delete_function);
      if (delete_function)
	delete_function(e->info);
      e->info = NULL;
      free(e);
    }
}

void
alist_delete(alist l)
{
  alist_delete_rec(l->start,l->delete_function);
  free(l);
}

void
alist_add(alist l, void* value)
{
  alist_elt e;
  e = alist_elt_create(value);
  e->next = l->start;
  l->start = e;
  l->size ++;
}

int
alist_is_in(alist l, void* e)
{
  alist_elt ptr;
  for(ptr = alist_get_first(l); ptr; ptr = alist_get_next(l,ptr))
    if (alist_elt_get_value(ptr) == e)
      return 1;
  return 0;
}

int
alist_equal(alist id1, alist id2)
{
  alist_elt e1;

  if (alist_get_size(id1) != alist_get_size(id2))
    return 0;

  for(e1 = alist_get_first(id1) ; e1 ; e1 = alist_get_next(id1,e1))
    {
      if (! alist_is_in(id2, alist_elt_get_value(e1)))
	return 0;
    }

  return 1;
}

void
alist_insert(alist dst, alist src)
{
  alist_elt ptr;
  for(ptr = alist_get_first(src); ptr ; ptr = alist_get_next(src,ptr))
    {
      void *e = alist_elt_get_value(ptr);
      if (! alist_is_in(dst,e))
	alist_add(dst,e);
    }
}

alist_elt
alist_get_first(alist l)
{
  return l->start;
}

alist_elt
alist_get_next(alist l, alist_elt e)
{
  return e->next;
}

void*
alist_pop_first_value(alist l)
{
  void* p = NULL;
  alist_elt e = l->start;
  if (e)
    {
      l->start = e->next;
      e->next  = NULL;
      p = e->info;
      l->size --;
      alist_delete_rec(e,l->delete_function);
    }
  return p;
}

int
alist_get_size(alist l)
{
  return l->size;
}

int
alist_is_empty(alist l)
{
  return l->size == 0;
}
