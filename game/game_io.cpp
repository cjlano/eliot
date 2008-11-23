/*****************************************************************************
 * Eliot
 * Copyright (C) 2002-2008 Antoine Fraboulet & Olivier Teulière
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

#include <cstring>
#include <cstdlib> // For atoi

#include "dic.h"
#include "pldrack.h"
#include "round.h"
#include "turn.h"
#include "player.h"
#include "ai_percent.h"
#include "game.h"
#include "game_factory.h"
#include "training.h"
#include "freegame.h"
#include "duplicate.h"
#include "encoding.h"
#include "game_exception.h"
#include "game_move_cmd.h"

using namespace std;

/*************************
 * Ident string used to identify saved Eliot games
 *************************/
#define IDENT_STRING "Eliot"
#define IDENT_FORMAT_14 ""
#define IDENT_FORMAT_15 "1.5"


/********************************************************
 *
 * Loading games
 *
 ********************************************************/

Game * Game::load(FILE *fin, const Dictionary& iDic)
{
    char buff[4096];
    // 10d is \012
    // 13d is \015
    char delim[] = " \t\n\012\015|";
    char *token;

    // Check characteristic string
    if (fgets(buff, sizeof(buff), fin) == NULL)
    {
        throw GameException("Cannot recognize the first line");
    }

    if ((token = strtok(buff, delim)) == NULL)
    {
        throw GameException("The first line is empty");
    }

    /* checks for IDENT_STRING and file format */
    if (string(token) != IDENT_STRING)
    {
        throw GameException("Invalid identity string: " + string(token));
    }

    if ((token = strtok(NULL, delim)) == NULL)
    {
        return Game::gameLoadFormat_14(fin,iDic);
    }

    if (string(token) == string(IDENT_FORMAT_15))
    {
        return Game::gameLoadFormat_15(fin,iDic);
    }

    throw GameException("Unknown format: " + string(token));
}


Game* Game::gameLoadFormat_14(FILE *fin, const Dictionary& iDic)
{
    int ret = 0;
    int line = 0;
    Tile tile;
    char buff[4096];
    char rack[20];
    char word[20];
    char pos [5];
    char delim[]=" \t\n\012\015";
    char *token;
    Game *pGame = NULL;

    pGame = GameFactory::Instance()->createTraining(iDic);
    pGame->start();

    /*    rack        word          ?bonus    pts  coord    */
    /*    EUOFMIE     FUMEE          *        26   H  4     */

    /* read all turns until total */
    while (fgets(buff, sizeof(buff), fin))
    {
        line++;
        token = strtok(buff, delim);
        if (token != NULL)
        {
            if (strcmp(token, "total") == 0)
            {
                break;
            }

            /* rack */
            strncpy(rack, token, sizeof(rack));
            static_cast<Training*>(pGame)->setRackManual(false, convertToWc(rack));

            /* word */
            token = strtok(NULL, delim);
            if (!token || strcmp(token, "total") == 0)
            {
                break;
            }

            strncpy(word, token, sizeof(word));

            /* bonus */
            if ((token = strtok(NULL, delim)) == NULL)
                break;

            /* points */
            if (token[0] == '*')
            {
                if ((token = strtok(NULL, delim)) == NULL)
                    break;
            }

            /* pos 1 */
            if ((token = strtok(NULL, delim)) == NULL)
                break;

            //debug("(%s ", token);
            strncpy(pos, token, sizeof(pos));

            /* pos 2 */
            if ((token = strtok(NULL, delim)) == NULL)
                break;

            //debug("%s)", token);
            strncat(pos, token, sizeof(pos));

            if ((ret = pGame->play(convertToWc(pos), convertToWc(word))))
            {
                GameFactory::Instance()->releaseGame(*pGame);
                char tmp1[10];
                snprintf(tmp1, 10, "%d", ret);
                char tmp2[10];
                snprintf(tmp2, 10, "%d", ret);
                throw GameException("Loading error " + string(tmp1) +
                                    " on line " + string(tmp2));
            }
        }
    }
    return pGame;
}


