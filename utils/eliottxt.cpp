/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <locale.h>
#include <wctype.h>
#include <wchar.h>
#include <fstream>
#include <readline/readline.h>
#include <readline/history.h>

#include "dic.h"
#include "dic_search.h"
#include "regexp.h"
#include "game_io.h"
#include "game_factory.h"
#include "training.h"
#include "duplicate.h"
#include "freegame.h"
#include "encoding.h"


/* A static variable for holding the line. */
static char *line_read = NULL;
/* Wide version of the line */
static wchar_t *wline_read = NULL;

/**
 * Read a string, and return a pointer to it.
 * Returns NULL on EOF.
 */
wchar_t *rl_gets()
{
    // If the buffer has already been allocated, return the memory to the free
    // pool
    if (line_read)
    {
        free(line_read);
        line_read = NULL;
    }
    if (wline_read)
    {
        delete[] wline_read;
        wline_read = NULL;
    }

    // Get a line from the user
    line_read = readline("commande> ");

    // If the line has any text in it, save it on the history
    if (line_read && *line_read)
        add_history(line_read);

    // Convert the line into wide characters
    // Get the needed length (we _can't_ use string::size())
    size_t len = mbstowcs(NULL, line_read, 0);
    if (len == (size_t)-1)
        return NULL;

    wline_read = new wchar_t[len + 1];
    len = mbstowcs(wline_read, line_read, len + 1);

    return wline_read;
}


wchar_t * next_token_alpha(wchar_t *cmd, const wchar_t *delim, wchar_t **state)
{
    wchar_t *token = wcstok(cmd, delim, state);
    if (token == NULL)
        return NULL;
    int i;
    for (i = 0; token[i] && iswalpha(token[i]); i++)
        ;
    token[i] = L'\0';
    return token;
}


wchar_t * next_token_alphanum(wchar_t *cmd, const wchar_t *delim, wchar_t **state)
{
    wchar_t *token = wcstok(cmd, delim, state);
    if (token == NULL)
        return NULL;
    int i;
    for (i = 0; token[i] && iswalnum(token[i]); i++)
        ;
    token[i] = L'\0';
    return token;
}


wchar_t * next_token_alphaplusjoker(wchar_t *cmd, const wchar_t *delim, wchar_t **state)
{
    wchar_t *token = wcstok(cmd, delim, state);
    if (token == NULL)
        return NULL;
    int i;
    for (i = 0; token[i] && (iswalpha(token[i]) ||
                             token[i] == L'?'   ||
                             token[i] == L'+');
         i++)
        ;
    token[i] = L'\0';
    return token;
}


wchar_t * next_token_digit(wchar_t *cmd, const wchar_t *delim, wchar_t **state)
{
    wchar_t *token = wcstok(cmd, delim, state);
    if (token == NULL)
        return NULL;
    int i;
    for (i = 0; token[i] && (iswdigit(token[i]) || token[i] == L'-'); i++)
        ;
    token[i] = L'\0';
    return token;
}


wchar_t * next_token_cross(wchar_t *cmd, const wchar_t *delim, wchar_t **state)
{
    wchar_t *token = wcstok(cmd, delim, state);
    if (token == NULL)
        return NULL;
    int i;
    for (i = 0; token[i] &&
         (iswalpha(token[i]) || token[i] == L'.');
         i++)
        ;
    token[i] = L'\0';
    return token;
}


wchar_t * next_token_filename(wchar_t *cmd, const wchar_t *delim, wchar_t **state)
{
    wchar_t *token = wcstok(cmd, delim, state);
    if (token == NULL)
        return NULL;
    int i;
    for (i = 0; token[i] && (iswalnum(token[i]) ||
                             token[i] == L'.' ||
                             token[i] == L'_'); i++)
        ;
    token[i] = L'\0';
    return token;
}


void eliottxt_get_cross(const Dictionary &iDic, wchar_t *cros)
{
    wchar_t wordlist[RES_CROS_MAX][DIC_WORD_MAX];
    Dic_search_Cros(iDic, cros, wordlist);
    for (int i = 0; i < RES_CROS_MAX && wordlist[i][0]; i++)
    {
        printf("  %s\n", convertToMb(wordlist[i]).c_str());
    }
}


