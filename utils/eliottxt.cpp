/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2009 Antoine Fraboulet & Olivier Teulière
 * Authors: Antoine Fraboulet <antoine.fraboulet @@ free.fr>
 *          Olivier Teulière <ipkiss @@ gmail.com>
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

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
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
#include "header.h"
#include "dic_exception.h"
#include "game_io.h"
#include "game_params.h"
#include "game_factory.h"
#include "public_game.h"
#include "game.h"
#include "player.h"
#include "ai_percent.h"
#include "encoding.h"
#include "game_exception.h"
#include "settings.h"
#include "move.h"

class Game;


// Use a more friendly type name for the tokenizer
typedef boost::tokenizer<boost::char_separator<wchar_t>,
        std::wstring::const_iterator,
        std::wstring> Tokenizer;

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


wstring checkAlphaToken(const vector<wstring> &tokens, uint8_t index)
{
    if (tokens.size() <= index)
        return L"";
    const wstring &wstr = tokens[index];
    BOOST_FOREACH(wchar_t wch, wstr)
    {
        if (!iswalpha(wch))
            return L"";
    }
    return wstr;
}


wstring checkLettersToken(const vector<wstring> &tokens, uint8_t index,
                          const Dictionary &iDic)
{
    if (tokens.size() <= index)
        return L"";
    return iDic.convertFromInput(tokens[index]);
}


wstring checkAlphaNumToken(const vector<wstring> &tokens, uint8_t index)
{
    if (tokens.size() <= index)
        return L"";
    const wstring &wstr = tokens[index];
    BOOST_FOREACH(wchar_t wch, wstr)
    {
        if (!iswalnum(wch))
            return L"";
    }
    return wstr;
}


wstring checkNumToken(const vector<wstring> &tokens, uint8_t index)
{
    if (tokens.size() <= index)
        return L"";
    const wstring &wstr = tokens[index];
    BOOST_FOREACH(wchar_t wch, wstr)
    {
        if (!iswdigit(wch) && wch != L'-')
            return L"";
    }
    return wstr;
}


wstring checkFileNameToken(const vector<wstring> &tokens, uint8_t index)
{
    if (tokens.size() <= index)
        return L"";
    const wstring &wstr = tokens[index];
    BOOST_FOREACH(wchar_t wch, wstr)
    {
        if (!iswalnum(wch) && wch != L'.' && wch != L'_')
            return L"";
    }
    return wstr;
}


wstring checkCrossToken(const vector<wstring> &tokens, uint8_t index)
{
    if (tokens.size() <= index)
        return L"";
    const wstring &wstr = tokens[index];
    BOOST_FOREACH(wchar_t wch, wstr)
    {
        if (!iswalpha(wch) && wch != L'.')
            return L"";
    }
    return wstr;
}

PublicGame * readGame(const Dictionary &iDic,
                      GameParams::GameMode iMode, const wstring &iToken)
{
    GameParams params(iDic, iMode);
    for (unsigned int i = 1; i < iToken.size(); ++i)
    {
        if (iToken[i] == L'j')
            params.addVariant(GameParams::kJOKER);
        else if (iToken[i] == L'e')
            params.addVariant(GameParams::kEXPLOSIVE);
        else if (iToken[i] == L'8')
            params.addVariant(GameParams::k7AMONG8);
    }
    Game *tmpGame = GameFactory::Instance()->createGame(params);
    return new PublicGame(*tmpGame);
}

void helpTraining()
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
    printf("  h [p|n|f|l|r] : naviguer dans l'historique (prev, next, first, last, replay)\n");
    printf("  q    : quitter le mode entraînement\n");
}


void helpFreegame()
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
    printf("  h [p|n|f|l|r] : naviguer dans l'historique (prev, next, first, last, replay)\n");
    printf("  q    : quitter le mode partie libre\n");
}


