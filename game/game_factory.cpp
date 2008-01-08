/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2007 Olivier Teulière & Antoine Fraboulet
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
 *          Antoine Fraboulet <antoine.fraboulet @@ free.fr>
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

#include <getopt.h>
#include <string>
#include <fstream>
#include <exception>

#include "game_factory.h"
#include "game.h"
#include "training.h"
#include "freegame.h"
#include "duplicate.h"
#include "dic.h"


GameFactory *GameFactory::m_factory = NULL;


GameFactory::GameFactory(): m_dic(NULL), m_human(0), m_ai(0), m_joker(false)
{
}


GameFactory::~GameFactory()
{
    delete m_dic;
}


GameFactory *GameFactory::Instance()
{
    if (m_factory == NULL)
        m_factory = new GameFactory;
    return m_factory;
}


void GameFactory::Destroy()
{
    delete m_factory;
    m_factory = NULL;
}


Training *GameFactory::createTraining(const Dictionary &iDic)
{
    Training *game = new Training(iDic);
    return game;
}


FreeGame *GameFactory::createFreeGame(const Dictionary &iDic)
{
    FreeGame *game = new FreeGame(iDic);
    return game;
}


Duplicate *GameFactory::createDuplicate(const Dictionary &iDic)
{
    Duplicate *game = new Duplicate(iDic);
    return game;
}


Game *GameFactory::createFromCmdLine(int argc, char **argv)
{
    // 1) Parse command-line and store everything in member variables
    static struct option long_options[] =
    {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {"dictionary", required_argument, NULL, 'd'},
        {"dict", required_argument, NULL, 'd'},
        {"mode", required_argument, NULL, 'm'},
        {"human", no_argument, NULL, 300},
        {"ai", no_argument, NULL, 400},
        {"joker", no_argument, NULL, 500},
        {0, 0, 0, 0}
    };
    static char short_options[] = "hvd:m:";

    int option_index = 1;
    int res;
    bool found_d = false;
    bool found_m = false;
    while ((res = getopt_long(argc, argv, short_options,
                              long_options, &option_index)) != -1)
    {
        switch (res)
        {
        case 'h':
            // Help requested, display it and exit
            printUsage(argv[0]);
            return NULL;
        case 'v':
            // Version requested, display it and exit
            printVersion();
            return NULL;
        case 'd':
            m_dicStr = optarg;
            found_d = true;
            break;
        case 'm':
            m_modeStr = optarg;
            found_m = true;
            break;
        case 300:
            m_human++;
            break;
        case 400:
            m_ai++;
            break;
        case 500:
            m_joker = true;
            break;
        }
    }

    // 2) Make sure the mandatory options are present
    if (!found_d || !found_m)
    {
        cerr << "Mandatory option missing: ";
        if (!found_d)
            cerr << "dict";
        else if (!found_m)
            cerr << "mode";
        cerr << endl;

        printUsage(argv[0]);
        return NULL;
    }

    // 3) Try to load the dictionary
    try
    {
        m_dic = new Dictionary(m_dicStr);
    }
    catch (std::exception &e)
    {
        cerr << e.what() << endl;
        return NULL;
    }

    // 4) Try to create a game object
    Game *game = NULL;
    if (m_modeStr == "training" || m_modeStr == "t")
    {
        game = createTraining(*m_dic);
    }
    else if (m_modeStr == "freegame" || m_modeStr == "f")
    {
        game = createFreeGame(*m_dic);
    }
    else if (m_modeStr == "duplicate" || m_modeStr == "d")
    {
        game = createDuplicate(*m_dic);
    }
    else
    {
        cerr << "Invalid game mode '" << m_modeStr << "'" << endl;
        return NULL;
    }

    // 5) Add the players
    for (int i = 0; i < m_human; i++)
        game->addHumanPlayer();
    for (int i = 0; i < m_ai; i++)
        game->addAIPlayer();

    // 6) Set the variant
    if (m_joker)
        game->setVariant(Game::kJOKER);

    return game;
}


Game* GameFactory::load(const string &iFileName, const Dictionary &iDic)
{
    FILE* fin = fopen(iFileName.c_str(), "r");
    if (fin == NULL)
    {
        printf("Cannot open %s\n", iFileName.c_str());
        return NULL;
    }
    Game *game = Game::load(fin, iDic);
    fclose(fin);
    return game;
}


void GameFactory::releaseGame(Game &iGame)
{
    delete &iGame;
}


void GameFactory::printUsage(const string &iBinaryName) const
{
    cout << "Usage: " << iBinaryName << " [options]" << endl
         << "Options:" << endl
         << "  -h, --help               Print this help and exit" << endl
         << "  -v, --version            Print version information and exit" << endl
         << "  -m, --mode {duplicate,d,freegame,f,training,t}" << endl
         << "                           Choose game mode (mandatory)" << endl
         << "  -d, --dict <string>      Choose a dictionary (mandatory)" << endl
         << "      --human              Add a human player" << endl
         << "      --ai                 Add a AI (Artificial Intelligence) player" << endl
         << "      --joker              Play with the \"Joker game\" variant" << endl;
}


void GameFactory::printVersion() const
{
    cout << PACKAGE_STRING << endl
         << "This program comes with NO WARRANTY, to the extent permitted by "
         << "law." << endl << "You may redistribute it under the terms of the "
         << "GNU General Public License;" << endl
         << "see the file named COPYING for details." << endl;
}


/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