Game* Game::gameLoadFormat_15(FILE *fin, const Dictionary& iDic)
{
    Game *pGame = NULL;

    char buff[4096];
    char *pos;

    /*************/
    /* Game type */
    /*************/

    while (fgets(buff, sizeof(buff), fin))
    {
        // Indication of game type
        pos = strstr(buff, "Game type: ");
        if (pos != NULL)
        {
            // No Game object should have been created yet
            if (pGame != NULL)
            {
                delete pGame;
                return NULL;
            }
            // Create the correct Game object
            if (strstr(buff, "Training"))
            {
                pGame = GameFactory::Instance()->createTraining(iDic);
                break;
            }
            else if (strstr(buff, "Free game"))
            {
                pGame = GameFactory::Instance()->createFreeGame(iDic);
                break;
            }
            else if (strstr(buff, "Duplicate"))
            {
                pGame = GameFactory::Instance()->createDuplicate(iDic);
                break;
            }
            else
            {
                throw GameException("Unknown game type");
            }
        }
    }

    /***************/
    /* Player List */
    /***************/

    while (fgets(buff, sizeof(buff), fin))
    {
        // Players type
        pos = strstr(buff, "Player ");
        if (pos != NULL)
        {
            int nb = 0;
            char type[20];
            if (sscanf(pos, "Player %d: %19s", &nb, type) > 1)
            {
                if (string(type) == "Human")
                {
                    pGame->addPlayer(new HumanPlayer);
                }
                else if (string(type) == "Computer")
                {
                    if (pGame->getMode() == kTRAINING)
                    {
                        break;
                    }
                    else
                    {
                        pGame->addPlayer(new AIPercent(1));
                    }
                }
                else
                {
                    delete pGame;
                    return NULL;
                }
            }
        }
        else if (strstr(buff," N |   RACK   "))
        {
            break;
        }
    }

    /*************/
    /* Turn list */
    /*************/

    while (fgets(buff, sizeof(buff), fin))
    {
        // Skip columns title
        if (strstr(buff,"| PTS | P |") != NULL)
        {
            continue;
        }

        // Skip columns title
        if (strstr(buff, "==") != NULL)
        {
            continue;
        }

        if (string(buff) == "\n")
        {
            continue;
        }


        int num;
        char rack[20];
        char tmpWord[20];
        char ref[4];
        int pts;
        unsigned int player;
        char bonus = 0;
        int res = sscanf(buff, "   %2d | %8s | %s | %3s | %3d | %1u | %c",
                         &num, rack, tmpWord, ref, &pts, &player, &bonus);

        if (res < 6)
        {
            continue;
        }

        //debug("              %2d | %8s | %s | %3s | %3d | %1d | %c \n",
        //      num, rack, tmpWord, ref, pts, player, bonus);

        // Integrity checks
        // TODO: add more checks
        if (pts < 0)
        {
            continue;
        }
        if (player > pGame->getNPlayers())
        {
            continue;
        }
        if (bonus && bonus != '*')
        {
            continue;
        }

        // Build a rack for the correct player
        PlayedRack pldrack;
        if (!iDic.validateLetters(convertToWc(rack)))
        {
            throw GameException("Rack invalid for the current dictionary");
        }
        pldrack.setManual(convertToWc(rack));

        // Build a round
        Round round;
        round.setPoints(pts);
        if (bonus == '*')
            round.setBonus(1);

        wstring word = convertToWc(tmpWord);
        Tile tile;
        if (isalpha(ref[0]))
        {
            // Horizontal word
            round.accessCoord().setDir(Coord::HORIZONTAL);
            round.accessCoord().setRow(ref[0] - 'A' + 1);
            round.accessCoord().setCol(atoi(ref + 1));

            for (unsigned int i = 0; i < word.size(); i++)
            {
                tile = Tile(word[i]);

                if (!pGame->m_board.getTile(round.getCoord().getRow(), round.getCoord().getCol() + i).isEmpty())
                {
                    round.addRightFromBoard(tile);
                }
                else
                {
                    round.addRightFromRack(tile, iswlower(word[i]));
                    pGame->m_bag.takeTile((iswlower(word[i])) ? Tile::Joker() : tile);
                }
            }
        }
        else
        {
            // Vertical word
            round.accessCoord().setDir(Coord::VERTICAL);
            round.accessCoord().setRow(ref[strlen(ref) - 1] - 'A' + 1);
            round.accessCoord().setCol(atoi(ref));

            for (unsigned int i = 0; i < word.size(); i++)
            {
                tile = Tile(word[i]);

                if (!pGame->m_board.getTile(round.getCoord().getRow() + i, round.getCoord().getCol()).isEmpty())
                {
                    round.addRightFromBoard(tile);
                }
                else
                {
                    round.addRightFromRack(tile, iswlower(word[i]));
                    pGame->m_bag.takeTile((iswlower(word[i])) ? Tile::Joker() : tile);
                }
            }
        }

//                     pGame->m_currPlayer = player;
//                     // Update the rack for the player
//                     pGame->m_players[player]->setCurrentRack(pldrack);
//                     // End the turn for the current player (this creates a new rack)
//                     pGame->m_players[player]->endTurn(round,num - 1);

        // Play the round
        GameMoveCmd cmd(*pGame, Move(round),
                        pGame->getCurrentPlayer().getLastRack(),
                        pGame->m_currPlayer);
        cmd.execute();
    }

    /**************************************/
    /* End of turn list, switching to ... */
    /**************************************/

    // Last racks
    pos = strstr(buff, "Rack ");
    if (pos != NULL && pGame != NULL)
    {
        int nb = 0;
        char letters[20];
        if (sscanf(pos, "Rack %d: %19s", &nb, letters) > 1)
        {
            // Create the played rack
            PlayedRack pldrack;
            pldrack.setManual(convertToWc(letters));
            // Give the rack to the player
            pGame->m_players[nb]->setCurrentRack(pldrack);
        }
        // Read next line
        //            continue;
    }


    // Finalize the game
    if (pGame)
    {
        // We don't really know whose turn it is, but at least we know that
        // the game was saved while a human was to play.
        for (unsigned int i = 0; i < pGame->getNPlayers(); i++)
        {
            if (pGame->m_players[i]->isHuman())
            {
                pGame->m_currPlayer = i;
                break;
            }
        }
    }

    return pGame;
}