void helpDuplicate()
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
    printf("  h [p|n|f|l|r] : naviguer dans l'historique (prev, next, first, last, replay\n");
    printf("  q    : quitter le mode duplicate\n");
}


void helpArbitration()
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
    printf("            r -- recherche\n");
    printf("            s -- score\n");
    printf("            S -- score de tous les joueurs\n");
    printf("            t -- tirage\n");
    printf("  d [] : vérifier le mot []\n");
    printf("  *    : tirage aléatoire\n");
    printf("  t [] : changer le tirage\n");
    printf("  j j [] {} : jouer le mot [] aux coordonnées {} pour le joueur j\n");
    printf("  m [] {} : définir le master move [] aux coordonnées {}\n");
    printf("  e j [w|p|s] {} : assigner un événement au joueur j :\n");
    printf("            w -- avertissement\n");
    printf("            p -- pénalité\n");
    printf("            s -- solo\n");
    printf("            {} -- valeur de l'événement\n");
    printf("  f    : finaliser le tour courant\n");
    printf("  s [] : sauver la partie en cours dans le fichier []\n");
    printf("  h [p|n|f|l|r] : naviguer dans l'historique (prev, next, first, last, replay)\n");
    printf("  q    : quitter le mode arbitrage\n");
}


void help()
{
    printf("  ?        : aide -- cette page\n");
    printf("  e        : démarrer le mode entraînement\n");
    printf("  ej       : démarrer le mode entraînement en partie joker\n");
    printf("  ee       : démarrer le mode entraînement en partie détonante\n");
    printf("  e8       : démarrer le mode entraînement en partie 7 sur 8\n");
    printf("  d [] {}  : démarrer une partie duplicate avec\n");
    printf("                [] joueurs humains et {} joueurs IA\n");
    printf("  dj [] {} : démarrer une partie duplicate avec\n");
    printf("                [] joueurs humains et {} joueurs IA (partie joker)\n");
    printf("  de [] {} : démarrer une partie duplicate avec\n");
    printf("                [] joueurs humains et {} joueurs IA (partie détonante)\n");
    printf("  d8 [] {} : démarrer une partie duplicate avec\n");
    printf("                [] joueurs humains et {} joueurs IA (partie 7 sur 8)\n");
    printf("  l [] {}  : démarrer une partie libre avec\n");
    printf("                [] joueurs humains et {} joueurs IA\n");
    printf("  lj [] {} : démarrer une partie libre avec\n");
    printf("                [] joueurs humains et {} joueurs IA (partie joker)\n");
    printf("  le [] {} : démarrer une partie libre avec\n");
    printf("                [] joueurs humains et {} joueurs IA (partie détonante)\n");
    printf("  l8 [] {} : démarrer une partie libre avec\n");
    printf("                [] joueurs humains et {} joueurs IA (partie 7 sur 8)\n");
    printf("  a [] {}  : démarrer une partie arbitrage avec\n");
    printf("                [] joueurs humains et {} joueurs IA\n");
    printf("  aj [] {} : démarrer une partie arbitrage avec\n");
    printf("                [] joueurs humains et {} joueurs IA (partie joker)\n");
    printf("  ae [] {} : démarrer une partie arbitrage avec\n");
    printf("                [] joueurs humains et {} joueurs IA (partie détonante)\n");
    printf("  a8 [] {} : démarrer une partie arbitrage avec\n");
    printf("                [] joueurs humains et {} joueurs IA (partie 7 sur 8)\n");
    printf("  c []     : charger la partie du fichier []\n");
    printf("  x [] {1} {2} {3} : expressions rationnelles\n");
    printf("          [] expression à rechercher\n");
    printf("          {1} nombre de résultats à afficher\n");
    printf("          {2} longueur minimum d'un mot\n");
    printf("          {3} longueur maximum d'un mot\n");
    printf("  s [b|i] {1} {2} : définir la valeur {2} pour l'option {1},\n");
    printf("                    qui est de type (b)ool ou (i)nt\n");
    printf("  q        : quitter\n");
}


