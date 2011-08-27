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
#include <cwctype> // For iswlower
#include <cstdio>

#include "dic.h"
#include "pldrack.h"
#include "round.h"
#include "turn.h"
#include "player.h"
#include "ai_percent.h"
#include "game_params.h"
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

    pGame = GameFactory::Instance()->createTraining(iDic, GameParams());
    pGame->addPlayer(new HumanPlayer);
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
            static_cast<Training*>(pGame)->setRackManual(false, wfl(rack));

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
            strncat(pos, token, sizeof(pos) - strlen(pos) - 1);

            if ((ret = pGame->play(wfl(pos), wfl(word))))
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
                pGame = GameFactory::Instance()->createTraining(iDic, GameParams());
                break;
            }
            else if (strstr(buff, "Free game"))
            {
                pGame = GameFactory::Instance()->createFreeGame(iDic, GameParams());
                break;
            }
            else if (strstr(buff, "Duplicate"))
            {
                pGame = GameFactory::Instance()->createDuplicate(iDic, GameParams());
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
        if (!iDic.validateLetters(wfl(rack)))
        {
            throw GameException("Rack invalid for the current dictionary");
        }
        pldrack.setManual(wfl(rack));

        // Build a round
        Round round;
        round.setPoints(pts);
        if (bonus == '*')
            round.setBonus(1);

        wstring word = wfl(tmpWord);
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
        GameMoveCmd cmd(*pGame, Move(round), pGame->m_currPlayer);
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
            pldrack.setManual(wfl(letters));
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

