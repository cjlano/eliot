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

/* $Id: compdic.c,v 1.1 2004/04/08 09:43:06 afrab Exp $ */
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "hashtable.h"
#include "dic_internals.h"

//#define DEBUG_LIST
//#define DEBUG_OUTPUT 
//#define DEBUG_OUTPUT_L2
#define CHECK_RECURSION

char*
load_uncompressed(const char* file_name, unsigned int *dic_size) 
{
  unsigned r;
  char *uncompressed;
  FILE* file_desc;

  if ((file_desc = fopen (file_name, "r")) == NULL)
    return NULL;

  if ((uncompressed = (char*)malloc (sizeof(char)*(*dic_size))) == NULL)
    return NULL;
  
  r = fread (uncompressed, 1, *dic_size, file_desc);
  if (r < *dic_size) 
    {
      /* \n is 2 chars under M$ */
      printf("\n");
      printf("** The number of bytes read is less than the size of the file **\n");
      printf("** this may be OK if you run a Microsoft OS but not on Unix   **\n");
      printf("** please check the results.                                  **\n");
      printf("\n");
      *dic_size = r;
    }

  fclose(file_desc);
  return uncompressed;
}


int 
file_length(const char* file_name)
{
  struct stat stat_buf;
  if (stat (file_name, &stat_buf) < 0)
    return - 1;
  return (int) stat_buf.st_size;
}


void
skip_init_header(FILE* outfile, Dict_header *header)
{
  header->unused_1   = 0;
  header->unused_2   = 0;
  header->root       = 0;
  header->nwords     = 0;
  header->nodesused  = 1;
  header->edgesused  = 1;
  header->nodessaved = 0;
  header->edgessaved = 0;

  fwrite (header, sizeof(Dict_header), 1, outfile);
}


void
fix_header(FILE* outfile, Dict_header* header)
{
  strcpy(header->ident,_COMPIL_KEYWORD_);
  header->root = header->edgesused;
  rewind (outfile); 
  fwrite (header, sizeof(Dict_header), 1, outfile); 
}


void
print_header_info(Dict_header *header)
{
  printf("============================\n");
  printf("keyword length %d bytes\n",(int)strlen(_COMPIL_KEYWORD_));
  printf("keyword size   %d bytes\n",sizeof(_COMPIL_KEYWORD_));
  printf("header size    %d bytes\n",sizeof(Dict_header));
  printf("\n");
  printf("%d words\n",header->nwords);
  printf("\n");
  printf("root : %7d (edge)\n",header->root);
  printf("root : %7d (byte)\n",header->root * sizeof(Dawg_edge));
  printf("\n");
  printf("nodes : %d+%d\n",header->nodesused, header->nodessaved);
  printf("edges : %d+%d\n",header->edgesused, header->edgessaved);
  printf("============================\n");
}


void
write_node(Dawg_edge *edges, int size, int num, FILE* outfile)
{
#ifdef DEBUG_OUTPUT
  int i;
  printf("writing %d edges\n",num);
  for(i=0; i<num; i++)
    {
#ifdef DEBUG_OUTPUT_L2
      printf("ptr=%2d t=%d l=%d f=%d chr=%2d (%c)\n",
	     edges[i].ptr, edges[i].term, edges[i].last,
	     edges[i].fill, edges[i].chr, edges[i].chr -1 +'a');
#endif 
      fwrite (edges+i, sizeof(Dawg_edge), 1, outfile);
    }
#else
  fwrite (edges, size, num, outfile);
#endif
}

#define MAX_STRING_LENGTH 200


#define MAX_EDGES 2000
/* ods3:
/* ods4: 1746 */

/* global variables */
FILE*       global_outfile;
Dict_header global_header;
Hash_table  global_hashtable;

char        global_stringbuf[MAX_STRING_LENGTH]; /* Space for current string */
char*       global_endstring;                    /* Marks END of current string */
char*       global_input;
char*       global_endofinput;
 
/* 
 * Makenode takes a prefix (as postion relative to stringbuf) and
 * returns an index of the start node of a dawg that recognizes all
 * words beginning with that prefix.  String is a pointer (relative
 * to stringbuf) indicating how much of prefix is matched in the
 * input.
 */
#ifdef CHECK_RECURSION
int current_rec =0;
int max_rec = 0;
#endif