void displayData(const PublicGame &iGame, const vector<wstring> &tokens)
{
    const wstring &displayType = checkAlphaNumToken(tokens, 1);
    if (displayType == L"")
    {
        cout << "commande incomplète\n";
        return;
    }
    if (displayType == L"g")
        GameIO::printBoard(cout, iGame);
    else if (displayType == L"gd")
        GameIO::printBoardDebug(cout, iGame);
    else if (displayType == L"gj")
        GameIO::printBoardJoker(cout, iGame);
    else if (displayType == L"gm")
        GameIO::printBoardMultipliers(cout, iGame);
    else if (displayType == L"gn")
        GameIO::printBoardMultipliers2(cout, iGame);
    else if (displayType == L"j")
        cout << "Joueur " << iGame.getCurrentPlayer().getId() << endl;
    else if (displayType == L"l")
        GameIO::printNonPlayed(cout, iGame);
    else if (displayType == L"p")
    {
        GameIO::printGameDebug(cout, iGame);
        GameIO::printAllRacks(cout, iGame);
        GameIO::printAllPoints(cout, iGame);
    }
    else if (displayType == L"pd")
        GameIO::printGameDebug(cout, iGame);
    else if (displayType == L"r")
    {
        const wstring &limit = checkNumToken(tokens, 2);
        if (limit == L"")
            GameIO::printSearchResults(cout,
                                       iGame.trainingGetResults(),
                                       10);
        else
            GameIO::printSearchResults(cout,
                                       iGame.trainingGetResults(),
                                       wtoi(limit.c_str()));
    }
    else if (displayType == L"s")
        GameIO::printPoints(cout, iGame);
    else if (displayType == L"S")
        GameIO::printAllPoints(cout, iGame);
    else if (displayType == L"t")
        GameIO::printPlayedRack(cout, iGame, iGame.getHistory().getSize());
    else if (displayType == L"T")
        GameIO::printAllRacks(cout, iGame);
    else
        cout << "commande inconnue\n";
}


void commonCommands(PublicGame &iGame, const vector<wstring> &tokens)
{
    if (tokens[0][0] == L'a')
        displayData(iGame, tokens);
    else if (tokens[0][0] == L'd')
    {
        const wstring &word = checkLettersToken(tokens, 1, iGame.getDic());
        if (word == L"")
            helpDuplicate();
        else
        {
            if (iGame.getDic().searchWord(word))
                printf("le mot -%s- existe\n", lfw(word).c_str());
            else
                printf("le mot -%s- n'existe pas\n", lfw(word).c_str());
        }
    }
    else if (tokens[0][0] == L'h')
    {
        const wstring &action = checkAlphaToken(tokens, 1);
        wstring count = checkNumToken(tokens, 2);
        if (count == L"")
            count = L"1";
        if (action == L"" || action.size() != 1)
            return;
        if (action[0] == L'p')
        {
            for (int i = 0; i < wtoi(count.c_str()); ++i)
                iGame.prevTurn();
        }
        else if (action[0] == L'n')
        {
            for (int i = 0; i < wtoi(count.c_str()); ++i)
                iGame.nextTurn();
        }
        else if (action[0] == L'f')
            iGame.firstTurn();
        else if (action[0] == L'l')
            iGame.lastTurn();
        else if (action[0] == L'r')
            iGame.clearFuture();
    }
    else if (tokens[0][0] == L'j')
    {
        const wstring &word = checkLettersToken(tokens, 1, iGame.getDic());
        if (word == L"")
            helpDuplicate();
        else
        {
            const wstring &coord = checkAlphaNumToken(tokens, 2);
            if (coord == L"")
            {
                helpDuplicate();
                return;
            }
            int res = iGame.play(word, coord);
            if (res != 0)
                printf("Mot incorrect ou mal placé (%i)\n", res);
        }
    }
    else if (tokens[0][0] == L's')
    {
        const wstring &word = checkFileNameToken(tokens, 1);
        if (word != L"")
        {
            try
            {
                iGame.save(lfw(word));
            }
            catch (std::exception &e)
            {
                printf("Cannot save game to %ls: %s\n", word.c_str(), e.what());
                return;
            }
        }
    }
}


