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

/* $Id: gameio.c,v 1.2 2004/08/07 18:10:42 ipkiss Exp $ */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


#include "dic.h"
#include "tiles.h"
#include "bag.h"
#include "rack.h"
#include "round.h"
#include "pldrack.h"
#include "results.h"
#include "board.h"
#include "board_internals.h"
#include "player.h"

#include "game.h"
#include "gameio.h"
#include "game_internals.h"

#include "debug.h"

void
Game_print_board(FILE* out, Game game)
{
     char l;
     int row,column;

     fprintf(out,"   ");
     for (column = BOARD_MIN; column <= BOARD_MAX; column++)
          fprintf(out," %2d", column - BOARD_MIN + 1);
     fprintf(out,"\n");
     for (row = BOARD_MIN; row <= BOARD_MAX; row++) {
          fprintf(out," %c ", row - BOARD_MIN + 'A');
          for (column = BOARD_MIN; column <= BOARD_MAX; column++) {
               l = Game_getboardchar(game,row,column);
               fprintf(out,"  %c",l ? l : '-');
          }
          fprintf(out,"\n");
     }
}

void
Game_print_board_joker(FILE* out, Game game)
{
     char l;
     int j;
     int row,column;

     fprintf(out,"   ");
     for (column = BOARD_MIN; column <= BOARD_MAX; column++)
          fprintf(out," %2d", column - BOARD_MIN + 1);
     fprintf(out,"\n");

     for (row = BOARD_MIN; row <= BOARD_MAX; row++) {
          fprintf(out," %c ", row - BOARD_MIN + 'A');
          for (column = BOARD_MIN; column <= BOARD_MAX; column++) {
               l = Game_getboardchar(game,row,column);
               j = Board_joker(game->board,row,column);

               fprintf(out," %c",j ? '.' : (l?' ':'-'));
               fprintf(out,"%c",l ? l : '-');
          }
          fprintf(out,"\n");
     }
}

void
Game_print_board_point(FILE* out, Game game)
{
     char l;
     int p1,p2;
     int row,column;

     fprintf(out,"   ");
     for (column = BOARD_MIN; column <= BOARD_MAX; column++)
          fprintf(out," %2d", column - BOARD_MIN + 1);
     fprintf(out,"\n");

     for (row = BOARD_MIN; row <= BOARD_MAX; row++) {
          fprintf(out," %c ", row - BOARD_MIN + 'A');
          for (column = BOARD_MIN; column <= BOARD_MAX; column++) {
               l = Game_getboardchar(game,row,column);
               p1 = game->board->point_r[row][column];
               p2 = game->board->point_c[column][row];

               if (p1 > 0 && p2 > 0)
                 fprintf(out," %2d",p1 + p2);
               else if (p1 > 0)
                 fprintf(out," %2d",p1);
               else if (p2 > 0)
                 fprintf(out," %2d",p2);
               else if (l)
                 fprintf(out,"  %c",l);
               else
                 fprintf(out," --");
          }
          fprintf(out,"\n");
     }
}


void
Game_print_board_multipliers(FILE* out, Game game)
{
    char l;
    int tm, wm;
    int row, column;

    fprintf(out, "   ");
    for (column = BOARD_MIN; column <= BOARD_MAX; column++)
        fprintf(out," %2d", column - BOARD_MIN + 1);
    fprintf(out, "\n");

    for (row = BOARD_MIN; row <= BOARD_MAX; row++)
    {
        fprintf(out," %c ", row - BOARD_MIN + 'A');
        for (column = BOARD_MIN; column <= BOARD_MAX; column++)
        {
            l = Game_getboardchar(game, row, column);
            if (l != 0)
                fprintf(out, "  %c", l);
            else
            {
                wm = Board_word_multipliers[row][column];
                tm = Board_tile_multipliers[row][column];

                if (wm > 1)
                    fprintf(out, "  %c", (wm == 3) ? '@' : '#');
                else if (tm > 1)
                    fprintf(out, "  %c", (tm == 3) ? '*' : '+');
                else
                    fprintf(out, "  -");
            }
        }
        fprintf(out,"\n");
    }
}


