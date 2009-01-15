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

#include "config.h"

#include <wchar.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>
#include <wctype.h>
#if HAVE_READLINE_READLINE_H
#   include <stdio.h>
#   include <readline/readline.h>
#   include <readline/history.h>
#endif

#include "dic.h"
#include "dic_exception.h"
#include "game_io.h"
#include "game_factory.h"
#include "public_game.h"
#include "training.h"
#include "duplicate.h"
#include "freegame.h"
#include "player.h"
#include "ai_percent.h"
#include "encoding.h"
#include "game_exception.h"
#include "settings.h"


/* A static variable for holding the line. */
static wchar_t *wline_read = NULL;

/**
 * Read a string, and return a pointer to it.
 * Returns NULL on EOF.
 */
wchar_t *rl_gets()
{
#if HAVE_READLINE_READLINE_H
    // If the buffer has already been allocated, return the memory to the free
    // pool
    if (wline_read)
    {
        delete[] wline_read;
    }

    // Get a line from the user
    static char *line_read;
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
    mbstowcs(wline_read, line_read, len + 1);

    if (line_read)
    {
        free(line_read);
    }
#else
    if (!cin.good())
        return NULL;

    cout << "commande> ";
    string line;
    std::getline(cin, line);
    // Echo the input, to behave like readline and allow playing the
    // non-regression tests
    cout << line << endl;

    // Get the needed length (we _can't_ use string::size())
    size_t len = mbstowcs(NULL, line.c_str(), 0);
    if (len == (size_t)-1)
        return NULL;

    wline_read = new wchar_t[len + 1];
    mbstowcs(wline_read, line.c_str(), len + 1);
#endif

    return wline_read;
}


