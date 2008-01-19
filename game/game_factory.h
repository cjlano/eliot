/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2007 Olivier Teulière
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
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

#ifndef _GAME_FACTORY_H_
#define _GAME_FACTORY_H_

#include <string>
#include <vector>

using std::string;
using std::wstring;
using std::vector;
using std::pair;

class Dictionary;
class Game;
class Training;
class FreeGame;
class Duplicate;


/**
 * This class is the entry point to create Game objects, either directly or
 * using command-line parameters. It also offers a method to destroy Game
 * objects.
 * This class implements the Singleton pattern.
 */
class GameFactory
{
public:
    static GameFactory *Instance();
    static void Destroy();

    /*************************
     * Functions to create and destroy a game
     * The dictionary does not belong to the
     * game (ie: it won't be destroyed by ~Game)
     *************************/
    Training *createTraining(const Dictionary &iDic);
    FreeGame *createFreeGame(const Dictionary &iDic);
    Duplicate *createDuplicate(const Dictionary &iDic);

    /**
     * load() returns the loaded game, or NULL if there was a problem
     * load() might need some more work to be robust enough to
     * handle "hand written" files
     */
    Game *load(const string &iFileName, const Dictionary &iDic);

    Game *createFromCmdLine(int argc, char **argv);

    /// Destroy a Game object, created by any of the createXXX methods above
    void releaseGame(Game &iGame);

private:

    GameFactory();
    ~GameFactory();

    /// The unique instance of the class
    static GameFactory *m_factory;

    /// Initial dictionary (it could be changed later)
    Dictionary *m_dic;

    /** Parameters specified on the command-line */
    //@{

    /// Dictionary
    string m_dicStr;

    /// Game mode
    string m_modeStr;

    /// Variant of the game
    bool m_joker;

    typedef pair<bool, wstring> PlayerDesc;
    vector<PlayerDesc> m_players;

    //@}

    /// Print command-line usage
    void printUsage(const string &iBinaryName) const;

    /// Print version
    void printVersion() const;

};

#endif // _GAME_FACTORY_H_

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
