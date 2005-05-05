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
/* Elit is distributed in the hope that it will be useful,                   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/*
 * $Id: hashtable.h,v 1.2 2005/05/05 23:45:04 afrab Exp $	 
 */

/**
 *  \file   hashtable.h
 *  \brief  Hashtable type
 *  \author Antoine Fraboulet
 *  \date   1999
 */

#ifndef _HASHTABLE_H
#define _HASHTABLE_H
#if defined(__cplusplus)
extern "C" 
  {
#endif 

typedef struct _Hash_table* Hash_table;

Hash_table hash_init(unsigned int);
int        hash_destroy(Hash_table);
int        hash_size(Hash_table);
void*      hash_find(Hash_table,void* key,unsigned keysize);
int        hash_add (Hash_table,void* key,unsigned keysize,
		     void* value,unsigned valuesize);

#if defined(__cplusplus)
  }
#endif 
#endif /* _HASHTABLE_H_ */