/********************************************************
 *
 * Loading games
 *
 ********************************************************/

void Game::save(ostream &out, game_file_format format) const
{
    if (getMode() == kTRAINING && format == FILE_FORMAT_STANDARD)
    {
        gameSaveFormat_14(out);
    }
    else
    {
        gameSaveFormat_15(out);
    }
}


void Game::gameSaveFormat_14(ostream &out) const
{
    char line[100];
    const string decal = "   ";
    out << IDENT_STRING << endl << endl;

    for (unsigned int i = 0; i < m_history.getSize(); i++)
    {
        const Turn& turn = m_history.getTurn(i);
        wstring rack = turn.getPlayedRack().toString(PlayedRack::RACK_EXTRA);
        // FIXME: this will not work if the move does not correspond to a played round!
        const Round &round = turn.getMove().getRound();
        wstring word = round.getWord();
        string coord = convertToMb(round.getCoord().toString(Coord::COORD_MODE_LONG));

        // rack [space] word [space] bonus points coord
        sprintf(line,"%s%s%c%4d %s",
                padAndConvert(rack, 12, false).c_str(),
                padAndConvert(word, 16, false).c_str(),
                round.getBonus() ? '*' : ' ',
                round.getPoints(),
                coord.c_str()
               );

        out << decal << line << endl;
    }

    out << endl;
    out << decal << "total" << string(24,' ');
    sprintf(line, "%4d", getCurrentPlayer().getPoints());
    out << line << endl;
}


