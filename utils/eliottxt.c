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

/* $Id: eliottxt.c,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dic.h"
#include "dic_search.h"
#include "game.h"
#include "gameio.h"

void 
aide()
{
    printf("  ?    : aide -- cette page\n");
    printf("  a [g|l|p|r|t] : afficher\n");
    printf("                g -- grille\n");
    printf("                gj -- grille + jokers\n");
    printf("                l -- lettres non jouées\n");
    printf("                p -- partie\n");
    printf("                r -- recherche\n");
    printf("                t -- tirage\n");
    printf("  c [] : charger la partie []\n");
    printf("  d [] : vérifier le mot []\n");
    printf("  j [] : jouer le résultat []\n");
    printf("  r    : rechercher à partir du tirage\n");
    printf("  s [] : sauver la partie []\n"); 
    printf("  t [] : changer le tirage\n");
    printf("  x [] : evaluer l'esxpression regulières\n");
    printf("  *    : tirage aleatoire\n");
    printf("  +    : tirage aleatoire ajouts\n");
    printf("  q    : quitter\n");
}

char *
next_token_alpha(char *cmd, const char *delim)
{
    int i;
    char *token = strtok(cmd, delim);
    if (token == NULL)
         return NULL;
    for (i = 0; token[i] && isalpha(token[i]); i++)
         ;
    token[i] = '\0';
    return token;
}

char *
next_token_alphaplusjoker(char *cmd, const char *delim)
{
    int i;
    char *token = strtok(cmd, delim);
    if (token == NULL)
         return NULL;
    for (i = 0; token[i] && (isalpha(token[i]) || 
                             token[i] == '?'   ||
                             token[i] == '+');
         i++)
         ;
    token[i] = '\0';
    return token;
}

char *
next_token_digit(char *cmd, const char *delim)
{
    int i;
    char *token = strtok(cmd, delim);
    if (token == NULL)
         return NULL;
    for (i = 0; token[i] && (isdigit(token[i]) || token[i] == '-'); i++)
         ;
    token[i] = '\0';
    return token;
}

char *
next_token_cross(char *cmd, const char *delim)
{
  int i;
  char *token = strtok(cmd, delim);
  if (token == NULL)
    return NULL;
  for(i=0; token[i] && 
	(isalpha(token[i]) ||
	 token[i] == '.');
      i++)
    ;
  token[i] = '\0';
  return token;
}

void
eliottxt_get_cross(Dictionary dic, char* cros)
{
  int i;
  //  (Dictionary dic, char* regx, char wordlist[RES_REGX_MAX][DIC_WORD_MAX])
  char wordlist[RES_CROS_MAX][DIC_WORD_MAX];
  Dic_search_Cros(dic,cros,wordlist);
  for(i=0; i<RES_CROS_MAX && wordlist[i][0]; i++)
    {
	printf("  %s\n",wordlist[i]);
    }
}

char *
next_token_filename(char *cmd, const char *delim)
{
    int i;
    char *token = strtok(cmd, delim);
    if (token == NULL)
         return NULL;
    for (i = 0; token[i] && (isalpha(token[i]) || 
                             token[i] == '.' ||
                             token[i] == '_'); i++)
         ;
    token[i] = '\0';
    return token;
}

void 
main_loop(Game game)
{
     char *token;
     char commande[100];
     char delim[] = " \t";
     int quitter = 0;
     
     printf("[?] pour l'aide\n");
     while (quitter == 0) {
          printf("commande> ");
          fgets(commande, sizeof(commande), stdin);
          token = strtok(commande, delim);
          if (token) {
               switch (token[0]) {
               case '?':
                    aide();
                    break;
               case 'a':
                    token = next_token_alpha(NULL, delim);
                    if (token == NULL) {
                         aide();
                         break;
                    }
                    switch (token[0]) {
                    case 'g':
		      switch (token[1]) {
		      case '\0':
			Game_print_board(stdout,game);
			break;
		      case 'j':
			Game_print_board_joker(stdout,game);
			break;
		      case 'p':
			Game_print_board_point(stdout,game);
			break;
		      default:
			fprintf(stderr,"commande inconnue\n");
			break;
		      }
		      break;
                    case 'p':
                         Game_print_game(stdout,game);
                         break;
                    case 'l':
                         Game_print_nonplayed(stdout,game);
                         break;
                    case 'r':
                         token = next_token_digit(NULL, delim);
                         if (token == NULL) {
                              Game_print_searchresults(stdout,game,10);
                         } else {
                              Game_print_searchresults(stdout,game,atoi(token));
                         }
                         break;
                    case 't':
                         Game_print_playedrack(stdout,game,Game_getnrounds(game));
                         break;
                    default:
                         printf("commande inconnue\n");
                         break;
                    }
                    break;
	       case 'c':
		    token = next_token_filename(NULL,delim);
		    if (token == NULL) {
		    } else {
			 FILE* fin;
			 fprintf(stderr,"chargement de -%s-\n",token);
			 if ((fin = fopen(token,"r")) == NULL) {
			      printf("impossible d'ouvrir %s\n",token);
			      break;
			 }
			 Game_init(game);
			 switch (Game_load(game,fin)) {
			 case 0: /* ok */
			   break;
			 case 1:
			   fprintf(stderr,"format non reconnu\n");
			   break;
			 default:
			   fprintf(stderr,"erreur pendant le chargement\n");
			   break;
			 }
			 fclose(fin);
		    }
		    break;
               case 'd':
                    token = next_token_alpha(NULL, delim);
                    if (token == NULL)
                         aide();
                    else {
                         if (Dic_search_word(Game_getdic(game),token))
                              printf("le mot -%s- existe\n", token);
                         else
                              printf("le mot -%s- n'existe pas\n", token);
                    }
                    break;
               case 'j':
                    token = next_token_digit(NULL,delim);
                    if (token == NULL) {
                         aide();
                    } else {
                         int n = atoi(token);
                         if (n <= 0) {
                              Game_back(game, n==0 ? 1 : -n);
                         } else {
                              if (Game_play(game, --n))
                                   printf("mauvais argument\n");
                         }
                    }
                    break;
               case 'r':
                    Game_search(game);
                    break;
               case 's':
                    token = next_token_filename(NULL,delim);
                    if (token == NULL) {
                    } else {
                         FILE *out;
                         if ((out = fopen(token,"w"))==NULL) {
                              printf("impossible d'ouvrir %s\n",token);
                              break;
                         }
                         Game_save(game,out);
                         fclose(out);                         
                    }
                    break;
               case 't':
                    token = next_token_alphaplusjoker(NULL, delim);
                    if (token == NULL)
                         aide();
                    else 
                         if (Game_setrack_manual(game,1,token))
                              printf("le sac ne contient pas assez de lettres\n");
                    break;
	       case 'x':
		 token = next_token_cross(NULL,delim);
		 if (token == NULL)
		   aide();
		 else
		   eliottxt_get_cross(Game_getdic(game),token);
		 break;
	       case '*':
		    Game_setrack_random(game,1,RACK_ALL);
		    break;
	       case '+':
		    Game_setrack_random(game,1,RACK_NEW);
		    break;
               case 'q':
                    quitter = 1;
                    break;
               default:
                    printf("commande inconnue\n");
                    break;
               }
          }
     }
}

int 
main(int argc, char *argv[])
{
  char dic_path[100];

  Game game = NULL;
  Dictionary dic = NULL;
  
  srand(time(NULL));
  
  if (argc < 2) {
    strcpy(dic_path,"/home/antoine/prog/scrabble/ods3/ods3.dawg");
  } else {
    strcpy(dic_path,argv[1]);
  }
          
     switch (Dic_load(&dic, dic_path)) {
         case 0:
           /* cas normal */
           break;
         case 1: 
           printf("chargement: problème d'ouverture de %s\n",argv[1]);
           exit(1);
           break;
         case 2:
           printf("chargement: mauvais en-tete de dictionnaire\n");
           exit(2);
           break;
         case 3:
           printf("chargement: problème 3 d'allocation mémoire\n");
           exit(3);
           break;
         case 4:
           printf("chargement: problème 4 d'alocation mémoire\n");
           exit(4);
           break;
         case 5:
           printf("chargement: problème de lecture des arcs du dictionnaire\n");
           exit(5);
           break;
         default:
           printf("chargement: problème non-repertorié\n");
           exit(6);
           break;
     }
     
     game = Game_create(dic);
     main_loop(game);
     
     Game_destroy(game);
     Dic_destroy(dic);
     return 0;
}
