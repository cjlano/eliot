/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: eliottxt.cpp,v 1.1 2005/02/05 11:14:56 ipkiss Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <fstream>

#include "dic.h"
#include "dic_search.h"
#include "training.h"
#include "duplicate.h"
#include "freegame.h"


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
    for (i = 0; token[i] && (isalnum(token[i]) ||
                             token[i] == '.' ||
                             token[i] == '_'); i++)
        ;
    token[i] = '\0';
    return token;
}


void
eliottxt_get_cross(const Dictionary &iDic, char* cros)
{
    int i;
    //  (Dictionary dic, char* regx, char wordlist[RES_REGX_MAX][DIC_WORD_MAX])
    char wordlist[RES_CROS_MAX][DIC_WORD_MAX];
    Dic_search_Cros(iDic, cros, wordlist);
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
    printf("  s [] : sauver la partie en cours dans le fichier []\n");
    printf("  c [] : charger la partie du fichier []\n");
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
    printf("  s [] : sauver la partie en cours dans le fichier []\n");
    printf("  c [] : charger la partie du fichier []\n");
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
    printf("  n [] : passer au joueur n°[]\n");
    printf("  s [] : sauver la partie en cours dans le fichier []\n");
    printf("  c [] : charger la partie du fichier []\n");
    printf("  q    : quitter le mode duplicate\n");
}


