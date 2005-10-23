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
/* Elit is distributed in the hope that it will be useful,                   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

/*
 * $Id: automaton.h,v 1.8 2005/10/23 14:53:43 ipkiss Exp $
 */

/**
 *  \file   automaton.h
 *  \brief  Diterministic Finite Automaton 
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#ifndef _DIC_AUTOMATON_H_
#define _DIC_AUTOMATON_H_
#if defined(__cplusplus)
extern "C" 
  {
#endif 

typedef struct automaton_t       *automaton;

    /**
     * build a static deterministic finite automaton from 
     * "init_state", "ptl" and "PS" given by the parser
     */     
automaton automaton_build(int init_state, int *ptl, int *PS, struct search_RegE_list_t *list);

    /**
     * automaton delete function
     */
void      automaton_delete         (automaton a);

    /**
     * get the number of states in the automaton
     * @returns number of states
     */
int       automaton_get_nstate     (automaton a);

    /**
     * query the id of the init state
     * @returns init state id
     */
int       automaton_get_init       (automaton a);

    /**
     * ask for the acceptor flag for the state
     * @returns boolean flag 0 or 1
     */
int       automaton_get_accept     (automaton a, int state);

    /**
     * returns the next state when the transition is taken
     * @returns next state id (1 <= id <= nstate, 0 = invalid id)
     */
int       automaton_get_next_state (automaton a, int start, char l);

void      automaton_dump           (automaton a, char* filename);

#if defined(__cplusplus)
  }
#endif 
#endif /* _DIC_AUTOMATON_H_ */