void handleRegexp(const Dictionary& iDic, const vector<wstring> &tokens)
{
    /*
    printf("  x [] {1} {2} {3} : expressions rationnelles\n");
    printf("          [] expression à rechercher\n");
    printf("          {1} nombre de résultats à afficher\n");
    printf("          {2} longueur minimum d'un mot\n");
    printf("          {3} longueur maximum d'un mot\n");
    */

    if (tokens.size() < 2 || tokens.size() > 5)
    {
        printf("Invalid number of parameters\n");
        return;
    }

    const wstring &regexp = tokens[1];
    const wstring &cnres = checkNumToken(tokens, 2);
    const wstring &clmin = checkNumToken(tokens, 3);
    const wstring &clmax = checkNumToken(tokens, 4);

    if (regexp == L"")
        return;

    unsigned int nres = (cnres != L"") ? wtoi(cnres.c_str()) : 50;
    unsigned int lmin = (clmin != L"") ? wtoi(clmin.c_str()) : 1;
    unsigned int lmax = (clmax != L"") ? wtoi(clmax.c_str()) : DIC_WORD_MAX - 1;

    if (lmax > (DIC_WORD_MAX - 1) || lmin < 1 || lmin > lmax)
    {
        printf("bad length -%ls,%ls-\n", clmin.c_str(), clmax.c_str());
        return;
    }

    printf("search for %s (%d,%d,%d)\n", lfw(regexp).c_str(),
           nres, lmin, lmax);

    vector<wdstring> wordList;
    try
    {
        iDic.searchRegExp(regexp, wordList, lmin, lmax, nres);
    }
    catch (InvalidRegexpException &e)
    {
        printf("Invalid regular expression: %s\n", e.what());
        return;
    }

    BOOST_FOREACH(const wdstring &wstr, wordList)
    {
        printf("%s\n", lfw(wstr).c_str());
    }
    printf("%lu printed results\n", (long unsigned)wordList.size());
}


void setSetting(const vector<wstring> &tokens)
{
    if (tokens.size() != 4)
    {
        printf("Invalid number of parameters\n");
        return;
    }
    const wstring &type = checkAlphaToken(tokens, 1);
    if (type.size() != 1 ||
        (type[0] != L'b' && type[0] != L'i'))
    {
        printf("Invalid type\n");
        return;
    }
    const wstring &settingWide = tokens[2];
    const wstring &value = tokens[3];

    try
    {
        string setting = lfw(settingWide);
        if (type == L"i")
        {
            Settings::Instance().setInt(setting, wtoi(value.c_str()));
        }
        else if (type == L"b")
        {
            Settings::Instance().setBool(setting, wtoi(value.c_str()));
        }
    }
    catch (GameException &e)
    {
        string msg = "Error while changing a setting: " + string(e.what()) + "\n";
        printf("%s", msg.c_str());
        return;
    }
}


