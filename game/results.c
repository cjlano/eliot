/* Eliot                                                                     */
/* Copyright (C) 1999  Antoine Fraboulet                                     */
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

/* $Id: results.c,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#include <string.h>
#include <stdlib.h>
#include "tiles.h"
#include "round.h"
#include "results.h"

#define RESULTS_INTERNAL_MAX 300

struct tresults {
  Round list [RESULTS_INTERNAL_MAX];
  int nresults;
};


Results 
Results_create()
{
  int i;
  Results r;
  r = (Results)malloc(sizeof(struct tresults));
  for(i=0; i < RESULTS_INTERNAL_MAX; i++)
    {
      r->list[i] = Round_create();
    }
  Results_init(r);
  return r;
}


void 
Results_init(Results r)
{
  int i;
  for(i=0; i < RESULTS_INTERNAL_MAX; i++)
    Round_init(r->list[i]);
  r->nresults = 0;
}


void 
Results_destroy(Results r)
{
  int i;
  if (r)
    {
      for(i=0; i < RESULTS_INTERNAL_MAX; i++)
	Round_destroy(r->list[i]);
      free(r);
    }
}


void
Results_add(Results re, Round ro)
{
    if (re->nresults != RESULTS_INTERNAL_MAX) {
	 Round_copy(re->list[re->nresults++],ro);
    }
}


void 
Results_addsorted(Results re, Round ro)
{
  int i;

  if (re == NULL || ro == NULL)
    return;
  i=re->nresults;
  while ((i > 0) && (Round_points(re->list[i-1]) < Round_points(ro)))
    {
      if (i != RESULTS_INTERNAL_MAX)
	Round_copy(re->list[i], re->list[i-1]);
      i--;
    }
  if (i != RESULTS_INTERNAL_MAX)
    {
      Round_copy(re->list[i],ro);
      if (re->nresults != RESULTS_INTERNAL_MAX) 
	  re->nresults++;
    }
}


int
Results_in(Results r)
{
  return r->nresults;
}


Round
Results_get(Results r, int i)
{
  if (i >= 0 && i < r->nresults)
    return r->list[i];
  return NULL;
}


void
Results_deletelast(Results r)
{
     if (r->nresults)
	  r->nresults--;
}