void
Game_print_board_multipliers2(FILE* out, Game game)
{
    char l;
    int tm, wm;
    int row, column;

    fprintf(out, "   ");
    for (column = BOARD_MIN; column <= BOARD_MAX; column++)
        fprintf(out," %2d", column - BOARD_MIN + 1);
    fprintf(out, "\n");

    for (row = BOARD_MIN; row <= BOARD_MAX; row++)
    {
        fprintf(out," %c ", row - BOARD_MIN + 'A');
        for (column = BOARD_MIN; column <= BOARD_MAX; column++)
        {
            l = Game_getboardchar(game, row, column);
            wm = Board_word_multipliers[row][column];
            tm = Board_tile_multipliers[row][column];

            if (wm > 1)
                fprintf(out, " %c", (wm == 3) ? '@' : '#');
            else if (tm > 1)
                fprintf(out, " %c", (tm == 3) ? '*' : '+');
            else
                fprintf(out, " -");
            fprintf(out, "%c", l ? l : '-');
        }
        fprintf(out,"\n");
    }
}


void
Game_print_nonplayed(FILE* out, Game game)
{
     int i;
     for (i = 'a'; i <= 'z'; i++) {
          if (Game_getcharinbag(game,i) > 9)
               fprintf(out," ");
          fprintf(out," %c", i);
     }
     fprintf(out," %s?\n",(Game_getcharinbag(game,'?') > 9)? " " : "");
     for (i = 'a'; i <= 'z'; i++) {
          fprintf(out," %d", Game_getcharinbag(game,i));
     }
     fprintf(out," %d\n",Game_getcharinbag(game,'?'));
}

static int
print_lineplayedrack(FILE* out, Game game, int n)
{
    char buff[RACK_SIZE_MAX];
    Game_getplayedrack(game, n, buff);
    fprintf(out,"%s", buff);
    return strlen(buff);
}


void
Game_print_playedrack(FILE* out, Game game, int n)
{
    if (print_lineplayedrack(out, game, n))
        fprintf(out, "\n");
}


void Game_print_allracks(FILE* out, Game game)
{
    Playedrack pld;
    tile_t tiles[RACK_MAX];
    int i, j;
    for (j = 0; j < game->nplayers; j++)
    {
        fprintf(out, "Joueur %d: ", j);
        pld = Player_getplayedrack(game->players[j]);
        Playedrack_getalltiles(pld, tiles);
        for (i = 0; i < Playedrack_ntiles(pld); i++)
        {
            fprintf(out, "%c", codetochar(tiles[i]));
        }
        fprintf(out, "\n");
    }
}


static void
whitespace(char* b,int n)
{
     int i;
     for(i=0; i < n; i++)
          b[i] = ' ';
     b[i] = '\0';
}

#define SEARCH_RESULT_LINE_SIZE_MAX 50

static void
Game_searchresultline(Game game, int num, char line[SEARCH_RESULT_LINE_SIZE_MAX])
{
     char word  [WORD_SIZE_MAX];
     char white [20];
     char first [COOR_SIZE_MAX];
     char second[COOR_SIZE_MAX];

     Game_getsearchedword(game,num,word);
     if (!word[0])
          return;
     whitespace(white,16 - strlen(word));
     Game_getsearchedfirstcoord(game,num,first);
     Game_getsearchedsecondcoord(game,num,second);
     sprintf(line,"%s%s%c%4d %s %s",
             word,
             white,
             Game_getsearchedbonus(game,num) ? '*' : ' ',
             Game_getsearchedpoints(game,num),
             first,second);
     return;
}

void
Game_print_searchresults(FILE* out, Game game, int num)
{
     int i;
     char line[SEARCH_RESULT_LINE_SIZE_MAX];
     for (i = 0; i < num && i < Game_getnresults(game); i++) {
          Game_searchresultline(game,i,line);
          fprintf(out,"%3d: %s", i + 1,line);
          fprintf(out,"\n");
     }
}


void Game_print_points(FILE* out, Game game)
{
    fprintf(out, "%d\n", Game_getplayerpoints(game, 0));
}


void Game_print_allpoints(FILE* out, Game game)
{
    int i;
    for (i = 0; i < game->nplayers; i++)
    {
        fprintf(out, "Joueur %d: %4d\n", i,
                Player_getpoints(game->players[i]));
    }
}


static int
print_lineplayedround(FILE* out, Game game, int num)
{
     char buff  [20];
     char word  [WORD_SIZE_MAX];
     char first [COOR_SIZE_MAX];
     char second[COOR_SIZE_MAX];

     Game_getplayedword(game,num,word);
     if (!word[0])
          return 0;
     fprintf(out,"%s",word);
     whitespace(buff,16 - strlen(word));
     fprintf(out,"%s",buff);
     fprintf(out,"%c",Game_getplayedbonus(game,num) ? '*' : ' ');
     fprintf(out,"%4d",Game_getplayedpoints(game,num));
     Game_getplayedfirstcoord(game,num,first);
     Game_getplayedsecondcoord(game,num,second);
     fprintf(out," %2s %2s",first,second);
     return 1;
}