void loopTraining(PublicGame &iGame)
{
    cout << "mode entraînement" << endl;
    cout << "[?] pour l'aide" << endl;

    bool quit = 0;
    while (!quit)
    {
        wstring command = rl_gets();
        // Split the command
        vector<wstring> tokens;
        boost::char_separator<wchar_t> sep(L" ");
        Tokenizer tok(command, sep);
        BOOST_FOREACH(const wstring &wstr, tok)
        {
            tokens.push_back(wstr);
        }

        if (tokens.empty())
            continue;
        if (tokens[0].size() > 1)
        {
            printf("%s\n", "Invalid command");
            continue;
        }

        try
        {
            switch (tokens[0][0])
            {
                case L'?':
                    helpTraining();
                    break;
                case L'a':
                case L'd':
                case L'h':
                case L'j':
                case L's':
                    commonCommands(iGame, tokens);
                    break;
                case L'b':
                    {
                        const wstring &type = checkAlphaToken(tokens, 1);
                        if (type == L"")
                            helpTraining();
                        else
                        {
                            const wstring &word = checkLettersToken(tokens, 2, iGame.getDic());
                            if (word == L"")
                                helpTraining();
                            else
                            {
                                switch (type[0])
                                {
                                    case L'b':
                                        {
                                            vector<wdstring> wordList;
                                            iGame.getDic().searchBenj(word, wordList);
                                            BOOST_FOREACH(const wdstring &wstr, wordList)
                                                cout << lfw(wstr) << endl;
                                        }
                                        break;
                                    case L'p':
                                        {
                                            map<unsigned int, vector<wdstring> > wordMap;
                                            iGame.getDic().search7pl1(word, wordMap, false);
                                            map<unsigned int, vector<wdstring> >::const_iterator it;
                                            for (it = wordMap.begin(); it != wordMap.end(); ++it)
                                            {
                                                if (it->first != 0)
                                                    cout << "+" << lfw(iGame.getDic().getHeader().getDisplayStr(it->first)) << endl;
                                                BOOST_FOREACH(const wdstring &wstr, it->second)
                                                {
                                                    cout << "  " << lfw(wstr) << endl;
                                                }
                                            }
                                        }
                                        break;
                                    case L'r':
                                        {
                                            vector<wdstring> wordList;
                                            iGame.getDic().searchRacc(word, wordList);
                                            BOOST_FOREACH(const wdstring &wstr, wordList)
                                                cout << lfw(wstr) << endl;
                                        }
                                        break;
                                }
                            }
                        }
                    }
                    break;
                case L'n':
                    {
                        const wstring &num = checkNumToken(tokens, 1);
                        if (num == L"")
                            helpTraining();
                        else
                        {
                            int n = wtoi(num.c_str());
                            if (n <= 0)
                                printf("mauvais argument\n");
                            iGame.trainingPlayResult(n - 1);
                        }
                    }
                    break;
                case L'r':
                    iGame.trainingSearch();
                    break;
                case L't':
                    {
                        const wstring &letters =
                            checkLettersToken(tokens, 1, iGame.getDic());
                        if (letters == L"")
                            helpTraining();
                        else
                            iGame.trainingSetRackManual(false, letters);
                    }
                    break;
                case L'*':
                    iGame.trainingSetRackRandom(false, PublicGame::kRACK_ALL);
                    break;
                case L'+':
                    iGame.trainingSetRackRandom(false, PublicGame::kRACK_NEW);
                    break;
                case L'q':
                    quit = true;
                    break;
                default:
                    printf("commande inconnue\n");
                    break;
            }
        }
        catch (std::exception &e)
        {
            printf("%s\n", e.what());
        }
    }
    printf("fin du mode entraînement\n");
}


void loopFreegame(PublicGame &iGame)
{
    printf("mode partie libre\n");
    printf("[?] pour l'aide\n");

    bool quit = 0;
    while (!quit)
    {
        wstring command = rl_gets();
        // Split the command
        vector<wstring> tokens;
        boost::char_separator<wchar_t> sep(L" ");
        Tokenizer tok(command, sep);
        BOOST_FOREACH(const wstring &wstr, tok)
        {
            tokens.push_back(wstr);
        }

        if (tokens.empty())
            continue;
        if (tokens[0].size() > 1)
        {
            printf("%s\n", "Invalid command");
            continue;
        }

        try
        {
            switch (tokens[0][0])
            {
                case L'?':
                    helpFreegame();
                    break;
                case L'a':
                case L'd':
                case L'h':
                case L'j':
                case L's':
                    commonCommands(iGame, tokens);
                    break;
                case L'p':
                    {
                        wstring letters = L"";
                        /* You can pass your turn without changing any letter */
                        if (tokens.size() > 1)
                        {
                            letters = checkLettersToken(tokens, 1, iGame.getDic());
                            if (letters == L"")
                                fprintf(stderr, "Invalid letters\n");
                        }
                        // XXX
                        if (iGame.freeGamePass(letters) != 0)
                            break;
                    }
                    break;
                case L'q':
                    quit = true;
                    break;
                default:
                    printf("commande inconnue\n");
                    break;
            }
        }
        catch (std::exception &e)
        {
            printf("%s\n", e.what());
        }
    }
    printf("fin du mode partie libre\n");
}