unsigned int
makenode(char *prefix)
{
  int    numedges;
  Dawg_edge  edges[MAX_EDGES];
  Dawg_edge *edgeptr = edges;
  unsigned  *saved_position;
  
#ifdef CHECK_RECURSION
  current_rec++;
  if (current_rec > max_rec)
    max_rec = current_rec;
#endif

  while (prefix == global_endstring) 
    {  
      /* More edges out of node */
      edgeptr->ptr  = 0;
      edgeptr->term = 0;
      edgeptr->last = 0;
      edgeptr->fill = 0;
      edgeptr->chr  = 0;
      
      (*(edgeptr++)).chr = (*global_endstring++ = *global_input++) & CHAR;
      if (*global_input == '\n')                 /* End of a word */
        {                       
          global_header.nwords++;
          edgeptr[-1].term = 1;                  /* Mark edge as word */
          *global_endstring++ = *global_input++; /* Skip \n */
          if (global_input == global_endofinput) /* At end of input? */
            break;

	  global_endstring = global_stringbuf;
	  while(*global_endstring == *global_input)
	    {
	       global_endstring++;
	       global_input++; 
	    }
        }
      /* make dawg pointed to by this edge */
      edgeptr[-1].ptr = makenode(prefix + 1);     
    }
  
  numedges = edgeptr - edges;
  if (numedges == 0)
    {
#ifdef CHECK_RECURSION      
      current_rec --;
#endif
      return 0;             /* Special node zero - no edges */
    }
  
  edgeptr[-1].last = 1;   /* Mark the last edge */
  
  saved_position = (unsigned int*) hash_find (global_hashtable,
					      (void*)edges,
					      numedges*sizeof(Dawg_edge)); 
  if (saved_position) 
    {           
      global_header.edgessaved += numedges;
      global_header.nodessaved++;

#ifdef CHECK_RECURSION      
      current_rec --;
#endif      
      return *saved_position;
    }
  else 
    {                   
      unsigned int node_pos;
      
      node_pos = global_header.edgesused;
      hash_add(global_hashtable,
	       (void*)edges,numedges*sizeof(Dawg_edge),
	       (void*)(&global_header.edgesused),sizeof(global_header.edgesused));
      global_header.edgesused += numedges;
      global_header.nodesused++;
      write_node (edges, sizeof(Dawg_edge), numedges, global_outfile);

#ifdef CHECK_RECURSION      
      current_rec --;
#endif
      return node_pos;
    }
}




int
main(int argc, char* argv[])
{
  int dicsize;  
  char *uncompressed;
  Dawg_edge rootnode = {0,0,0,0,0};
  Dawg_edge specialnode = {0,0,0,0,0};

  char* outfilename;
  char outfilenamedefault[] = "dict.daw";
  clock_t starttime, endtime;

  if (argc < 2)
    {
      fprintf(stderr,"usage: %s uncompressed_dic [compressed_dic]\n",argv[0]);
      exit(1);
    }

  dicsize = file_length (argv[1]);       
  if (dicsize < 0)
    {
      fprintf(stderr,"Cannot stat uncompressed dictionary %s\n",argv[1]);
      exit(1);
    }

  outfilename = (argc == 3) ? argv[2] : outfilenamedefault;

  if ((global_outfile = fopen (outfilename,"wb")) == NULL)
    {
      fprintf(stderr,"Cannot open output file %s\n",outfilename);
      exit(1);
    }

  if ((uncompressed = load_uncompressed (argv[1], &dicsize)) == NULL)
    {
      fprintf(stderr,"Cannot load uncompressed dictionary into memory\n");
      exit(1);
    }

  global_input = uncompressed;
  global_endofinput = global_input + dicsize; 

#define SCALE 0.6
  global_hashtable = hash_init(dicsize * SCALE);
#undef SCALE

  skip_init_header(global_outfile,&global_header);      
        
  specialnode.last = 1;
  write_node(&specialnode,sizeof(specialnode),1,global_outfile);
  /* 
   * Call makenode with null (relative to stringbuf) prefix;
   * Initialize string to null; Put index of start node on output 
   */
  starttime=clock();
  rootnode.ptr = makenode(global_endstring = global_stringbuf);
  endtime=clock();
  write_node(&rootnode,sizeof(rootnode),1,global_outfile);

  fix_header(global_outfile,&global_header);           

  print_header_info(&global_header);
  hash_destroy(global_hashtable);
  free(uncompressed);
  fclose(global_outfile);

  printf(" Elapsed time is                 : %f s\n", 1.0*(endtime-starttime) / CLOCKS_PER_SEC);
#ifdef CHECK_RECURSION      
  printf(" Maximum recursion level reached : %d\n",max_rec);
#endif
  return 0;
}


