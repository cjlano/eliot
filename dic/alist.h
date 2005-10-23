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
 * $Id: alist.h,v 1.2 2005/10/23 14:53:43 ipkiss Exp $
 */

/**
 *  \file   alist.h
 *  \brief  List type used by automaton 
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#ifndef _ALIST_H_
#define _ALIST_H_
#if defined(__cplusplus)
extern "C" 
  {
#endif 

    /**
     * untyped list type element
     */
    typedef struct alist_elt_t* alist_elt;

    /**
     * extract the value from an alist element
     * result is untyped si the user should know
     * what the value type is
     */
    void* alist_elt_get_value(alist_elt);
    
    /**
     * untyped list type
     */
    typedef struct alist_t* alist;

    /**
     * list creation 
     * @returns list
     */
    alist     alist_create     ();
    alist     alist_clone      (alist);

    /**
     * funtion to use on data during list deletion.  
     */
    void      alist_set_delete (alist,void (*f)(void*));

    /**
     * delete a complete list. 
     */
    void      alist_delete     (alist);
    
    /** 
     * add a element to the list
     */
    void      alist_add        (alist, void*);
    void      alist_insert     (alist, alist);
    /**
     * get first element
     */
    int       alist_is_in      (alist l, void* e);
    int       alist_equal      (alist , alist);

    alist_elt alist_get_first  (alist);

    /**
     * get next element from current
     */
    alist_elt alist_get_next   (alist,alist_elt);

    /**
     * @returns 0 or 1
     */
    int       alist_is_empty   (alist);

    int       alist_get_size   (alist);

    void*     alist_pop_first_value  (alist);

#if defined(__cplusplus)
  }
#endif 
#endif /* _ALIST_H_ */