void loopDuplicate(PublicGame &iGame)
{
    printf("mode duplicate\n");
    printf("[?] pour l'aide\n");

    bool quit = false;
    while (!quit)
    {
        wstring command = rl_gets();
        // Split the command
        vector<wstring> tokens;
        boost::char_separator<wchar_t> sep(L" ");
        Tokenizer tok(command, sep);
        BOOST_FOREACH(const wstring &wstr, tok)
        {
            tokens.push_back(wstr);
        }

        if (tokens.empty())
            continue;
        if (tokens[0].size() > 1)
        {
            printf("%s\n", "Invalid command");
            continue;
        }

        try
        {
            switch (tokens[0][0])
            {
                case L'?':
                    helpDuplicate();
                    break;
                case L'a':
                case L'd':
                case L'h':
                case L'j':
                case L's':
                    commonCommands(iGame, tokens);
                    break;
                case L'n':
                    {
                        const wstring &id = checkNumToken(tokens, 1);
                        if (id == L"")
                            helpDuplicate();
                        else
                        {
                            int n = wtoi(id.c_str());
                            if (n < 0 || n >= (int)iGame.getNbPlayers())
                            {
                                fprintf(stderr, "Numéro de joueur invalide\n");
                                break;
                            }
                            iGame.duplicateSetPlayer(n);
                        }
                    }
                    break;
                case L'q':
                    quit = true;
                    break;
                default:
                    printf("commande inconnue\n");
                    break;
            }
        }
        catch (std::exception &e)
        {
            printf("%s\n", e.what());
        }
    }
    printf("fin du mode duplicate\n");
}


