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
/*
 * $Id: regexp.h,v 1.1 2004/06/20 20:13:59 afrab Exp $
 */
#ifndef _TREE_H_
#define _TREE_H_

#define NODE_TOP    0
#define NODE_VAR    1
#define NODE_OR     2
#define NODE_AND    3
#define NODE_STAR   4

typedef struct node {
  int              type;
  char             var; 
  struct node      *fg; 
  struct node      *fd; 
  
  int numero;
  int position;
  int annulable;
  int PP;
  int DP;
} NODE;

#endif



