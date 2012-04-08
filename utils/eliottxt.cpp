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
#include "base_exception.h"
#include "settings.h"
#include "move.h"

class Game;


// Use a more friendly type name for the tokenizer
typedef boost::tokenizer<boost::char_separator<wchar_t>,
        std::wstring::const_iterator,
        std::wstring> Tokenizer;

// A static variable for holding the line
static wchar_t *wline_read = NULL;

// Read a string, and return a pointer to it.
// Returns NULL on EOF.
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


vector<wstring> readTokens()
{
    wstring command = rl_gets();
    // Split the command
    vector<wstring> tokens;
    boost::char_separator<wchar_t> sep(L" ");
    Tokenizer tok(command, sep);
    BOOST_FOREACH(const wstring &wstr, tok)
    {
        if (wstr != L"")
            tokens.push_back(wstr);
    }
    return tokens;
}


class ParsingException : public BaseException
{
    public:
        ParsingException(const string &s) : BaseException(s) {}
};


wstring parseAlpha(const vector<wstring> &tokens, uint8_t index)
{
    if (tokens.size() <= index)
        throw ParsingException("Not enough tokens");
    const wstring &wstr = tokens[index];
    BOOST_FOREACH(wchar_t wch, wstr)
    {
        if (!iswalpha(wch))
            throw ParsingException("Not an alphabetical character: " + lfw(wch));
    }
    return wstr;
}


int parseNum(const vector<wstring> &tokens, uint8_t index,
             bool acceptDefault = false, int defValue = -1)
{
    if (tokens.size() <= index)
    {
        if (acceptDefault)
            return defValue;
        throw ParsingException("Not enough tokens");
    }
    const wstring &wstr = tokens[index];
    BOOST_FOREACH(wchar_t wch, wstr)
    {
        if (!iswdigit(wch))
            throw ParsingException("Not a numeric character: " + lfw(wch));
    }
    int value = wtoi(wstr.c_str());
    return value;
}


wstring parseAlphaNum(const vector<wstring> &tokens, uint8_t index)
{
    if (tokens.size() <= index)
        throw ParsingException("Not enough tokens");
    const wstring &wstr = tokens[index];
    BOOST_FOREACH(wchar_t wch, wstr)
    {
        if (!iswalnum(wch))
            throw ParsingException("Not an alphanumeric character: " + lfw(wch));
    }
    return wstr;
}


wstring parseLetters(const vector<wstring> &tokens, uint8_t index,
                     const Dictionary &iDic)
{
    if (tokens.size() <= index)
        throw ParsingException("Not enough tokens");
    return iDic.convertFromInput(tokens[index]);
}


wchar_t parseCharInList(const vector<wstring> &tokens, uint8_t index,
                        const wstring &allowed)
{
    if (tokens.size() <= index)
        throw ParsingException("Not enough tokens");
    const wstring &wstr = tokens[index];
    if (wstr.size() != 1)
        throw ParsingException("Not an allowed value: " + lfw(wstr));
    if (allowed.find(wstr[0]) == string::npos)
        throw ParsingException("Not an allowed value: " + lfw(wstr));
    return wstr[0];
}


int parsePlayerId(const vector<wstring> &tokens,
                  uint8_t index, const PublicGame &iGame)
{
    int playerId = parseNum(tokens, index);
    if (playerId < 0 || playerId >= (int)iGame.getNbPlayers())
        throw ParsingException("Invalid player ID");
    return playerId;
}