void loopArbitration(PublicGame &iGame)
{
    printf("mode arbitrage\n");
    printf("[?] pour l'aide\n");

    bool quit = false;
    while (!quit)
    {
        wstring command = rl_gets();
        // Split the command
        vector<wstring> tokens;
        boost::char_separator<wchar_t> sep(L" ");
        Tokenizer tok(command, sep);
        BOOST_FOREACH(const wstring &wstr, tok)
        {
            tokens.push_back(wstr);
        }

        if (tokens.empty())
            continue;
        if (tokens[0].size() > 1)
        {
            printf("%s\n", "Invalid command");
            continue;
        }

        try
        {
            switch (tokens[0][0])
            {
                case L'?':
                    helpArbitration();
                    break;
                case L'a':
                case L'd':
                case L'h':
                case L's':
                    commonCommands(iGame, tokens);
                    break;
                case L'f':
                    iGame.arbitrationFinalizeTurn();
                    break;
                case L'j':
                    {
                        const wstring &id = checkNumToken(tokens, 1);
                        const wstring &word = checkLettersToken(tokens, 2, iGame.getDic());
                        const wstring &coord = checkAlphaNumToken(tokens, 3);
                        if (id == L"" || word == L"" || coord == L"")
                        {
                            helpArbitration();
                            break;
                        }
                        int n = wtoi(id.c_str());
                        if (n < 0 || n >= (int)iGame.getNbPlayers())
                        {
                            fprintf(stderr, "Numéro de joueur invalide\n");
                            break;
                        }
                        const Move &move = iGame.arbitrationCheckWord(word, coord);
                        iGame.arbitrationAssign(n, move);
                    }
                    break;
                case L'm':
                    {
                        const wstring &word = checkLettersToken(tokens, 1, iGame.getDic());
                        const wstring &coord = checkAlphaNumToken(tokens, 2);
                        if (word == L"" || coord == L"")
                        {
                            helpArbitration();
                            break;
                        }
                        const Move &move = iGame.arbitrationCheckWord(word, coord);
                        iGame.duplicateSetMasterMove(move);
                    }
                    break;
                case L'e':
                    {
                        const wstring &id = checkNumToken(tokens, 1);
                        const wstring &type = checkAlphaToken(tokens, 2);
                        const wstring &value = checkNumToken(tokens, 3);
                        if (id == L"" || type == L"" || value == L"")
                        {
                            helpArbitration();
                            break;
                        }
                        int n = wtoi(id.c_str());
                        if (n < 0 || n >= (int)iGame.getNbPlayers())
                        {
                            fprintf(stderr, "Numéro de joueur invalide\n");
                            break;
                        }
                        switch (type[0])
                        {
                            case L'w':
                                iGame.arbitrationToggleWarning(n);
                                break;
                            case L'p':
                                iGame.arbitrationAddPenalty(n, wtoi(value.c_str()));
                                break;
//                             case L's':
//                                 iGame.arbitrationSetSolo(n, value);
//                                 break;
                            default:
                                fprintf(stderr, "Invalid event type\n");
                                break;
                        }
                    }
                    break;
                case L't':
                    {
                        const wstring &letters =
                            checkLettersToken(tokens, 1, iGame.getDic());
                        if (letters == L"")
                            helpArbitration();
                        else
                            iGame.arbitrationSetRackManual(letters);
                    }
                    break;
                case L'*':
                    iGame.arbitrationSetRackRandom();
                    break;
                case L'q':
                    quit = true;
                    break;
                default:
                    printf("commande inconnue\n");
                    break;
            }
        }
        catch (std::exception &e)
        {
            printf("%s\n", e.what());
        }
    }
    printf("fin du mode arbitrage\n");
}