void
help()
{
    printf("  ?       : aide -- cette page\n");
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
display_data(const Game &iGame, const char *delim)
{
    char *token;

    token = next_token_alpha(NULL, delim);
    if (token == NULL)
    {
        cout << "commande incomplète\n";
        return;
    }
    switch (token[0])
    {
        case 'g':
            switch (token[1])
            {
                case '\0':
                    iGame.printBoard(cout);
                    break;
                case 'j':
                    iGame.printBoardJoker(cout);
                    break;
/*                 case 'p': */
/*                     iGame.printBoardPoint(cout); */
/*                     break; */
                case 'm':
                    iGame.printBoardMultipliers(cout);
                    break;
                case 'n':
                    iGame.printBoardMultipliers2(cout);
                    break;
                default:
                    printf("commande inconnue\n");
                    break;
            }
            break;
        case 'j':
            cout << "Joueur " << iGame.currPlayer() << endl;
            break;
        case 'l':
            iGame.printNonPlayed(cout);
            break;
        case 'p':
            iGame.save(cout);
            break;
        case 'r':
            token = next_token_digit(NULL, delim);
            if (token == NULL)
                iGame.printSearchResults(cout, 10);
            else
                iGame.printSearchResults(cout, atoi(token));
            break;
        case 's':
            iGame.printPoints(cout);
            break;
        case 'S':
            iGame.printAllPoints(cout);
            break;
        case 't':
            iGame.printPlayedRack(cout, iGame.getNRounds());
            break;
        case 'T':
            iGame.printAllRacks(cout);
            break;
        default:
            cout << "commande inconnue\n";
            break;
    }
}


void
loop_training(Training &iGame)
{
    char *token;
    char commande[100];
    char delim[] = " \t";
    int quit = 0;

    cout << "mode entraînement\n";
    cout << "[?] pour l'aide\n";
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
                    display_data(iGame, delim);
                    break;
                case 'd':
                    token = next_token_alpha(NULL, delim);
                    if (token == NULL)
                        help_training();
                    else
                    {
                        if (Dic_search_word(iGame.getDic(), token))
                            printf("le mot -%s- existe\n", token);
                        else
                            printf("le mot -%s- n'existe pas\n", token);
                    }
                    break;
               case 'j':
                    token = next_token_alpha(NULL, delim);
                    if (token == NULL)
                        help_training();
                    else
                    {
                        int res;
                        char *coord = next_token_alphanum(NULL, delim);
                        if (coord == NULL)
                        {
                            help_training();
                            break;
                        }
                        if ((res = iGame.play(coord, token)) != 0)
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
                            iGame.back(n == 0 ? 1 : -n);
                        else
                        {
                            if (iGame.playResult(--n))
                                printf("mauvais argument\n");
                        }
                    }
                    break;
                case 'r':
                    iGame.search();
                    break;
                case 't':
                    token = next_token_alphaplusjoker(NULL, delim);
                    if (token == NULL)
                        help_training();
                    else
                        if (iGame.setRackManual(0, token))
                            printf("le sac ne contient pas assez de lettres\n");
                    break;
                case 'x':
                    token = next_token_cross(NULL, delim);
                    if (token == NULL)
                        help_training();
                    else
                        eliottxt_get_cross(iGame.getDic(), token);
                    break;
                case '*':
                    iGame.setRackRandom(0, false, Game::RACK_ALL);
                    break;
                case '+':
                    iGame.setRackRandom(0, false, Game::RACK_NEW);
                    break;
                case 's':
                    token = next_token_filename(NULL, delim);
                    if (token != NULL)
                    {
                        ofstream fout(token);
                        if (fout.rdstate() == ios::failbit)
                        {
                            printf("impossible d'ouvrir %s\n", token);
                            break;
                        }
                        iGame.save(fout);
                        fout.close();
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
    printf("fin du mode entraînement\n");
}


void
loop_freegame(FreeGame &iGame)
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
                    display_data(iGame, delim);
                    break;
                case 'd':
                    token = next_token_alpha(NULL, delim);
                    if (token == NULL)
                        help_freegame();
                    else
                    {
                        if (Dic_search_word(iGame.getDic(), token))
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
                        if ((res = iGame.play(coord, token)) != 0)
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

                    if (iGame.pass(token, iGame.currPlayer()) != 0)
                        break;
                    break;
                case 's':
                    token = next_token_filename(NULL, delim);
                    if (token != NULL)
                    {
                        ofstream fout(token);
                        if (fout.rdstate() == ios::failbit)
                        {
                            printf("impossible d'ouvrir %s\n", token);
                            break;
                        }
                        iGame.save(fout);
                        fout.close();
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
    printf("fin du mode partie libre\n");
}


void
loop_duplicate(Duplicate &iGame)
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
                    display_data(iGame, delim);
                    break;
                case 'd':
                    token = next_token_alpha(NULL, delim);
                    if (token == NULL)
                        help_duplicate();
                    else
                    {
                        if (Dic_search_word(iGame.getDic(), token))
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
                        if ((res = iGame.play(coord, token)) != 0)
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
                        help_duplicate();
                    else
                    {
                        int res = iGame.setPlayer(atoi(token));
                        if (res == 1)
                            fprintf(stderr, "Numéro de joueur invalide\n");
                        else if (res == 2)
                            fprintf(stderr, "Impossible de choisir un joueur non humain\n");
                    }
                    break;
                case 's':
                    token = next_token_filename(NULL, delim);
                    if (token != NULL)
                    {
                        ofstream fout(token);
                        if (fout.rdstate() == ios::failbit)
                        {
                            printf("impossible d'ouvrir %s\n", token);
                            break;
                        }
                        iGame.save(fout);
                        fout.close();
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
main_loop(const Dictionary &iDic)
{
    char *token;
    char commande[100];
    char delim[] = " \t";
    int quit = 0;

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
                        Game *game = Game::load(fin, iDic);
                        fclose(fin);
                        if (game == NULL)
                        {
                            fprintf(stderr, "erreur pendant le chargement\n");
                        }
                        else
                        {
                            if (game->getMode() == Game::kTRAINING)
                                loop_training((Training&)*game);
                            else if (game->getMode() == Game::kFREEGAME)
                                loop_freegame((FreeGame&)*game);
                            else
                                loop_duplicate((Duplicate&)*game);
                        }
                    }
                    break;
                case 'e':
                {
                    /* New training game */
                    Training game(iDic);
                    game.start();
                    loop_training(game);
                    break;
                }
                case 'd':
                {
                    int i;
                    /* New duplicate game */
                    token = next_token_digit(NULL, delim);
                    if (token == NULL)
                    {
                        help();
                        break;
                    }
                    Duplicate game(iDic);
                    for (i = 0; i < atoi(token); i++)
                        game.addHumanPlayer();
                    token = next_token_digit(NULL, delim);
                    if (token == NULL)
                    {
                        help();
                        break;
                    }
                    for (i = 0; i < atoi(token); i++)
                        game.addAIPlayer();
                    game.start();
                    loop_duplicate(game);
                    break;
                }
                case 'l':
                {
                    int i;
                    /* New free game */
                    token = next_token_digit(NULL, delim);
                    if (token == NULL)
                    {
                        help();
                        break;
                    }
                    FreeGame game(iDic);
                    for (i = 0; i < atoi(token); i++)
                        game.addHumanPlayer();
                    token = next_token_digit(NULL, delim);
                    if (token == NULL)
                    {
                        help();
                        break;
                    }
                    for (i = 0; i < atoi(token); i++)
                        game.addAIPlayer();
                    game.start();
                    loop_freegame(game);
                    break;
                }
                case 'D':
                {
                    /* New duplicate game */
                    Duplicate game(iDic);
                    game.addHumanPlayer();
                    game.addAIPlayer();
                    game.start();
                    loop_duplicate(game);
                    break;
                }
                case 'L':
                {
                    /* New free game */
                    FreeGame game(iDic);
                    game.addHumanPlayer();
                    game.addAIPlayer();
                    game.start();
                    loop_freegame(game);
                    break;
                }
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

    main_loop(dic);

    Dic_destroy(dic);
    return 0;
}