wchar_t * next_token_alpha(wchar_t *cmd, const wchar_t *delim, wchar_t **state)
{
    wchar_t *token = _wcstok(cmd, delim, state);
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
    wchar_t *token = _wcstok(cmd, delim, state);
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
    wchar_t *token = _wcstok(cmd, delim, state);
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
    wchar_t *token = _wcstok(cmd, delim, state);
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
    wchar_t *token = _wcstok(cmd, delim, state);
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
    wchar_t *token = _wcstok(cmd, delim, state);
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


void eliottxt_get_cross(const Dictionary &iDic, const wstring &iCros)
{
    vector<wstring> wordList;
    iDic.searchCross(iCros, wordList);

    vector<wstring>::const_iterator it;
    for (it = wordList.begin(); it != wordList.end(); it++)
    {
        printf("  %s\n", convertToMb(*it).c_str());
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
    printf("            gd -- grille + debug cross (debug only)\n");
    printf("            l -- lettres non jouées\n");
    printf("            p -- partie\n");
    printf("            pd -- partie (debug)\n");
    printf("            P -- partie (format standard)\n");
    printf("            r -- recherche\n");
    printf("            s -- score\n");
    printf("            S -- score de tous les joueurs\n");
    printf("            t -- tirage\n");
    printf("  d [] : vérifier le mot []\n");
    printf("  b [b|p|r] [] : effectuer une recherche speciale à partir de []\n");
    printf("            b -- benjamins\n");
    printf("            p -- 7 + 1\n");
    printf("            r -- raccords\n");
    printf("  *    : tirage aléatoire\n");
    printf("  +    : tirage aléatoire ajouts\n");
    printf("  t [] : changer le tirage\n");
    printf("  j [] {} : jouer le mot [] aux coordonnées {}\n");
    printf("  n [] : jouer le résultat numéro []\n");
    printf("  r    : rechercher les meilleurs résultats\n");
    printf("  s [] : sauver la partie en cours dans le fichier []\n");
    printf("  h [p|n|f|l] : naviguer dans l'historique (prev, next, first, last\n");
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
    printf("  h [p|n|f|l] : naviguer dans l'historique (prev, next, first, last\n");
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
    printf("  h [p|n|f|l] : naviguer dans l'historique (prev, next, first, last\n");
    printf("  q    : quitter le mode duplicate\n");
}


void help()
{
    printf("  ?       : aide -- cette page\n");
    printf("  e       : démarrer le mode entraînement\n");
    printf("  c []    : charger la partie du fichier []\n");
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
    printf("  s [b|i] {1} {2} : définir la valeur {2} pour l'option {1},\n");
    printf("                    qui est de type (b)ool ou (i)nt}\n");
    printf("  q       : quitter\n");
}


void display_data(const PublicGame &iGame, const wchar_t *delim, wchar_t **state)
{
    const wchar_t *token;

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
                case L'd':
                    GameIO::printBoardDebug(cout, iGame);
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
            cout << "Joueur " << iGame.getCurrentPlayer().getId() << endl;
            break;
        case L'l':
            GameIO::printNonPlayed(cout, iGame);
            break;
        case L'p':
            switch (token[1])
                {
                case '\0':
                    iGame.save(cout,PublicGame::kFILE_FORMAT_ADVANCED);
                    break;
                case 'd':
                    GameIO::printGameDebug(cout, iGame);
                    break;
                default:
                    printf("commande inconnue\n");
                }
            break;
        case L'P':
            iGame.save(cout,PublicGame::kFILE_FORMAT_STANDARD);
            break;
        case L'r':
            token = next_token_digit(NULL, delim, state);
            if (token == NULL)
                GameIO::printSearchResults(cout,
                                           iGame.trainingGetResults(),
                                           10);
            else
                GameIO::printSearchResults(cout,
                                           iGame.trainingGetResults(),
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


void loop_training(PublicGame &iGame)
{
    const wchar_t *token;
    wchar_t *state;
    wchar_t *commande = NULL;
    wchar_t delim[] = L" \t";
    int quit = 0;

    cout << "mode entraînement\n";
    cout << "[?] pour l'aide\n";
    while (quit == 0)
    {
        commande = rl_gets();
        token = _wcstok(commande, delim, &state);
        if (token)
        {
            try
            {
                switch (token[0])
                {
                    case L'?':
                        help_training();
                        break;
                    case L'a':
                        display_data(iGame, delim, &state);
                        break;
                    case L'b':
                        token = next_token_alpha(NULL, delim, &state);
                        if (token == NULL)
                            help_training();
                        else
                        {
                            const wchar_t *word = next_token_alpha(NULL, delim, &state);
                            if (word == NULL)
                                help_training();
                            else
                            {
                                switch (token[0])
                                {
                                    case L'b':
                                        {
                                            vector<wstring> wordList;
                                            iGame.getDic().searchBenj(word, wordList);
                                            vector<wstring>::const_iterator it;
                                            for (it = wordList.begin(); it != wordList.end(); ++it)
                                                cout << convertToMb(*it) << endl;
                                            break;
                                        }
                                    case L'p':
                                        {
                                            map<wchar_t, vector<wstring> > wordMap;
                                            iGame.getDic().search7pl1(word, wordMap, false);
                                            map<wchar_t, vector<wstring> >::const_iterator it;
                                            for (it = wordMap.begin(); it != wordMap.end(); ++it)
                                            {
                                                if (it->first)
                                                    cout << "+" << convertToMb(it->first) << endl;
                                                vector<wstring>::const_iterator itWord;;
                                                for (itWord = it->second.begin(); itWord != it->second.end(); itWord++)
                                                {
                                                    cout << "  " << convertToMb(*itWord) << endl;
                                                }
                                            }
                                            break;
                                        }
                                    case L'r':
                                        {
                                            vector<wstring> wordList;
                                            iGame.getDic().searchRacc(word, wordList);
                                            vector<wstring>::const_iterator it;
                                            for (it = wordList.begin(); it != wordList.end(); ++it)
                                                cout << convertToMb(*it) << endl;
                                            break;
                                        }
                                }
                            }
                        }
                        break;
                    case L'd':
                        token = next_token_alpha(NULL, delim, &state);
                        if (token == NULL)
                            help_training();
                        else
                        {
                            if (iGame.getDic().searchWord(token))
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
                            if ((res = iGame.play(token, coord)) != 0)
                            {
                                fprintf(stdout, "Mot incorrect ou mal placé (%i)\n",
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
                            {
                                if (n == 0)
                                    n = -1;
                                if ((unsigned int)-n > iGame.getHistory().getSize())
                                    throw GameException("Cannot go back that far");
                                for (int i = 0; i < -n; ++i)
                                    iGame.prevTurn();
                                iGame.clearFuture();
                            }
                            else
                            {
                                if (iGame.trainingPlayResult(--n))
                                    printf("mauvais argument\n");
                            }
                        }
                        break;
                    case L'r':
                        iGame.trainingSearch();
                        break;
                    case L't':
                        token = next_token_alphaplusjoker(NULL, delim, &state);
                        if (token == NULL)
                            help_training();
                        else
                            if (iGame.trainingSetRackManual(0, token))
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
                        iGame.trainingSetRackRandom(false, PublicGame::kRACK_ALL);
                        break;
                    case L'+':
                        iGame.trainingSetRackRandom(false, PublicGame::kRACK_NEW);
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
                    case L'h':
                        token = next_token_alpha(NULL, delim, &state);
                        if (token != NULL)
                        {
                            switch (token[0])
                            {
                                case L'p':
                                    iGame.prevTurn();
                                    break;
                                case L'n':
                                    iGame.nextTurn();
                                    break;
                                case L'f':
                                    iGame.firstTurn();
                                    break;
                                case L'l':
                                    iGame.lastTurn();
                                    break;
                            }
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
            catch (GameException &e)
            {
                printf("%s\n", e.what());
            }
        }
    }
    printf("fin du mode entraînement\n");
}


void loop_freegame(PublicGame &iGame)
{
    const wchar_t *token;
    wchar_t *state;
    wchar_t *commande = NULL;
    wchar_t delim[] = L" \t";
    int quit = 0;

    printf("mode partie libre\n");
    printf("[?] pour l'aide\n");
    while (quit == 0)
    {
        commande = rl_gets();
        token = _wcstok(commande, delim, &state);
        if (token)
        {
            try
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
                            if (iGame.getDic().searchWord(token))
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
                            if ((res = iGame.play(token, coord)) != 0)
                            {
                                fprintf(stdout, "Mot incorrect ou mal placé (%i)\n",
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

                        if (iGame.freeGamePass(token) != 0)
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
                    case L'h':
                        token = next_token_alpha(NULL, delim, &state);
                        if (token != NULL)
                        {
                            switch (token[0])
                            {
                                case L'p':
                                    iGame.prevTurn();
                                    break;
                                case L'n':
                                    iGame.nextTurn();
                                    break;
                                case L'f':
                                    iGame.firstTurn();
                                    break;
                                case L'l':
                                    iGame.lastTurn();
                                    break;
                            }
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
            catch (GameException &e)
            {
                printf("%s\n", e.what());
            }
        }
    }
    printf("fin du mode partie libre\n");
}


void loop_duplicate(PublicGame &iGame)
{
    const wchar_t *token;
    wchar_t *state;
    wchar_t *commande = NULL;
    wchar_t delim[] = L" \t";
    int quit = 0;

    printf("mode duplicate\n");
    printf("[?] pour l'aide\n");
    while (quit == 0)
    {
        commande = rl_gets();
        token = _wcstok(commande, delim, &state);
        if (token)
        {
            try
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
                            if (iGame.getDic().searchWord(token))
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
                            if ((res = iGame.play(token, coord)) != 0)
                            {
                                fprintf(stdout, "Mot incorrect ou mal placé (%i)\n",
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
                            int n = _wtoi(token);
                            if (n < 0 || n >= (int)iGame.getNbPlayers())
                            {
                                fprintf(stderr, "Numéro de joueur invalide\n");
                                break;
                            }
                            int res = iGame.duplicateSetPlayer(_wtoi(token));
                            if (res == 1)
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
                    case L'h':
                        token = next_token_alpha(NULL, delim, &state);
                        if (token != NULL)
                        {
                            switch (token[0])
                            {
                                case L'p':
                                    iGame.prevTurn();
                                    break;
                                case L'n':
                                    iGame.nextTurn();
                                    break;
                                case L'f':
                                    iGame.firstTurn();
                                    break;
                                case L'l':
                                    iGame.lastTurn();
                                    break;
                            }
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
            catch (GameException &e)
            {
                printf("%s\n", e.what());
            }
        }
    }
    printf("fin du mode duplicate\n");
}


void eliot_regexp(const Dictionary& iDic,
                  const wchar_t *delim, wchar_t **state)
{
    /*
    printf("  x [] {1} {2} {3} : expressions rationnelles\n");
    printf("          [] expression à rechercher\n");
    printf("          {1} nombre de résultats à afficher\n");
    printf("          {2} longueur minimum d'un mot\n");
    printf("          {3} longueur maximum d'un mot\n");
    */

    wchar_t *regexp = _wcstok(NULL, delim, state);
    wchar_t *cnres = _wcstok(NULL, delim, state);
    wchar_t *clmin = _wcstok(NULL, delim, state);
    wchar_t *clmax = _wcstok(NULL, delim, state);

    if (regexp == NULL)
    {
        return;
    }
    unsigned int nres = cnres ? _wtoi(cnres) : 50;
    unsigned int lmin = clmin ? _wtoi(clmin) : 1;
    unsigned int lmax = clmax ? _wtoi(clmax) : DIC_WORD_MAX - 1;

    if (lmax > (DIC_WORD_MAX - 1) || lmin < 1 || lmin > lmax)
    {
        printf("bad length -%s,%s-\n", (const char*)clmin, (const char*)clmax);
        return;
    }

    printf("search for %s (%d,%d,%d)\n", convertToMb(regexp).c_str(),
           nres, lmin, lmax);

    vector<wstring> wordList;
    try
    {
        iDic.searchRegExp(regexp, wordList, lmin, lmax, nres);
    }
    catch (InvalidRegexpException &e)
    {
        printf("Invalid regular expression: %s\n", e.what());
        return;
    }

    vector<wstring>::const_iterator it;
    for (it = wordList.begin(); it != wordList.end(); it++)
    {
        printf("%s\n", convertToMb(*it).c_str());
    }
    printf("%d printed results\n", wordList.size());
}


void set_setting(const wchar_t *delim, wchar_t **state)
{
    wchar_t *type = next_token_alpha(NULL, delim, state);
    if (type == NULL || (*type != L'b' && *type != L'i'))
    {
        printf("Invalid type\n");
        return;
    }
    wchar_t *settingWide = _wcstok(NULL, delim, state);
    if (settingWide == NULL)
    {
        printf("Invalid setting name\n");
        return;
    }
    wchar_t *value = _wcstok(NULL, delim, state);
    if (value == NULL)
    {
        printf("Invalid value\n");
        return;
    }

    try
    {
        string setting = convertToMb(settingWide);
        if (*type == L'i')
        {
            Settings::Instance().setInt(setting, _wtoi(value));
        }
        else if (*type == L'b')
        {
            Settings::Instance().setBool(setting, _wtoi(value));
        }
    }
    catch (GameException &e)
    {
        string msg = "Error while changing a setting: " + string(e.what()) + "\n";
        printf("%s", msg.c_str());
        return;
    }

}


void main_loop(const Dictionary &iDic)
{
    const wchar_t *token;
    wchar_t *state;
    wchar_t *commande = NULL;
    wchar_t delim[] = L" \t";
    int quit = 0;

    printf("[?] pour l'aide\n");
    while (quit == 0)
    {
        commande = rl_gets();
        token = _wcstok(commande, delim, &state);
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
                        Game *tmpGame = GameFactory::Instance()->load(filename, iDic);
                        if (tmpGame == NULL)
                        {
                            printf("erreur pendant le chargement de la partie\n");
                        }
                        else
                        {
                            PublicGame *game = new PublicGame(*tmpGame);
                            switch (game->getMode())
                                {
                                case PublicGame::kTRAINING:
                                    loop_training(*game);
                                    break;
                                case PublicGame::kFREEGAME:
                                    loop_freegame(*game);
                                    break;
                                case PublicGame::kDUPLICATE:
                                    loop_duplicate(*game);
                                    break;
                                }
                            //GameFactory::Instance()->releaseGame(*game);
                            delete game;
                        }
                    }
                    break;
                case L'e':
                {
                    // New training game
                    Training *tmpGame = GameFactory::Instance()->createTraining(iDic);
                    PublicGame *game = new PublicGame(*tmpGame);
                    game->start();
                    loop_training(*game);
                    //GameFactory::Instance()->releaseGame(*game);
                    delete game;
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
                    Duplicate *tmpGame = GameFactory::Instance()->createDuplicate(iDic);
                    PublicGame *game = new PublicGame(*tmpGame);
                    for (i = 0; i < _wtoi(token); i++)
                        game->addPlayer(new HumanPlayer);
                    token = next_token_digit(NULL, delim, &state);
                    if (token == NULL)
                    {
                        help();
                        break;
                    }
                    for (i = 0; i < _wtoi(token); i++)
                        game->addPlayer(new AIPercent(1));
                    game->start();
                    loop_duplicate(*game);
                    //GameFactory::Instance()->releaseGame(*game);
                    delete game;
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
                    FreeGame *tmpGame = GameFactory::Instance()->createFreeGame(iDic);
                    PublicGame *game = new PublicGame(*tmpGame);
                    for (i = 0; i < _wtoi(token); i++)
                        game->addPlayer(new HumanPlayer);
                    token = next_token_digit(NULL, delim, &state);
                    if (token == NULL)
                    {
                        help();
                        break;
                    }
                    for (i = 0; i < _wtoi(token); i++)
                        game->addPlayer(new AIPercent(1));
                    game->start();
                    loop_freegame(*game);
                    //GameFactory::Instance()->releaseGame(*game);
                    delete game;
                    break;
                }
                case L'D':
                {
                    // New duplicate game
                    Duplicate *tmpGame = GameFactory::Instance()->createDuplicate(iDic);
                    PublicGame *game = new PublicGame(*tmpGame);
                    game->addPlayer(new HumanPlayer);
                    game->addPlayer(new AIPercent(1));
                    game->start();
                    loop_duplicate(*game);
                    //GameFactory::Instance()->releaseGame(*game);
                    delete game;
                    break;
                }
                case L'L':
                {
                    // New free game
                    FreeGame *tmpGame = GameFactory::Instance()->createFreeGame(iDic);
                    PublicGame *game = new PublicGame(*tmpGame);
                    game->addPlayer(new HumanPlayer);
                    game->addPlayer(new AIPercent(1));
                    game->start();
                    loop_freegame(*game);
                    //GameFactory::Instance()->releaseGame(*game);
                    delete game;
                    break;
                }
                case L'x':
                    // Regular expression tests
                    eliot_regexp(iDic, delim, &state);
                    break;
                case L's':
                    set_setting(delim, &state);
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
    string dicPath;

    // Let the user choose the locale
    setlocale(LC_ALL, "");

    if (argc != 2 && argc != 3)
    {
        fprintf(stdout, "Usage: eliot /chemin/vers/ods4.dawg [random_seed]\n");
        exit(1);
    }
    else
    {
        dicPath = argv[1];
    }

    try
    {
        Dictionary dic(dicPath);

        unsigned int seed;
        if (argc == 3)
            seed = atoi(argv[2]);
        else
            seed = time(NULL);
        srand(seed);
        cerr << "Using seed: " << seed << endl;

        main_loop(dic);
        GameFactory::Destroy();

        // Free the readline static variable
        if (wline_read)
            delete[] wline_read;
    }
    catch (std::exception &e)
    {
        cerr << e.what() << endl;
    }

    return 0;
}

