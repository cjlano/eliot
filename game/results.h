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

/* $Id: results.h,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#ifndef _RESULTS_H_
#define _RESULTS_H_

#if defined(__cplusplus)
extern "C" {
#endif 

  /*************************
   * Results is a container. The structure
   * stores the rounds that have been found
   * during a search on the board
   *************************/

typedef struct tresults* Results;

  /*************************
   * Results general routines
   *************************/

Results Results_create     ();
void    Results_init       (Results);
void    Results_destroy    (Results);

  /*************************
   * 
   * 
   *************************/

int     Results_in         (Results);
Round   Results_get        (Results,int);

void    Results_add        (Results,Round);
void    Results_addsorted  (Results,Round);

void    Results_deletelast (Results);

  /*************************
   * 
   * 
   *************************/

#if defined(__cplusplus)
	   }
#endif 
#endif