void
Game_print_game(FILE* out, Game game)
{
    int i, l;
    char buff[100];
    char decal[] = "   ";

    fprintf(out, "%s\n\n", IDENT_STRING);

    for(i = 0; i < Game_getnrounds(game); i++)
    {
        fprintf(out, "%s",decal);

        l = print_lineplayedrack(out, game, i);
        whitespace(buff, 12 - l);
        fprintf(out,"%s", buff);
        print_lineplayedround(out, game, i);
        fprintf(out, "\n");
    }
    fprintf(out, "%s", decal);
    Game_print_playedrack(out, game, Game_getnrounds(game));
    whitespace(buff, 24);

    fprintf(out, "\n%stotal%s%4d\n\n", decal, buff, game->points);
}


int
Game_read_game(FILE* fin, Game game)
{
     int  row,col;
     tile_t tile;
     char buff[4096];
     char word[WORD_SIZE_MAX];
     char delim[]=" \t\n";
     char *token,*wordptr;

     if (game->dic == NULL)
          return 1;

     Game_init(game);

     if (fgets(buff,sizeof(buff),fin) == NULL)
         return 1;

     if ((token = strtok(buff,delim)) == NULL)
         return 1;

     if (strcmp(buff,IDENT_STRING) != 0)
         return 1;

     while(fgets(buff,sizeof(buff),fin))
       {
         token = strtok(buff,delim);
         if (token != NULL)
           {
             if (strcmp(token,"total")==0)
               {
                 break;
               }

             /*
              * rack
              */

             while(*token && *token!='+')
               {
                 Playedrack_addold(game->playedracks[game->nrounds],chartocode(*token));
                 token++;
               }

             if (*token == '+')
               token++;

             while(*token)
               {
                 Playedrack_addnew(game->playedracks[game->nrounds],chartocode(*token));
                 token++;
               }

             /*
              * word
              */

             token = strtok(NULL,delim);

             if (token == NULL)
               break;

             strcpy(word,token);
             wordptr = word;

             /*
              * bonus
              */

             token = strtok(NULL,delim);
             if (token[0]=='*')
               {
                 Round_setbonus(game->playedrounds[game->nrounds],1);
                 token = strtok(NULL,delim);
               }

             /*
              * points
              */

             game->points += atoi(token);
             Round_setpoints(game->playedrounds[game->nrounds],atoi(token));

             /*
              * pos
              */

             token = strtok(NULL,delim);
             if (isalpha(token[0]))
               {
                 /* horizontal word */
                 Round_setdir(game->playedrounds[game->nrounds],HORIZONTAL);
                 row = token[0] - 'A' + 1;
                 Round_setrow(game->playedrounds[game->nrounds],row);
                 token = strtok(NULL,delim);
                 col = atoi(token);
                 Round_setcolumn(game->playedrounds[game->nrounds],col);

                 while(*wordptr)
                   {
                     tile = chartocode(*wordptr);

                     if (Board_tile(game->board,row,col))
                       {
                         Round_addrightfromboard(game->playedrounds[game->nrounds],tile);
                       }
                     else
                       {
                         Round_addrightfromrack(game->playedrounds[game->nrounds],tile,islower(*wordptr));
                         Bag_taketile(game->bag,(islower(*wordptr))?JOKER_TILE:tile);
                       }
                     wordptr++;
                     col++;
                   }
               }
             else /* isalpha[token[0]] */
               {
                 Round_setdir(game->playedrounds[game->nrounds],VERTICAL);
                 col = atoi(token);
                 Round_setcolumn(game->playedrounds[game->nrounds],col);
                 token = strtok(NULL,delim);
                 row = token[0] - 'A' + 1;
                 Round_setrow(game->playedrounds[game->nrounds],row);
                 while(*wordptr)
                   {
                     tile = chartocode(*wordptr);

                     if (Board_tile(game->board,row,col))
                       {
                         Round_addrightfromboard(game->playedrounds[game->nrounds],tile);
                       }
                     else
                       {
                         Round_addrightfromrack(game->playedrounds[game->nrounds],tile,islower(*wordptr));
                         Bag_taketile(game->bag,(islower(*wordptr))?JOKER_TILE:tile);
                       }
                     wordptr++;
                     row++;
                   }
               }
             Board_addround(game->dic,game->board,game->playedrounds[game->nrounds]);
             game->nrounds++;
          }
     }
     return 0;
}