wstring parseFileName(const vector<wstring> &tokens, uint8_t index)
{
    if (tokens.size() <= index)
        throw ParsingException("Not enough tokens");
    const wstring &wstr = tokens[index];
    BOOST_FOREACH(wchar_t wch, wstr)
    {
        if (!iswalnum(wch) && wch != L'.' && wch != L'_')
            throw ParsingException("Invalid file name");
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
    printf("  a [g|gm|gd|l|p|s|S|t|T] : afficher :\n");
    printf("            g -- grille\n");
    printf("            gm -- grille + valeur des cases\n");
    printf("            gd -- grille + debug cross (debug only)\n");
    printf("            l -- lettres non jouées\n");
    printf("            p -- partie\n");
    printf("            r -- recherche\n");
    printf("            s -- score\n");
    printf("            S -- score de tous les joueurs\n");
    printf("            t -- tirage\n");
    printf("            T -- tirage de tous les joueurs\n");
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
    printf("  a [g|gm|gd|l|p|s|S|t|T] : afficher :\n");
    printf("            g -- grille\n");
    printf("            gm -- grille + valeur des cases\n");
    printf("            gd -- grille + debug cross (debug only)\n");
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
    printf("  a [g|gm|gd|l|p|s|S|t|T] : afficher :\n");
    printf("            g -- grille\n");
    printf("            gm -- grille + valeur des cases\n");
    printf("            gd -- grille + debug cross (debug only)\n");
    printf("            l -- lettres non jouées\n");
    printf("            p -- partie\n");
    printf("            s -- score\n");
    printf("            S -- score de tous les joueurs\n");
    printf("            t -- tirage\n");
    printf("            T -- tirage de tous les joueurs\n");
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
    printf("  a [g|gm|gd|l|p|s|S|t|T] : afficher :\n");
    printf("            g -- grille\n");
    printf("            gm -- grille + valeur des cases\n");
    printf("            gd -- grille + debug cross (debug only)\n");
    printf("            l -- lettres non jouées\n");
    printf("            p -- partie\n");
    printf("            s -- score\n");
    printf("            S -- score de tous les joueurs\n");
    printf("            t -- tirage\n");
    printf("            T -- tirage de tous les joueurs\n");
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
    const wstring &displayType = parseAlpha(tokens, 1);
    if (displayType == L"g")
        GameIO::printBoard(cout, iGame);
    else if (displayType == L"gd")
        GameIO::printBoardDebug(cout, iGame);
    else if (displayType == L"gm")
        GameIO::printBoardMultipliers(cout, iGame);
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
    else if (displayType == L"r")
    {
        int limit = parseNum(tokens, 2, true, 10);
        GameIO::printSearchResults(cout, iGame.trainingGetResults(), limit);
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
        throw ParsingException("Invalid command");
}


void commonCommands(PublicGame &iGame, const vector<wstring> &tokens)
{
    wchar_t command = parseCharInList(tokens, 0, L"adhjs");
    if (command == L'a')
        displayData(iGame, tokens);
    else if (command == L'd')
    {
        const wstring &word = parseLetters(tokens, 1, iGame.getDic());
        if (iGame.getDic().searchWord(word))
            printf("le mot -%s- existe\n", lfw(word).c_str());
        else
            printf("le mot -%s- n'existe pas\n", lfw(word).c_str());
    }
    else if (command == L'h')
    {
        wchar_t action = parseCharInList(tokens, 1, L"pnflr");
        int count = parseNum(tokens, 2, true, 1);
        if (action == L'p')
        {
            for (int i = 0; i < count; ++i)
                iGame.prevTurn();
        }
        else if (action == L'n')
        {
            for (int i = 0; i < count; ++i)
                iGame.nextTurn();
        }
        else if (action == L'f')
            iGame.firstTurn();
        else if (action == L'l')
            iGame.lastTurn();
        else if (action == L'r')
            iGame.clearFuture();
    }
    else if (command == L'j')
    {
        const wstring &word = parseLetters(tokens, 1, iGame.getDic());
        const wstring &coord = parseAlphaNum(tokens, 2);
        int res = iGame.play(word, coord);
        if (res != 0)
            printf("Mot incorrect ou mal placé (%i)\n", res);
    }
    else if (command == L's')
    {
        const wstring &fileName = parseFileName(tokens, 1);
        try
        {
            iGame.save(lfw(fileName));
        }
        catch (std::exception &e)
        {
            printf("Cannot save game to %ls: %s\n", fileName.c_str(), e.what());
            return;
        }
    }
}


void handleRegexp(const Dictionary& iDic, const vector<wstring> &tokens)
{
    const wstring &regexp = tokens[1];
    int nres = parseNum(tokens, 2, true, 50);
    int lmin = parseNum(tokens, 3, true, 1);
    int lmax = parseNum(tokens, 4, true, DIC_WORD_MAX - 1);

    if (regexp == L"")
        return;

    if (lmax > (DIC_WORD_MAX - 1) || lmin < 1 || lmin > lmax)
    {
        printf("bad length -%d,%d-\n", lmin, lmax);
        return;
    }

    printf("search for %ls (%d,%d,%d)\n", regexp.c_str(),
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
    printf("%u printed results\n", (unsigned)wordList.size());
}


void setSetting(const vector<wstring> &tokens)
{
    wchar_t type = parseCharInList(tokens, 1, L"bi");
    const wstring &settingWide = tokens[2];
    int value = parseNum(tokens, 3);

    try
    {
        string setting = lfw(settingWide);
        if (type == L'i')
            Settings::Instance().setInt(setting, value);
        else if (type == L'b')
            Settings::Instance().setBool(setting, value);
    }
    catch (GameException &e)
    {
        printf("Error while changing a setting: %s\n", e.what());
        return;
    }
}


void loopTraining(PublicGame &iGame)
{
    cout << "mode entraînement" << endl;
    cout << "[?] pour l'aide" << endl;

    bool quit = false;
    while (!quit)
    {
        const vector<wstring> &tokens = readTokens();
        if (tokens.empty())
            continue;
        try
        {
            wchar_t command = parseCharInList(tokens, 0, L"?adhjsbnrt*+q");
            if (command == L'?')
                helpTraining();
            else if (command == L'b')
            {
                wchar_t type = parseCharInList(tokens, 1, L"bpr");
                const wstring &word = parseLetters(tokens, 2, iGame.getDic());
                if (type == L'b')
                {
                    vector<wdstring> wordList;
                    iGame.getDic().searchBenj(word, wordList);
                    BOOST_FOREACH(const wdstring &wstr, wordList)
                    {
                        cout << lfw(wstr) << endl;
                    }
                }
                else if (type == L'p')
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
                else if (type == L'r')
                {
                    vector<wdstring> wordList;
                    iGame.getDic().searchRacc(word, wordList);
                    BOOST_FOREACH(const wdstring &wstr, wordList)
                    {
                        cout << lfw(wstr) << endl;
                    }
                }
            }
            else if (command == L'n')
            {
                int num = parseNum(tokens, 1);
                if (num <= 0)
                    printf("mauvais argument\n");
                iGame.trainingPlayResult(num - 1);
            }
            else if (command == L'r')
                iGame.trainingSearch();
            else if (command == L't')
            {
                const wstring &letters =
                    parseLetters(tokens, 1, iGame.getDic());
                iGame.trainingSetRackManual(false, letters);
            }
            else if (command == L'*')
                iGame.trainingSetRackRandom(false, PublicGame::kRACK_ALL);
            else if (command == L'+')
                iGame.trainingSetRackRandom(false, PublicGame::kRACK_NEW);
            else if (command == L'q')
                quit = true;
            else
                commonCommands(iGame, tokens);
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
        const vector<wstring> &tokens = readTokens();
        if (tokens.empty())
            continue;
        try
        {
            wchar_t command = parseCharInList(tokens, 0, L"?adhjspq");
            if (command == L'?')
                helpFreegame();
            else if (command == L'p')
            {
                wstring letters = L"";
                // You can pass your turn without changing any letter
                if (tokens.size() > 1)
                {
                    letters = parseLetters(tokens, 1, iGame.getDic());
                }
                int res = iGame.freeGamePass(letters);
                if (res != 0)
                    printf("Cannot pass (%d)\n", res);
            }
            else if (command == L'q')
                quit = true;
            else
                commonCommands(iGame, tokens);
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
        const vector<wstring> &tokens = readTokens();
        if (tokens.empty())
            continue;
        try
        {
            wchar_t command = parseCharInList(tokens, 0, L"?adhjsnq");
            if (command == L'?')
                helpDuplicate();
            else if (command == L'n')
            {
                unsigned id = parsePlayerId(tokens, 1, iGame);
                iGame.duplicateSetPlayer(id);
            }
            else if (command == L'q')
                quit = true;
            else
                commonCommands(iGame, tokens);
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
        const vector<wstring> &tokens = readTokens();
        if (tokens.empty())
            continue;
        try
        {
            wchar_t command = parseCharInList(tokens, 0, L"?adhjsnq");
            if (command == L'?')
                helpArbitration();
            else if (command == L'f')
                iGame.arbitrationFinalizeTurn();
            else if (command == L'j')
            {
                unsigned id = parsePlayerId(tokens, 1, iGame);
                const wstring &word = parseLetters(tokens, 2, iGame.getDic());
                const wstring &coord = parseAlphaNum(tokens, 3);
                const Move &move = iGame.arbitrationCheckWord(word, coord);
                iGame.arbitrationAssign(id, move);
            }
            else if (command == L'm')
            {
                const wstring &word = parseLetters(tokens, 1, iGame.getDic());
                const wstring &coord = parseAlphaNum(tokens, 2);
                const Move &move = iGame.arbitrationCheckWord(word, coord);
                iGame.duplicateSetMasterMove(move);
            }
            else if (command == L'e')
            {
                unsigned id = parsePlayerId(tokens, 1, iGame);
                wchar_t type = parseCharInList(tokens, 2, L"wp");
                int value = parseNum(tokens, 3);
                if (type == L'w')
                    iGame.arbitrationToggleWarning(id);
                else if (type == 'p')
                    iGame.arbitrationAddPenalty(id, value);
//                 else if (type == 's')
//                     iGame.arbitrationSetSolo(id, value);
            }
            else if (command == L't')
            {
                const wstring &letters =
                    parseLetters(tokens, 1, iGame.getDic());
                iGame.arbitrationSetRackManual(letters);
            }
            else if (command == L'*')
                iGame.arbitrationSetRackRandom();
            else if (command == L'q')
                quit = true;
            else
                commonCommands(iGame, tokens);
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
        const vector<wstring> &tokens = readTokens();
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
                        const string &fileName = lfw(parseFileName(tokens, 1));
                        try
                        {
                            PublicGame *game = PublicGame::load(fileName, iDic);
                            if (game->getMode() == PublicGame::kTRAINING)
                                loopTraining(*game);
                            else if (game->getMode() == PublicGame::kFREEGAME)
                                loopFreegame(*game);
                            else if (game->getMode() == PublicGame::kDUPLICATE)
                                loopDuplicate(*game);
                            else if (game->getMode() == PublicGame::kARBITRATION)
                                loopArbitration(*game);
                            //GameFactory::Instance()->releaseGame(*game);
                            delete game;
                        }
                        catch (const std::exception &e)
                        {
                            printf("Error loading the game: %s\n", e.what());
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
                        int nbHuman = parseNum(tokens, 1);
                        int nbAI = parseNum(tokens, 2);
                        // New duplicate game
                        PublicGame *game = readGame(iDic, GameParams::kDUPLICATE, tokens[0]);
                        for (int i = 0; i < nbHuman; i++)
                            game->addPlayer(new HumanPlayer);
                        for (int i = 0; i < nbAI; i++)
                            game->addPlayer(new AIPercent(1));
                        game->start();
                        loopDuplicate(*game);
                        //GameFactory::Instance()->releaseGame(*game);
                        delete game;
                    }
                    break;
                case L'l':
                    {
                        int nbHuman = parseNum(tokens, 1);
                        int nbAI = parseNum(tokens, 2);
                        // New free game
                        PublicGame *game = readGame(iDic, GameParams::kFREEGAME, tokens[0]);
                        for (int i = 0; i < nbHuman; i++)
                            game->addPlayer(new HumanPlayer);
                        for (int i = 0; i < nbAI; i++)
                            game->addPlayer(new AIPercent(1));
                        game->start();
                        loopFreegame(*game);
                        //GameFactory::Instance()->releaseGame(*game);
                        delete game;
                    }
                    break;
                case L'a':
                    {
                        int nbHuman = parseNum(tokens, 1);
                        int nbAI = parseNum(tokens, 2);
                        // New free game
                        PublicGame *game = readGame(iDic, GameParams::kARBITRATION, tokens[0]);
                        for (int i = 0; i < nbHuman; i++)
                            game->addPlayer(new HumanPlayer);
                        for (int i = 0; i < nbAI; i++)
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