void help_training()
{
    printf("  ?    : aide -- cette page\n");
    printf("  a [g|l|p|r|t] : afficher :\n");
    printf("            g -- grille\n");
    printf("            gj -- grille + jokers\n");
    printf("            gm -- grille + valeur des cases\n");
    printf("            gn -- grille + valeur des cases (variante)\n");
    printf("            l -- lettres non jouées\n");
    printf("            p -- partie\n");
    printf("            P -- partie (format standard)\n");
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
    printf("  r    : rechercher les meilleurs résultats\n");
    printf("  s [] : sauver la partie en cours dans le fichier []\n");
    printf("  q    : quitter le mode entraînement\n");
}


void help_freegame()
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
    printf("            P -- partie (format standard)\n");
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


void help_duplicate()
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
    printf("            P -- partie (format standard)\n");
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


void help()
{
    printf("  ?       : aide -- cette page\n");
    printf("  e       : démarrer le mode entraînement\n");
    printf("  d [] {} : démarrer une partie duplicate avec\n");
    printf("                [] joueurs humains et {} joueurs IA\n");
    printf("  l [] {} : démarrer une partie libre avec\n");
    printf("                [] joueurs humains et {} joueurs IA\n");
    printf("  D       : raccourci pour d 1 1\n");
    printf("  L       : raccourci pour l 1 1\n");
    printf("  x [] {1} {2} {3} : expressions rationnelles\n");
    printf("          [] expression à rechercher\n");
    printf("          {1} nombre de résultats à afficher\n");
    printf("          {2} longueur minimum d'un mot\n");
    printf("          {3} longueur maximum d'un mot\n");
    printf("  q       : quitter\n");
}


void display_data(const Game &iGame, const wchar_t *delim, wchar_t **state)
{
    wchar_t *token;

    token = next_token_alpha(NULL, delim, state);
    if (token == NULL)
    {
        cout << "commande incomplète\n";
        return;
    }
    switch (token[0])
    {
        case L'g':
            switch (token[1])
            {
                case L'\0':
                    GameIO::printBoard(cout, iGame);
                    break;
                case L'j':
                    GameIO::printBoardJoker(cout, iGame);
                    break;
                case L'm':
                    GameIO::printBoardMultipliers(cout, iGame);
                    break;
                case L'n':
                    GameIO::printBoardMultipliers2(cout, iGame);
                    break;
                default:
                    printf("commande inconnue\n");
                    break;
            }
            break;
        case L'j':
            cout << "Joueur " << iGame.currPlayer() << endl;
            break;
        case L'l':
            GameIO::printNonPlayed(cout, iGame);
            break;
        case L'p':
            iGame.save(cout,Game::FILE_FORMAT_ADVANCED);
            break;
        case L'P':
            iGame.save(cout,Game::FILE_FORMAT_STANDARD);
            break;
        case L'r':
            token = next_token_digit(NULL, delim, state);
            if (token == NULL)
                GameIO::printSearchResults(cout,
                                           static_cast<const Training&>(iGame),
                                           10);
            else
                GameIO::printSearchResults(cout,
                                           static_cast<const Training&>(iGame),
                                           _wtoi(token));
            break;
        case L's':
            GameIO::printPoints(cout, iGame);
            break;
        case L'S':
            GameIO::printAllPoints(cout, iGame);
            break;
        case L't':
            GameIO::printPlayedRack(cout, iGame, iGame.getHistory().getSize());
            break;
        case L'T':
            GameIO::printAllRacks(cout, iGame);
            break;
        default:
            cout << "commande inconnue\n";
            break;
    }
}