void mainLoop(const Dictionary &iDic)
{
    printf("[?] pour l'aide\n");

    bool quit = false;
    while (!quit)
    {
        wstring command = rl_gets();
        // Split the command
        vector<wstring> tokens;
        boost::char_separator<wchar_t> sep(L" ");
        Tokenizer tok(command, sep);
        BOOST_FOREACH(const wstring &wstr, tok)
        {
            tokens.push_back(wstr);
        }

        if (tokens.empty())
            continue;
        if (tokens[0].size() > 3)
        {
            printf("%s\n", "Invalid command");
            continue;
        }

        try
        {
            switch (tokens[0][0])
            {
                case L'?':
                    help();
                    break;
                case L'c':
                    {
                        const wstring &wfileName = checkFileNameToken(tokens, 1);
                        if (wfileName != L"")
                        {
                            string filename = lfw(wfileName);
                            try
                            {
                                PublicGame *game = PublicGame::load(filename, iDic);
                                switch (game->getMode())
                                {
                                    case PublicGame::kTRAINING:
                                        loopTraining(*game);
                                        break;
                                    case PublicGame::kFREEGAME:
                                        loopFreegame(*game);
                                        break;
                                    case PublicGame::kDUPLICATE:
                                        loopDuplicate(*game);
                                        break;
                                    case PublicGame::kARBITRATION:
                                        loopArbitration(*game);
                                        break;
                                }
                                //GameFactory::Instance()->releaseGame(*game);
                                delete game;
                            }
                            catch (const std::exception &e)
                            {
                                string msg = string("Error loading the game: ") + e.what();
                                printf("%s\n", msg.c_str());
                            }
                        }
                    }
                    break;
                case L'e':
                    {
                        // New training game
                        PublicGame *game = readGame(iDic, GameParams::kTRAINING, tokens[0]);
                        game->addPlayer(new HumanPlayer);
                        game->start();
                        loopTraining(*game);
                        //GameFactory::Instance()->releaseGame(*game);
                        delete game;
                    }
                    break;
                case L'd':
                    {
                        if (tokens.size() != 3)
                        {
                            help();
                            break;
                        }
                        const wstring &nbHuman = checkNumToken(tokens, 1);
                        const wstring &nbAI = checkNumToken(tokens, 2);
                        if (nbHuman == L"" || nbAI == L"")
                        {
                            help();
                            break;
                        }
                        // New duplicate game
                        PublicGame *game = readGame(iDic, GameParams::kDUPLICATE, tokens[0]);
                        for (int i = 0; i < wtoi(nbHuman.c_str()); ++i)
                            game->addPlayer(new HumanPlayer);
                        for (int i = 0; i < wtoi(nbAI.c_str()); ++i)
                            game->addPlayer(new AIPercent(1));
                        game->start();
                        loopDuplicate(*game);
                        //GameFactory::Instance()->releaseGame(*game);
                        delete game;
                    }
                    break;
                case L'l':
                    {
                        if (tokens.size() != 3)
                        {
                            help();
                            break;
                        }
                        const wstring &nbHuman = checkNumToken(tokens, 1);
                        const wstring &nbAI = checkNumToken(tokens, 2);
                        if (nbHuman == L"" || nbAI == L"")
                        {
                            help();
                            break;
                        }
                        // New free game
                        PublicGame *game = readGame(iDic, GameParams::kFREEGAME, tokens[0]);
                        for (int i = 0; i < wtoi(nbHuman.c_str()); i++)
                            game->addPlayer(new HumanPlayer);
                        for (int i = 0; i < wtoi(nbAI.c_str()); i++)
                            game->addPlayer(new AIPercent(1));
                        game->start();
                        loopFreegame(*game);
                        //GameFactory::Instance()->releaseGame(*game);
                        delete game;
                    }
                    break;
                case L'a':
                    {
                        if (tokens.size() != 3)
                        {
                            help();
                            break;
                        }
                        const wstring &nbHuman = checkNumToken(tokens, 1);
                        const wstring &nbAI = checkNumToken(tokens, 2);
                        if (nbHuman == L"" || nbAI == L"")
                        {
                            help();
                            break;
                        }
                        // New free game
                        PublicGame *game = readGame(iDic, GameParams::kARBITRATION, tokens[0]);
                        for (int i = 0; i < wtoi(nbHuman.c_str()); i++)
                            game->addPlayer(new HumanPlayer);
                        for (int i = 0; i < wtoi(nbAI.c_str()); i++)
                            game->addPlayer(new AIPercent(1));
                        game->start();
                        loopArbitration(*game);
                        //GameFactory::Instance()->releaseGame(*game);
                        delete game;
                    }
                    break;
                case L'x':
                    // Regular expression tests
                    handleRegexp(iDic, tokens);
                    break;
                case L's':
                    setSetting(tokens);
                    break;
                case L'q':
                    quit = 1;
                    break;
                default:
                    printf("commande inconnue\n");
                    break;
            }
        }
        catch (std::exception &e)
        {
            printf("%s\n", e.what());
        }
    }
}


int main(int argc, char *argv[])
{
    // Let the user choose the locale
    setlocale(LC_ALL, "");

    if (argc != 2 && argc != 3)
    {
        fprintf(stdout, "Usage: eliot /chemin/vers/ods5.dawg [random_seed]\n");
        exit(1);
    }

    try
    {
        Dictionary dic(argv[1]);

        unsigned int seed;
        if (argc == 3)
            seed = atoi(argv[2]);
        else
            seed = time(NULL);
        srand(seed);
        cerr << "Using seed: " << seed << endl;

        mainLoop(dic);
        GameFactory::Destroy();

        // Free the readline static variable
        if (wline_read)
            delete[] wline_read;
    }
    catch (std::exception &e)
    {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}

