/* Eliot                                                                     */
/* Copyright (C) 1999-2004 Eliot                                             */
/* Antoine Fraboulet <antoine.fraboulet@free.fr>                             */
/* Olivier Teuliere  <ipkiss@via.ecp.fr>                                     */
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

/* $Id: eliottxt.c,v 1.2 2004/08/07 18:10:42 ipkiss Exp $ */

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

#if 0
// test
#include "tiles.h"
#include "bag.h"
#include "rack.h"
#include "round.h"
#include "pldrack.h"
#include "results.h"
#include "board.h"
#include "player.h"
#include "game_internals.h"

void
test2(Game g, char *coord, char *word, int expected)
{
    int res;
    res = Game_freegame_play(g, coord, word);
    fprintf(stderr, "res (%d) = %d    score: %3d%3d\n",
            expected, res, Game_getplayerpoints(g, 0),
            Game_getplayerpoints(g, 1));
}


void
test(Game g)
{
    int res;
    Game_addhumanplayer(g);
    Game_addhumanplayer(g);
    Game_nextplayer(g);
    Game_training_setrackmanual(g, 0, "AVNZERI");
    test2(g, "Z2", "RAVINEZ", 2);
    test2(g, "H0", "RAVINEZ", 2);
    test2(g, "H51", "RAVINEZ", 2);
    test2(g, "HH", "RAVINEZ", 2);
/*     test2(g, "8H", "RAVINEZ", 2); */
    test2(g, "H5Z2hello world", "RAVINZ", 2); // XXX
    test2(g, "H5", "RAVINZ", 3);
    test2(g, "H10", "RAVINEZ", 6); // XXX
    test2(g, "H5", "RAVIVEZ", 4);
    test2(g, "H5", "RAVINEZ", 0);

    Game_nextplayer(g);
    Game_training_setrackmanual(g, 0, "AEI?NRS");
    test2(g, "H5", "RAvINE", 7);
    test2(g, "A1", "SIgNERA", 8);
    test2(g, "H4", "SIgNERAI", 5);
    test2(g, "9E", "SIgNERAI", 4);
    test2(g, "I1", "SIgNERA", 6);
    test2(g, "8A", "SIgNERAI", 0);

    Game_nextplayer(g);
    Game_training_setrackmanual(g, 0, "ACIORU?");
    test2(g, "I1", "COURAIS", 4);
    test2(g, "I2", "COURAIs", 6);
    test2(g, "10F", "COeUR", 0);
    Game_nextplayer(g);
    test2(g, "I1", "COURAIs", 0);
}
#endif

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
next_token_alphanum(char *cmd, const char *delim)
{
    int i;
    char *token = strtok(cmd, delim);
    if (token == NULL)
        return NULL;
    for (i = 0; token[i] && isalnum(token[i]); i++)
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
    for (i = 0; token[i] &&
         (isalpha(token[i]) || token[i] == '.');
         i++)
        ;
    token[i] = '\0';
    return token;
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
eliottxt_get_cross(Dictionary dic, char* cros)
{
    int i;
    //  (Dictionary dic, char* regx, char wordlist[RES_REGX_MAX][DIC_WORD_MAX])
    char wordlist[RES_CROS_MAX][DIC_WORD_MAX];
    Dic_search_Cros(dic, cros, wordlist);
    for (i = 0; i<RES_CROS_MAX && wordlist[i][0]; i++)
    {
        printf("  %s\n", wordlist[i]);
    }
}


void
help_training()
{
    printf("  ?    : aide -- cette page\n");
    printf("  a [g|l|p|r|t] : afficher :\n");
    printf("            g -- grille\n");
    printf("            gj -- grille + jokers\n");
    printf("            gm -- grille + valeur des cases\n");
    printf("            gn -- grille + valeur des cases (variante)\n");
    printf("            l -- lettres non jouées\n");
    printf("            p -- partie\n");
    printf("            r -- recherche\n");
    printf("            s -- score\n");
    printf("            S -- score de tous les joueurs\n");
    printf("            t -- tirage\n");
    printf("  d [] : vérifier le mot []\n");
    printf("  *    : tirage aléatoire\n");
    printf("  +    : tirage aléatoire ajouts\n");
    printf("  t [] : changer le tirage\n");
    printf("  j [] {} : jouer le mot [] aux coordonnées {}\n");
    printf("  n [] : jouer le résultat numéro []\n");
    printf("  q    : quitter le mode entraînement\n");
}


void
help_freegame()
{
    printf("  ?    : aide -- cette page\n");
    printf("  a [g|l|p|s|t] : afficher :\n");
    printf("            g -- grille\n");
    printf("            gj -- grille + jokers\n");
    printf("            gm -- grille + valeur des cases\n");
    printf("            gn -- grille + valeur des cases (variante)\n");
    printf("            j -- joueur courant\n");
    printf("            l -- lettres non jouées\n");
    printf("            p -- partie\n");
    printf("            s -- score\n");
    printf("            S -- score de tous les joueurs\n");
    printf("            t -- tirage\n");
    printf("            T -- tirage de tous les joueurs\n");
    printf("  d [] : vérifier le mot []\n");
    printf("  j [] {} : jouer le mot [] aux coordonnées {}\n");
    printf("  p [] : passer son tour en changeant les lettres []\n");
    printf("  q    : quitter le mode partie libre\n");
}


void
help_duplicate()
{
    printf("  ?    : aide -- cette page\n");
    printf("  a [g|l|p|s|t] : afficher :\n");
    printf("            g -- grille\n");
    printf("            gj -- grille + jokers\n");
    printf("            gm -- grille + valeur des cases\n");
    printf("            gn -- grille + valeur des cases (variante)\n");
    printf("            j -- joueur courant\n");
    printf("            l -- lettres non jouées\n");
    printf("            p -- partie\n");
    printf("            s -- score\n");
    printf("            S -- score de tous les joueurs\n");
    printf("            t -- tirage\n");
    printf("  d [] : vérifier le mot []\n");
    printf("  j [] {} : jouer le mot [] aux coordonnées {}\n");
    printf("  s [] : passer au joueur n°[]\n");
    printf("  q    : quitter le mode duplicate\n");
}


void
help()
{
    printf("  ?       : aide -- cette page\n");
    printf("  c []    : charger la partie []\n");
    printf("  s []    : sauver la partie []\n");
    printf("  e       : démarrer le mode entraînement\n");
    printf("  d [] {} : démarrer une partie duplicate avec\n");
    printf("                [] joueurs humains et {} joueurs IA\n");
    printf("  l [] {} : démarrer une partie libre avec\n");
    printf("                [] joueurs humains et {} joueurs IA\n");
    printf("  D       : raccourci pour d 1 1\n");
    printf("  L       : raccourci pour l 1 1\n");
    printf("  q       : quitter\n");
}


void
display_data(Game game, const char *delim)
{
    char *token;

    token = next_token_alpha(NULL, delim);
    if (token == NULL)
    {
        printf("commande incomplète\n");
        return;
    }
    switch (token[0])
    {
        case 'g':
            switch (token[1])
            {
                case '\0':
                    Game_print_board(stdout, game);
                    break;
                case 'j':
                    Game_print_board_joker(stdout, game);
                    break;
                case 'p':
                    Game_print_board_point(stdout, game);
                    break;
                case 'm':
                    Game_print_board_multipliers(stdout, game);
                    break;
                case 'n':
                    Game_print_board_multipliers2(stdout, game);
                    break;
                default:
                    printf("commande inconnue\n");
                    break;
            }
            break;
        case 'j':
            fprintf(stdout, "Joueur %i\n", Game_currplayer(game));
            break;
        case 'l':
            Game_print_nonplayed(stdout, game);
            break;
        case 'p':
            Game_print_game(stdout, game);
            break;
        case 'r':
            token = next_token_digit(NULL, delim);
            if (token == NULL)
                Game_print_searchresults(stdout, game, 10);
            else
                Game_print_searchresults(stdout, game, atoi(token));
            break;
        case 's':
            Game_print_points(stdout, game);
            break;
        case 'S':
            Game_print_allpoints(stdout, game);
            break;
        case 't':
            Game_print_playedrack(stdout, game, Game_getnrounds(game));
            break;
        case 'T':
            Game_print_allracks(stdout, game);
            break;
        default:
            printf("commande inconnue\n");
            break;
    }
}


void
loop_training(Game game)
{
    char *token;
    char commande[100];
    char delim[] = " \t";
    int quit = 0;

    printf("mode entraînement\n");
    printf("[?] pour l'aide\n");
    while (quit == 0)
    {
        printf("commande> ");
        fgets(commande, sizeof(commande), stdin);
        token = strtok(commande, delim);
        if (token)
        {
            switch (token[0])
            {
                case '?':
                    help_training();
                    break;
                case 'a':
                    display_data(game, delim);
                    break;
                case 'd':
                    token = next_token_alpha(NULL, delim);
                    if (token == NULL)
                        help_training();
                    else
                    {
                        if (Dic_search_word(Game_getdic(game), token))
                            printf("le mot -%s- existe\n", token);
                        else
                            printf("le mot -%s- n'existe pas\n", token);
                    }
                    break;
               case 'j':
                    token = next_token_alpha(NULL, delim);
                    if (token == NULL)
                        help_duplicate();
                    else
                    {
                        int res;
                        char *coord = next_token_alphanum(NULL, delim);
                        if (coord == NULL)
                        {
                            help_training();
                            break;
                        }
                        if ((res = Game_training_play(game, coord, token)) != 0)
                        {
                            fprintf(stderr, "Mot incorrect ou mal placé (%i)\n",
                                    res);
                            break;
                        }
                    }
                    break;
                case 'n':
                    token = next_token_digit(NULL, delim);
                    if (token == NULL)
                        help_training();
                    else
                    {
                        int n = atoi(token);
                        if (n <= 0)
                            Game_back(game, n == 0 ? 1 : -n);
                        else
                        {
                            if (Game_training_playresult(game, --n))
                                printf("mauvais argument\n");
                        }
                    }
                    break;
                case 'r':
                    Game_training_search(game);
                    break;
                case 't':
                    token = next_token_alphaplusjoker(NULL, delim);
                    if (token == NULL)
                        help_training();
                    else
                        if (Game_training_setrackmanual(game, 0, token))
                            printf("le sac ne contient pas assez de lettres\n");
                    break;
                case 'x':
                    token = next_token_cross(NULL, delim);
                    if (token == NULL)
                        help_training();
                    else
                        eliottxt_get_cross(Game_getdic(game), token);
                    break;
                case '*':
                    Game_training_setrackrandom(game, 0, RACK_ALL);
                    break;
                case '+':
                    Game_training_setrackrandom(game, 0, RACK_NEW);
                    break;
                case 'q':
                    quit = 1;
                    break;
                default:
                    printf("commande inconnue\n");
                    break;
            }
        }
    }
    printf("fin du mode entraînement\n");
}


void
loop_freegame(Game game)
{
    char *token;
    char commande[100];
    char delim[] = " \t";
    int quit = 0;

    printf("mode partie libre\n");
    printf("[?] pour l'aide\n");
    while (quit == 0)
    {
        printf("commande> ");
        fgets(commande, sizeof(commande), stdin);
        token = strtok(commande, delim);
        if (token)
        {
            switch (token[0])
            {
                case '?':
                    help_freegame();
                    break;
                case 'a':
                    display_data(game, delim);
                    break;
                case 'd':
                    token = next_token_alpha(NULL, delim);
                    if (token == NULL)
                        help_freegame();
                    else
                    {
                        if (Dic_search_word(Game_getdic(game), token))
                            printf("le mot -%s- existe\n", token);
                        else
                            printf("le mot -%s- n'existe pas\n", token);
                    }
                    break;
               case 'j':
                    token = next_token_alpha(NULL, delim);
                    if (token == NULL)
                        help_freegame();
                    else
                    {
                        int res;
                        char *coord = next_token_alphanum(NULL, delim);
                        if (coord == NULL)
                        {
                            help_freegame();
                            break;
                        }
                        if ((res = Game_freegame_play(game, coord, token)) != 0)
                        {
                            fprintf(stderr, "Mot incorrect ou mal placé (%i)\n",
                                    res);
                            break;
                        }
                    }
                    break;
               case 'p':
                    token = next_token_alpha(NULL, delim);
                    /* You can pass your turn without changing any letter */
                    if (token == NULL)
                        token = "";

                    if (Game_freegame_pass(game, token,
                                           Game_currplayer(game)) != 0)
                        break;
                    break;
                case 'q':
                    quit = 1;
                    break;
                default:
                    printf("commande inconnue\n");
                    break;
            }
        }
    }
    printf("fin du mode partie libre\n");
}


void
loop_duplicate(Game game)
{
    char *token;
    char commande[100];
    char delim[] = " \t";
    int quit = 0;

    printf("mode duplicate\n");
    printf("[?] pour l'aide\n");
    while (quit == 0)
    {
        printf("commande> ");
        fgets(commande, sizeof(commande), stdin);
        token = strtok(commande, delim);
        if (token)
        {
            switch (token[0])
            {
                case '?':
                    help_duplicate();
                    break;
                case 'a':
                    display_data(game, delim);
                    break;
                case 'd':
                    token = next_token_alpha(NULL, delim);
                    if (token == NULL)
                        help_duplicate();
                    else
                    {
                        if (Dic_search_word(Game_getdic(game), token))
                            printf("le mot -%s- existe\n", token);
                        else
                            printf("le mot -%s- n'existe pas\n", token);
                    }
                    break;
                case 'j':
                    token = next_token_alpha(NULL, delim);
                    if (token == NULL)
                        help_duplicate();
                    else
                    {
                        int res;
                        char *coord = next_token_alphanum(NULL, delim);
                        if (coord == NULL)
                        {
                            help_duplicate();
                            break;
                        }
                        if ((res = Game_duplicate_play(game, coord, token)) != 0)
                        {
                            fprintf(stderr, "Mot incorrect ou mal placé (%i)\n",
                                    res);
                            break;
                        }
                    }
                    break;
                case 's':
                    token = next_token_digit(NULL, delim);
                    if (token == NULL)
                        help_duplicate();
                    else
                    {
                        int res = Game_duplicate_setplayer(game, atoi(token));
                        if (res == 1)
                            fprintf(stderr, "Numéro de joueur invalide\n");
                        else if (res == 2)
                            fprintf(stderr, "Impossible de choisir un joueur non humain\n");
                    }
                    break;
                case 'q':
                    quit = 1;
                    break;
                default:
                    printf("commande inconnue\n");
                    break;
            }
        }
    }
    printf("fin du mode duplicate\n");
}


void
main_loop(Game game)
{
    char *token;
    char commande[100];
    char delim[] = " \t";
    int quit = 0;

    printf("[?] pour l'aide\n");
    while (quit == 0) {
        printf("commande> ");
        fgets(commande, sizeof(commande), stdin);
        token = strtok(commande, delim);
        if (token) {
            switch (token[0])
            {
                case '?':
                    help();
                    break;
                case 'c':
                    token = next_token_filename(NULL, delim);
                    if (token == NULL)
                    {}
                    else
                    {
                        FILE* fin;
                        fprintf(stderr, "chargement de -%s-\n", token);
                        if ((fin = fopen(token, "r")) == NULL)
                        {
                            printf("impossible d'ouvrir %s\n", token);
                            break;
                        }
                        Game_init(game);
                        switch (Game_load(game, fin))
                        {
                            case 0: /* ok */
                                break;
                            case 1:
                                fprintf(stderr, "format non reconnu\n");
                                break;
                            default:
                                fprintf(stderr, "erreur pendant le chargement\n");
                                break;
                        }
                        fclose(fin);
                    }
                    break;
                case 's':
                    token = next_token_filename(NULL, delim);
                    if (token == NULL)
                    {}
                    else
                    {
                        FILE *out;
                        if ((out = fopen(token, "w")) == NULL)
                        {
                            printf("impossible d'ouvrir %s\n", token);
                            break;
                        }
                        Game_save(game, out);
                        fclose(out);
                    }
                    break;
                case 'e':
                    /* Re-init the game */
                    Game_init(game);
                    Game_training_start(game);
                    loop_training(game);
                    break;
                case 'd':
                {
                    int i;
                    /* Re-init the game */
                    Game_init(game);
                    token = next_token_digit(NULL, delim);
                    if (token == NULL)
                    {
                        help();
                        break;
                    }
                    for (i = 0; i < atoi(token); i++)
                        Game_addhumanplayer(game);
                    token = next_token_digit(NULL, delim);
                    if (token == NULL)
                    {
                        help();
                        break;
                    }
                    for (i = 0; i < atoi(token); i++)
                        Game_addaiplayer(game);
                    Game_duplicate_start(game);
                    loop_duplicate(game);
                    break;
                }
                case 'l':
                {
                    int i;
                    /* Re-init the game */
                    Game_init(game);
                    token = next_token_digit(NULL, delim);
                    if (token == NULL)
                    {
                        help();
                        break;
                    }
                    for (i = 0; i < atoi(token); i++)
                        Game_addhumanplayer(game);
                    token = next_token_digit(NULL, delim);
                    if (token == NULL)
                    {
                        help();
                        break;
                    }
                    for (i = 0; i < atoi(token); i++)
                        Game_addaiplayer(game);
                    Game_freegame_start(game);
                    loop_freegame(game);
                    break;
                }
                case 'D':
                    /* Re-init the game */
                    Game_init(game);
                    Game_addhumanplayer(game);
                    Game_addaiplayer(game);
                    Game_duplicate_start(game);
                    loop_duplicate(game);
                    break;
                case 'L':
                    /* Re-init the game */
                    Game_init(game);
                    Game_addhumanplayer(game);
                    Game_addaiplayer(game);
                    Game_freegame_start(game);
                    loop_freegame(game);
                    break;
                case 'q':
                    quit = 1;
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

    if (argc != 2)
    {
        fprintf(stdout, "Usage: eliot /chemin/vers/ods4.dawg\n");
        exit(1);
    }
    else
        strcpy(dic_path, argv[1]);

    switch (Dic_load(&dic, dic_path))
    {
        case 0:
            /* Normal case */
            break;
        case 1:
            printf("chargement: problème d'ouverture de %s\n", argv[1]);
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
