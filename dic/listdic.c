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
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "dic_internals.h"
#include "dic.h"


static void
print_dic_rec(FILE* out, Dictionary dic, char *buf, char* s, Dawg_edge i)
{
  if (i.term)  /* edge points at a complete word */
    {
      *s = '\0';
      fprintf (out,"%s\n", buf);
    }
  if (i.ptr)
    {           /* Compute index: is it non-zero ? */
      Dawg_edge *p = dic->dawg + i.ptr;
      do {                         /* for each edge out of this node */
        *s = p->chr + 'a' - 1;
        print_dic_rec (out,dic,buf,s + 1, *p);
      }
      while (!(*p++).last);
    }
}

void
dic_load(Dictionary* dic, char* filename)
{
  int res;
  if ((res = Dic_load(dic, filename)) != 0)
    {
      switch (res) {
      case 1: printf("chargement: problème d'ouverture de %s\n",filename); break;
      case 2: printf("chargement: mauvais en-tete de dictionnaire\n"); break;
      case 3: printf("chargement: problème 3 d'allocation mémoire\n"); break;
      case 4: printf("chargement: problème 4 d'alocation mémoire\n"); break;
      case 5: printf("chargement: problème de lecture des arcs du dictionnaire\n"); break;
      default: printf("chargement: problème non-repertorié\n"); break;
      }
      exit(res);
    }
}

void
print_dic_list(char* filename, char* out)
{
  FILE* fout;
  Dictionary dic;
  static char buf[80];

  dic_load(&dic,filename);

  if (strcmp(out,"stdout") == 0)
    print_dic_rec(stdout,dic,buf,buf,dic->dawg[dic->root]);
  else if (strcmp(out,"stderr") == 0)
    print_dic_rec(stderr,dic,buf,buf,dic->dawg[dic->root]);
  else
    {
      if ((fout = fopen(out,"w")) == NULL)
	return;
      print_dic_rec(fout,dic,buf,buf,dic->dawg[dic->root]);
      fclose(fout);
    }
  Dic_destroy(dic);
}

char
b2h(int i)
{
  if (i < 10)
    return i+'0';
  return i-10+'a';
}

char*
hexb(unsigned char h)
{
  static char buf[3];
  buf[0] = b2h((h & 0xf0) >> 4);
  buf[1] = b2h((h & 0x0f));
  buf[2] = 0;
  return buf;
}

char*
hexl(unsigned int h)
{
  static char buf[9];
  int i;
  for(i=0; i<4; i++)
    {
      int l = h >> (24 - i*8);
      buf[i*2+0] = b2h((l & 0xf0) >> 4);
      buf[i*2+1] = b2h((l & 0x0f));
    }
  buf[8] = 0;
  return buf;
}

char*
offset(void* base, void* off)
{
  static char buf[20];
  int o = (char*)off - (char*)base;
  sprintf(buf,"%s",hexb(o));
  return buf;
}

void
print_header(char* filename)
{
  FILE* file;
  Dict_header header;

  if ((file = fopen(filename,"rb")) == NULL)
    return;
  if (fread(&header,sizeof(Dict_header),1,file) != 1)
    return;
  fclose(file);

  printf("Dictionary header information\n");
  printf("0x%s ident       : %s\n",offset(&header,&header.ident),header.ident);
  printf("0x%s unused 1    : %6d %s\n",offset(&header,&header.unused_1)  ,header.unused_1  ,hexl(header.unused_1));
  printf("0x%s unused 2    : %6d %s\n",offset(&header,&header.unused_2)  ,header.unused_2  ,hexl(header.unused_2));
  printf("0x%s root        : %6d %s\n",offset(&header,&header.root)      ,header.root      ,hexl(header.root));
  printf("0x%s words       : %6d %s\n",offset(&header,&header.nwords)    ,header.nwords    ,hexl(header.nwords));
  printf("0x%s edges used  : %6d %s\n",offset(&header,&header.edgesused) ,header.edgesused ,hexl(header.edgesused));
  printf("0x%s nodes used  : %6d %s\n",offset(&header,&header.nodesused) ,header.nodesused ,hexl(header.nodesused));
  printf("0x%s nodes saved : %6d %s\n",offset(&header,&header.nodessaved),header.nodessaved,hexl(header.nodessaved));
  printf("0x%s edges saved : %6d %s\n",offset(&header,&header.edgessaved),header.edgessaved,hexl(header.edgessaved));
  printf("\n");
  printf("sizeof(header) = 0x%s (%d)\n",hexb(sizeof(header)),sizeof(header));
}

void
print_node_hex(int i, Dictionary dic)
{
  unsigned int* pe;
  Dawg_edge e = dic->dawg[i];
  pe = (unsigned int*)&e;
  printf("0x%s %s |%2d ptr=%2d t=%d l=%d f=%d chr=%d (%c)\n",
	 offset(&(dic->dawg[0]),&(dic->dawg[i])),hexl(*pe),i,
	 e.ptr, e.term, e.last, e.fill, e.chr, e.chr +'a' -1);
}

void
print_dic_hex(char* filename)
{
  int i;
  Dictionary dic;
  dic_load(&dic,filename);

  printf("offs binary       structure         \n");
  printf("---- -------- |   ------------------\n");
  for(i=0; i < (dic->nedges + 1); i++)
    print_node_hex(i,dic);
  Dic_destroy(dic);
}

void
usage(char* name)
{
  printf("usage: %s [-a|-d|-h|-l] dictionnaire\n", name);
  printf("  -a : print all\n");
  printf("  -h : print header\n");
  printf("  -d : print dic in hex\n");
  printf("  -l : print dic word list\n");
}


int
main(int argc, char *argv[])
{
  int arg_count;
  int option_print_all      = 0;
  int option_print_header   = 0;
  int option_print_dic_hex  = 0;
  int option_print_dic_list = 0;

  if (argc < 2)
    {
      usage(argv[0]);
      exit(1);
    }

  arg_count = 1;
  while(argv[arg_count][0] == '-')
    {
      switch (argv[arg_count][1])
	{
	case 'a': option_print_all = 1; break;
	case 'h': option_print_header = 1; break;
	case 'd': option_print_dic_hex = 1; break;
	case 'l': option_print_dic_list = 1; break;
	default: usage(argv[0]); exit(2);
	  break;
	}
      arg_count++;
    }

  if (option_print_header || option_print_all)
    {
      print_header(argv[arg_count]);
    }
  if (option_print_dic_hex || option_print_all)
    {
      print_dic_hex(argv[arg_count]);
    }
  if (option_print_dic_list || option_print_all)
    {
      print_dic_list(argv[arg_count],"stdout");
    }
  return 0;
}