void loop_training(Training &iGame)
{
    wchar_t *token;
    wchar_t *state;
    wchar_t *commande = NULL;
    wchar_t delim[] = L" \t";
    int quit = 0;

    cout << "mode entraînement\n";
    cout << "[?] pour l'aide\n";
    while (quit == 0)
    {
        commande = rl_gets();
        token = wcstok(commande, delim, &state);
        if (token)
        {
            switch (token[0])
            {
                case L'?':
                    help_training();
                    break;
                case L'a':
                    display_data(iGame, delim, &state);
                    break;
                case L'd':
                    token = next_token_alpha(NULL, delim, &state);
                    if (token == NULL)
                        help_training();
                    else
                    {
                        if (Dic_search_word(iGame.getDic(), token))
                        {
                            printf("le mot -%s- existe\n",
                                   convertToMb(token).c_str());
                        }
                        else
                        {
                            printf("le mot -%s- n'existe pas\n",
                                   convertToMb(token).c_str());
                        }
                    }
                    break;
               case L'j':
                    token = next_token_alpha(NULL, delim, &state);
                    if (token == NULL)
                        help_training();
                    else
                    {
                        int res;
                        wchar_t *coord = next_token_alphanum(NULL, delim, &state);
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
                case L'n':
                    token = next_token_digit(NULL, delim, &state);
                    if (token == NULL)
                        help_training();
                    else
                    {
                        int n = _wtoi(token);
                        if (n <= 0)
                            iGame.back(n == 0 ? 1 : -n);
                        else
                        {
                            if (iGame.playResult(--n))
                                printf("mauvais argument\n");
                        }
                    }
                    break;
                case L'r':
                    iGame.search();
                    break;
                case L't':
                    token = next_token_alphaplusjoker(NULL, delim, &state);
                    if (token == NULL)
                        help_training();
                    else
                        if (iGame.setRackManual(0, token))
                            printf("le sac ne contient pas assez de lettres\n");
                    break;
                case L'x':
                    token = next_token_cross(NULL, delim, &state);
                    if (token == NULL)
                        help_training();
                    else
                        eliottxt_get_cross(iGame.getDic(), token);
                    break;
                case L'*':
                    iGame.setRackRandom(false, Game::RACK_ALL);
                    break;
                case L'+':
                    iGame.setRackRandom(false, Game::RACK_NEW);
                    break;
                case L's':
                    token = next_token_filename(NULL, delim, &state);
                    if (token != NULL)
                    {
                        string filename = convertToMb(token);
                        ofstream fout(filename.c_str());
                        if (fout.rdstate() == ios::failbit)
                        {
                            printf("impossible d'ouvrir %s\n",
                                   filename.c_str());
                            break;
                        }
                        iGame.save(fout);
                        fout.close();
                    }
                    break;
                case L'q':
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


void loop_freegame(FreeGame &iGame)
{
    wchar_t *token;
    wchar_t *state;
    wchar_t *commande = NULL;
    wchar_t delim[] = L" \t";
    int quit = 0;

    printf("mode partie libre\n");
    printf("[?] pour l'aide\n");
    while (quit == 0)
    {
        commande = rl_gets();
        token = wcstok(commande, delim, &state);
        if (token)
        {
            switch (token[0])
            {
                case L'?':
                    help_freegame();
                    break;
                case L'a':
                    display_data(iGame, delim, &state);
                    break;
                case L'd':
                    token = next_token_alpha(NULL, delim, &state);
                    if (token == NULL)
                        help_freegame();
                    else
                    {
                        if (Dic_search_word(iGame.getDic(), token))
                        {
                            printf("le mot -%s- existe\n",
                                   convertToMb(token).c_str());
                        }
                        else
                        {
                            printf("le mot -%s- n'existe pas\n",
                                   convertToMb(token).c_str());
                        }
                    }
                    break;
               case L'j':
                    token = next_token_alpha(NULL, delim, &state);
                    if (token == NULL)
                        help_freegame();
                    else
                    {
                        int res;
                        wchar_t *coord = next_token_alphanum(NULL, delim, &state);
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
               case L'p':
                    token = next_token_alpha(NULL, delim, &state);
                    /* You can pass your turn without changing any letter */
                    if (token == NULL)
                        token = L"";

                    if (iGame.pass(token, iGame.currPlayer()) != 0)
                        break;
                    break;
                case L's':
                    token = next_token_filename(NULL, delim, &state);
                    if (token != NULL)
                    {
                        string filename = convertToMb(token);
                        ofstream fout(filename.c_str());
                        if (fout.rdstate() == ios::failbit)
                        {
                            printf("impossible d'ouvrir %s\n",
                                   filename.c_str());
                            break;
                        }
                        iGame.save(fout);
                        fout.close();
                    }
                    break;
                case L'q':
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


void loop_duplicate(Duplicate &iGame)
{
    wchar_t *token;
    wchar_t *state;
    wchar_t *commande = NULL;
    wchar_t delim[] = L" \t";
    int quit = 0;

    printf("mode duplicate\n");
    printf("[?] pour l'aide\n");
    while (quit == 0)
    {
        commande = rl_gets();
        token = wcstok(commande, delim, &state);
        if (token)
        {
            switch (token[0])
            {
                case L'?':
                    help_duplicate();
                    break;
                case L'a':
                    display_data(iGame, delim, &state);
                    break;
                case L'd':
                    token = next_token_alpha(NULL, delim, &state);
                    if (token == NULL)
                        help_duplicate();
                    else
                    {
                        if (Dic_search_word(iGame.getDic(), token))
                        {
                            printf("le mot -%s- existe\n",
                                   convertToMb(token).c_str());
                        }
                        else
                        {
                            printf("le mot -%s- n'existe pas\n",
                                   convertToMb(token).c_str());
                        }
                    }
                    break;
                case L'j':
                    token = next_token_alpha(NULL, delim, &state);
                    if (token == NULL)
                        help_duplicate();
                    else
                    {
                        int res;
                        wchar_t *coord = next_token_alphanum(NULL, delim, &state);
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
                case L'n':
                    token = next_token_digit(NULL, delim, &state);
                    if (token == NULL)
                        help_duplicate();
                    else
                    {
                        int res = iGame.setPlayer(_wtoi(token));
                        if (res == 1)
                            fprintf(stderr, "Numéro de joueur invalide\n");
                        else if (res == 2)
                            fprintf(stderr, "Impossible de choisir un joueur non humain\n");
                    }
                    break;
                case L's':
                    token = next_token_filename(NULL, delim, &state);
                    if (token != NULL)
                    {
                        string filename = convertToMb(token);
                        ofstream fout(filename.c_str());
                        if (fout.rdstate() == ios::failbit)
                        {
                            printf("impossible d'ouvrir %s\n",
                                   filename.c_str());
                            break;
                        }
                        iGame.save(fout);
                        fout.close();
                    }
                    break;
                case L'q':
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


void eliot_regexp_build_default_llist(struct search_RegE_list_t &llist)
{
    memset(&llist, 0, sizeof(llist));

    llist.minlength = 1;
    llist.maxlength = 15;

    llist.symbl[0] = RE_ALL_MATCH;
    llist.symbl[1] = RE_VOWL_MATCH;
    llist.symbl[2] = RE_CONS_MATCH;
    llist.symbl[3] = RE_USR1_MATCH;
    llist.symbl[5] = RE_USR2_MATCH;

    llist.valid[0] = 1; // all letters
    llist.valid[1] = 1; // vowels
    llist.valid[2] = 1; // consonants
    llist.valid[3] = 0; // user defined list 1
    llist.valid[4] = 0; // user defined list 2

    for (int i = 0; i < DIC_SEARCH_REGE_LIST; i++)
    {
        memset(llist.letters[i], 0, sizeof(llist.letters[i]));
    }

    const list<Tile>& allTiles = Tile::getAllTiles();
    list<Tile>::const_iterator it;
    for (it = allTiles.begin(); it != allTiles.end(); it++)
    {
        if (! it->isJoker() && ! it->isEmpty())
        {
            // all tiles
            llist.letters[0][it->toCode()] = 1;
            // vowels
            if (it->isVowel())
            {
                llist.letters[1][it->toCode()] = 1;
            }
            // consonants
            if (it->isConsonant())
            {
                llist.letters[2][it->toCode()] = 1;
            }
        }
    }
}

void eliot_regexp(const Dictionary& iDic, wchar_t *cmd,
                  const wchar_t *delim, wchar_t **state)
{
    /*
    printf("  x [] {1} {2} {3} : expressions rationnelles\n");
    printf("          [] expression à rechercher\n");
    printf("          {1} nombre de résultats à afficher\n");
    printf("          {2} longueur minimum d'un mot\n");
    printf("          {3} longueur maximum d'un mot\n");
    */

#define DIC_RE_MAX (3*DIC_WORD_MAX) // yes, it's 3

    struct search_RegE_list_t llist;
    eliot_regexp_build_default_llist(llist);

    wchar_t *exp, *cnres, *clmin, *clmax;

    exp   = wcstok(NULL, delim, state);
    cnres = wcstok(NULL, delim, state);
    clmin = wcstok(NULL, delim, state);
    clmax = wcstok(NULL, delim, state);

    if (exp == NULL)
    {
        return;
    }
    int nres = cnres ? _wtoi(cnres) : 50;
    int lmin = clmin ? _wtoi(clmin) : 1;
    int lmax = clmax ? _wtoi(clmax) : DIC_WORD_MAX - 1;

    if (lmax <= (DIC_WORD_MAX - 1) && lmin >= 1 && lmin <= lmax)
    {
        llist.minlength = lmin;
        llist.maxlength = lmax;
    }
    else
    {
        printf("bad length -%s,%s-\n", (const char*)clmin, (const char*)clmax);
        return;
    }

    wchar_t re[DIC_RE_MAX];
    wcsncpy(re, exp, DIC_RE_MAX);
    wchar_t buff[RES_REGE_MAX][DIC_WORD_MAX];

    printf("search for %s (%d,%d,%d)\n", convertToMb(exp).c_str(),
           nres, lmin, lmax);
    Dic_search_RegE(iDic, re, buff, &llist);

    int nresult = 0;
    for (int i = 0; i < RES_REGE_MAX && i < nres && buff[i][0]; i++)
    {
        printf("%s\n", convertToMb(buff[i]).c_str());
        nresult++;
    }
    printf("%d printed results\n", nresult);
}


void main_loop(const Dictionary &iDic)
{
    wchar_t *token;
    wchar_t *state;
    wchar_t *commande = NULL;
    wchar_t delim[] = L" \t";
    int quit = 0;

    printf("[?] pour l'aide\n");
    while (quit == 0)
    {
        commande = rl_gets();
        token = wcstok(commande, delim, &state);
        if (token)
        {
            switch (token[0])
            {
                case L'?':
                    help();
                    break;
                case L'c':
                    token = next_token_filename(NULL, delim, &state);
                    if (token == NULL)
                    {}
                    else
                    {
                        string filename = convertToMb(token);
                        FILE* fin;
                        if ((fin = fopen(filename.c_str(), "r")) == NULL)
                        {
                            printf("impossible d'ouvrir %s\n",
                                   filename.c_str());
                            break;
                        }
                        Game *game = Game::load(fin, iDic);
                        fclose(fin);
                        if (game == NULL)
                        {
                            printf("erreur pendant le chargement\n");
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
                case L'e':
                {
                    // New training game
                    Training *game = GameFactory::Instance()->createTraining(iDic);
                    game->start();
                    loop_training(*game);
                    GameFactory::Instance()->releaseGame(*game);
                    break;
                }
                case L'd':
                {
                    int i;
                    // New duplicate game
                    token = next_token_digit(NULL, delim, &state);
                    if (token == NULL)
                    {
                        help();
                        break;
                    }
                    Duplicate *game = GameFactory::Instance()->createDuplicate(iDic);
                    for (i = 0; i < _wtoi(token); i++)
                        game->addHumanPlayer();
                    token = next_token_digit(NULL, delim, &state);
                    if (token == NULL)
                    {
                        help();
                        break;
                    }
                    for (i = 0; i < _wtoi(token); i++)
                        game->addAIPlayer();
                    game->start();
                    loop_duplicate(*game);
                    GameFactory::Instance()->releaseGame(*game);
                    break;
                }
                case L'l':
                {
                    int i;
                    // New free game
                    token = next_token_digit(NULL, delim, &state);
                    if (token == NULL)
                    {
                        help();
                        break;
                    }
                    FreeGame *game = GameFactory::Instance()->createFreeGame(iDic);
                    for (i = 0; i < _wtoi(token); i++)
                        game->addHumanPlayer();
                    token = next_token_digit(NULL, delim, &state);
                    if (token == NULL)
                    {
                        help();
                        break;
                    }
                    for (i = 0; i < _wtoi(token); i++)
                        game->addAIPlayer();
                    game->start();
                    loop_freegame(*game);
                    GameFactory::Instance()->releaseGame(*game);
                    break;
                }
                case L'D':
                {
                    // New duplicate game
                    Duplicate *game = GameFactory::Instance()->createDuplicate(iDic);
                    game->addHumanPlayer();
                    game->addAIPlayer();
                    game->start();
                    loop_duplicate(*game);
                    GameFactory::Instance()->releaseGame(*game);
                    break;
                }
                case L'L':
                {
                    // New free game
                    FreeGame *game = GameFactory::Instance()->createFreeGame(iDic);
                    game->addHumanPlayer();
                    game->addAIPlayer();
                    game->start();
                    loop_freegame(*game);
                    GameFactory::Instance()->releaseGame(*game);
                    break;
                }
                case L'x':
                    // Regular expression tests
                    eliot_regexp(iDic, NULL, delim, &state);
                    break;
                case L'q':
                    quit = 1;
                    break;
                default:
                    printf("commande inconnue\n");
                    break;
            }
        }
    }
}


int main(int argc, char *argv[])
{
    char dic_path[100];

    // Let the user choose the locale
    setlocale(LC_ALL, "");

    Dictionary dic = NULL;

    if (argc != 2 && argc != 3)
    {
        fprintf(stdout, "Usage: eliot /chemin/vers/ods4.dawg [random_seed]\n");
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

    if (argc == 3)
        srand(atoi(argv[2]));
    else
        srand(time(NULL));

    main_loop(dic);
    GameFactory::Destroy();

    Dic_destroy(dic);

    // Free the readline static variable and its wide equivalent
    if (line_read)
        free(line_read);
    if (wline_read)
        delete[] wline_read;

    return 0;
}

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