void Game::gameSaveFormat_15(ostream &out) const
{
    const string decal = "   ";
    // "Header" of the game
    out << IDENT_STRING << " " << IDENT_FORMAT_15 << endl << endl;
    // Game type
    out << "Game type: " << getModeAsString() << endl;
    // Player list
    for (unsigned int i = 0; i < getNPlayers(); i++)
    {
        out << "Player " << i << ": ";
        if (m_players[i]->isHuman())
            out << "Human" << endl;
        else
            out << "Computer" << endl;
    }
    out << endl;

    // Title of the columns
    char line[100];
    out << decal << " N |   RACK   |    SOLUTION     | REF | PTS | P | BONUS" << endl;
    out << decal << "===|==========|=================|=====|=====|===|======" << endl;

    // Print the game itself
    for (unsigned int i = 0; i < m_history.getSize(); i++)
    {
        const Turn& turn = m_history.getTurn(i);
        wstring rack = turn.getPlayedRack().toString(PlayedRack::RACK_EXTRA);
        const Move &move = turn.getMove();
        switch (move.getType())
        {
            case Move::VALID_ROUND:
            {
                const Round &round = move.getRound();
                wstring word = round.getWord();
                string coord = convertToMb(round.getCoord().toString());
                sprintf(line, "%2d | %s | %s | %3s | %3d | %1d | %c",
                        i + 1,
                        padAndConvert(rack, 8).c_str(),             /* pldrack     */
                        padAndConvert(word, 15, false).c_str(),     /* word        */
                        coord.c_str(),                              /* coord       */
                        move.getScore(),
                        turn.getPlayer(),
                        round.getBonus() ? '*' : ' ');
                break;
            }
            case Move::INVALID_WORD:
            {
                wstring word = move.getBadWord();
                string coord = convertToMb(move.getBadCoord());
                sprintf(line, "%2d | %s | %s | %3s | %3d | %1d |",
                        i + 1,
                        padAndConvert(rack, 8).c_str(),             /* pldrack     */
                        padAndConvert(word, 15, false).c_str(),     /* word        */
                        coord.c_str(),                              /* coord       */
                        move.getScore(),
                        turn.getPlayer());
                break;
            }
            case Move::PASS:
            {
                string action = "(PASS)";
                string coord = " - ";
                sprintf(line, "%2d | %s | %s | %3s | %3d | %1d |",
                        i + 1,
                        padAndConvert(rack, 8).c_str(),             /* pldrack     */
                        truncOrPad(action, 15, ' ').c_str(),        /* word        */
                        coord.c_str(),                              /* coord       */
                        move.getScore(),
                        turn.getPlayer());
                break;
            }
            case Move::CHANGE_LETTERS:
            {
                wstring action = L"(-" + move.getChangedLetters() + L")";
                string coord = " - ";
                sprintf(line, "%2d | %s | %s | %3s | %3d | %1d |",
                        i + 1,
                        padAndConvert(rack, 8).c_str(),             /* pldrack     */
                        padAndConvert(action, 15, false).c_str(),   /* word        */
                        coord.c_str(),                              /* coord       */
                        move.getScore(),
                        turn.getPlayer());
                break;
            }

        }

        out << decal << line << endl;
    }

    switch (getMode())
    {
    case kDUPLICATE:
        // TODO : we should note the score individualy
        out << endl << decal << "Total: " << m_points << endl;
        break;
    case kFREEGAME:
        out << endl << decal << "Total: " << m_points << endl;
        break;
    case kTRAINING:
        out << endl << decal << "Total: " << m_points << endl;
        break;
    }

    // Print current rack for all the players
    out << endl;
    for (unsigned int i = 0; i < getNPlayers(); i++)
    {
        wstring rack = m_players[i]->getCurrentRack().toString();
        out << "Rack " << i << ": " << convertToMb(rack) << endl;
    }
}

