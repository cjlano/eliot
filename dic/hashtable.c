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
 * $Id: hashtable.c,v 1.2 2005/05/05 23:45:04 afrab Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

typedef struct _Hash_node {
  struct _Hash_node *next;
  void* key;
  unsigned int keysize;
  void* value;
  unsigned int valuesize;
} Hash_node;

struct _Hash_table {
  unsigned int size;
  Hash_node** nodes;
};


Hash_table
hash_init(unsigned int size)
{
  Hash_table ht;

  ht = (Hash_table) calloc(1,sizeof(struct _Hash_table));
  ht->size = size;
  ht->nodes = (Hash_node  **) calloc (size, sizeof (Hash_node*));
  return ht;
}

void
hash_rec_free(Hash_node* node)
{
  if (node)
    {
      if (node->next)
	hash_rec_free(node->next);
      if (node->key)
	free(node->key);
      if (node->value)
	free(node->value);
      free(node);
    }
}

int
hash_destroy(Hash_table hashtable)
{
  unsigned int i;
  if (hashtable)
    {
      for(i=0; i<hashtable->size; i++)
	if (hashtable->nodes[i])
	  hash_rec_free(hashtable->nodes[i]);
      if (hashtable->nodes)
	free(hashtable->nodes);
      free(hashtable);
    }
  return 0;
}


static unsigned int
hash_key(Hash_table hashtable, void* ptr, unsigned int size)
{
  unsigned int i;
  unsigned int key = 0;

  if (size % 4 == 0)
    {
      unsigned int *v = (unsigned int*)ptr;
      for (i = 0; i < (size / 4); i++)
	key ^= (key << 3) ^ (key >> 1) ^ v[i];
    }
  else
    {
      unsigned char *v = (unsigned char*)ptr;
      for (i = 0; i < size; i++)
	key ^= (key << 3) ^ (key >> 1) ^ v[i];
    }
  key %= hashtable->size;
  return key;
}


void*
hash_find(Hash_table hashtable, void* key, unsigned int keysize)
{
  Hash_node *entry;
  unsigned int h_key;

  h_key = hash_key(hashtable,key,keysize);
  for (entry = hashtable->nodes[h_key]; entry; entry = entry -> next)
    {
      if ((entry -> keysize == keysize) &&
	  (memcmp(entry->key,key,keysize) == 0))
	{
	  return entry->value;
	}
    }
  return NULL;
}


static Hash_node*
new_entry(void* key, unsigned int keysize, void* value, unsigned int
	  valuesize)
{
  Hash_node *n;
  n = (Hash_node*)calloc(1,sizeof(Hash_node));
  n->key = (void*)malloc(keysize);
  n->value = (void*)malloc(valuesize);
  n->keysize = keysize;
  n->valuesize = valuesize;
  memcpy(n->key,key,keysize);
  memcpy(n->value,value,valuesize);
  return n;
}


int
hash_add(Hash_table hashtable,
	 void* key, unsigned int keysize, 
	 void* value, unsigned int valuesize)
{
  Hash_node *entry;
  unsigned int h_key;

  h_key = hash_key(hashtable,key,keysize);
  entry = new_entry(key,keysize,value,valuesize);
  entry->next = hashtable->nodes[h_key];
  hashtable->nodes[h_key] = entry;

  return 0;
}


