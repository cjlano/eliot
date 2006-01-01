/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
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

/**
 *  \file   game_io.cpp
 *  \brief  Eliot game class file load/save handling
 *  \author Antoine Fraboulet & Olivier Teuliere
 *  \date   2002 - 2005
 */

#include "pldrack.h"
#include "round.h"
#include "turn.h"
#include "player.h"
#include "game.h"
#include "game_factory.h"
#include "debug.h"

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
        debug("Game::load cannot load first line\n");
        return NULL;
    }

    if ((token = strtok(buff, delim)) == NULL)
    {
        debug("Game::load first line is empty\n");
        return NULL;
    }

    /* checks for IDENT_STRING and file format */
    if (string(token) != IDENT_STRING)
    {
        debug("Game::load IDENT_STRING %s unknown\n",token);
        return NULL;
    }

    if ((token = strtok(NULL, delim)) == NULL)
      {
	debug("Game_io::loading file format 1.4\n");
	return Game::gameLoadFormat_14(fin,iDic);
      }

    if (string(token) == string(IDENT_FORMAT_15))
      {
	debug("Game_io::loading file format 1.5\n");
	return Game::gameLoadFormat_15(fin,iDic);
      }

    debug("Game::load unknown format %s\n",token);
    return NULL;
}


Game* Game::gameLoadFormat_14(FILE *fin, const Dictionary& iDic)
{
     Tile tile;
     char buff[4096];
     char rack[20];
     char word[20];
     char pos [5];
     char delim[]=" \t\n\012\015";
     char *token;
     Game *pGame = NULL;

     debug("Game::gameLoadFormat_14\n");

     pGame = GameFactory::Instance()->createTraining(iDic);
     pGame->start();
     
     /*    rack        word          ?bonus    pts  coord    */
     /*    EUOFMIE     FUMEE          *        26   H  4     */
     
     /* read all turns until total */
     while(fgets(buff,sizeof(buff),fin))
     {
         token = strtok(buff,delim);
         if (token != NULL)
         {
             if (strcmp(token,"total")==0)
             {
                 break;
             }

             /* rack */
             strncpy(rack,token,sizeof(rack));
             ((Training*)pGame)->setRack(RACK_MANUAL,false,string(rack));
             debug("%s ",rack);

             /* word */
             token = strtok(NULL,delim);
             if (!token || strcmp(token,"total")==0)
             {
                 break;
             }

             strncpy(word,token,sizeof(word));
             debug("\t%s ",word);

             /* bonus */
             if ((token = strtok(NULL,delim)) == NULL)
                 break;

             /* points */
             if (token[0]=='*')
             {
                 debug("%s\t",token);
                 if ((token = strtok(NULL,delim)) == NULL)
                     break;
             }

             /* pos 1 */
             if ((token = strtok(NULL,delim)) == NULL)
                 break;

             debug("(%s ",token);
             strncpy(pos,token,sizeof(pos));

             /* pos 2 */
             if ((token = strtok(NULL,delim)) == NULL)
                 break;

             debug("%s)",token);
             strncat(pos,token,sizeof(pos));
             debug("%s\n",pos);

             debug("   play %s %s\n",pos, word);
	     pGame->play(string(pos),string(word));
         }
     }
     return pGame;
}


Game* Game::gameLoadFormat_15(FILE *fin, const Dictionary& iDic)
{
    Game *pGame = NULL;

    char buff[4096];
    int num;
    char rack[20];
    char word[20];
    char ref[4];
    int pts;
    int player;
    char *pos;
    Tile tile;

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
                debug("Game::gameLoadFormat_15 new Training\n");
                pGame = GameFactory::Instance()->createTraining(iDic);
                break;
            }
            else if (strstr(buff, "Free game"))
            {
                debug("Game::gameLoadFormat_15 new Freegame\n");
                pGame = GameFactory::Instance()->createFreeGame(iDic);
                break;
            }
            else if (strstr(buff, "Duplicate"))
            {
                debug("Game::gameLoadFormat_15 new Duplicate\n");
                pGame = GameFactory::Instance()->createDuplicate(iDic);
                break;
            }
            else
            {
                debug("Game::gameLoadFormat_15 unknown Game type\n");
                return NULL;
            }
        }
    }

    debug("   Game type is ok, switching to player list\n");

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
                    debug("   add Human player\n");
                    pGame->addHumanPlayer();
                }
                else if (string(type) == "Computer")
                {
                    if (pGame->getMode() == kTRAINING)
                    {
                        debug("   mode == TRAINING, skip player list since we have a computer player\n");
                        break;
                    }
                    else
                    {
                        debug("   add Computer player\n");
                        pGame->addAIPlayer();
                    }    
                }
                else
                {
                    debug("   unknown player, bailing out\n");
                    delete pGame;
                    return NULL;
                }
            }
        }
        else if (strstr(buff," N |   RACK   "))
        {
            debug("   ok for player list, going turn list\n");
            break;
        }
    }

    debug("   Player list ok, switching to turn list \n");

    /*************/
    /* Turn list */
    /*************/

    while (fgets(buff, sizeof(buff), fin))
    {
        // Skip columns title
        if (strstr(buff,"| PTS | P |") != NULL)
        {
            debug("   ** PTS line, skiping\n");
            continue;
        }

        // Skip columns title
        if (strstr(buff, "==") != NULL)
        {
            debug("   ** == line, skiping\n");
            continue;
        }

        if (string(buff) == "\n")
        {
            debug("   ** empty line\n");
            continue;
        }


        char bonus = 0;
        int res = sscanf(buff, "   %2d | %8s | %s | %3s | %3d | %1d | %c",
                         &num, rack, word, ref, &pts, &player, &bonus);
        
        debug("   -- line %s",buff);
        
        if (res < 6)
            {
                debug("   Game::load15 invalid line -%s-\n",buff);
                continue;
            }
        
        debug("              %2d | %8s | %s | %3s | %3d | %1d | %c \n",
              num, rack, word, ref, pts, player, bonus);
        
        // Integrity checks
        // TODO: add more checks
        if (pts < 0)
            {
                debug("   Game::load15 line -%s- points < 0  ?\n",buff);
                continue;
            }
        if (player < 0 || player > pGame->getNPlayers())
            {
                debug("   Game::load15 line -%s- too much player (%d>%d)",buff,player,pGame->getNPlayers());
                continue;
            }
        if (bonus && bonus != '*')
            {
                debug("   Game::load15 line -%s- wring bonus sign\n",buff);
                continue;
            }
        
        // Build a rack for the correct player
        PlayedRack pldrack;
        if ((res = pldrack.setManual(string(rack))) > 0)
        {
            debug("   Game::load15 set rack manual returned with error %d\n",res);
        }
        debug("    history rack %s\n",pldrack.toString().c_str());

        // Build a round
        Round round;
        round.setPoints(pts);
        if (bonus == '*')
            round.setBonus(1);
        
        if (isalpha(ref[0]))
            {
                // Horizontal word
                round.accessCoord().setDir(Coord::HORIZONTAL);
                round.accessCoord().setRow(ref[0] - 'A' + 1);
                round.accessCoord().setCol(atoi(ref + 1));
                
                for (unsigned int i = 0; i < strlen(word); i++)
                    {
                        tile = Tile(word[i]);
                        
                        if (!pGame->m_board.getTile(round.getCoord().getRow(), round.getCoord().getCol() + i).isEmpty())
                            {
                                round.addRightFromBoard(tile);
                            }
                        else
                            {
                                round.addRightFromRack(tile, islower(word[i]));
                                pGame->m_bag.takeTile((islower(word[i])) ? Tile::Joker() : tile);
                            }
                    }
            }
        else
            {
                // Vertical word
                round.accessCoord().setDir(Coord::VERTICAL);
                round.accessCoord().setRow(ref[strlen(ref) - 1] - 'A' + 1);
                round.accessCoord().setCol(atoi(ref));
                
                for (unsigned int i = 0; i < strlen(word); i++)
                    {
                        tile = Tile(word[i]);
                        
                        if (!pGame->m_board.getTile(round.getCoord().getRow() + i, round.getCoord().getCol()).isEmpty())
                            {
                                round.addRightFromBoard(tile);
                            }
                        else
                            {
                                round.addRightFromRack(tile, islower(word[i]));
                                pGame->m_bag.takeTile((islower(word[i])) ? Tile::Joker() : tile);
                            }
                    }
            }
        
        //             pGame->m_currPlayer = player;
        //             // Update the rack for the player
        //             pGame->m_players[player]->setCurrentRack(pldrack);
        //             // End the turn for the current player (this creates a new rack)
        //             pGame->m_players[player]->endTurn(round,num - 1);
        
        // Play the round
        pGame->helperPlayRound(round);
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
            pldrack.setManual(string(letters));
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
        for (int i = 0; i < pGame->getNPlayers(); i++)
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
    int i;
    char line[100];
    const char decal[]="   ";
    out << IDENT_STRING << endl << endl;

    for(i = 0; i < m_history.getSize(); i++)
	{
	    const Turn& turn = m_history.getTurn(i);
            string rack = turn.getPlayedRack().toString(PlayedRack::RACK_EXTRA);
            string word = turn.getRound().getWord();
            string coord = turn.getRound().getCoord().toString(Coord::COORD_MODE_LONG);

            // rack [space] word [space] bonus points coord
            sprintf(line,"%s%s%s%s%c%4d %s",
                    rack.c_str(),
                    string(12 - rack.size(), ' ').c_str(),
                    word.c_str(),
                    string(16 - word.size(), ' ').c_str(),
                    turn.getRound().getBonus() ? '*' : ' ',
                    turn.getRound().getPoints(),
                    coord.c_str()
                    );

            out << decal << line << endl;
        }

    out << endl;
    out << decal << "total" << string(24,' ');
    sprintf(line,"%4d", getCurrentPlayer().getPoints());
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
    for (int i = 0; i < getNPlayers(); i++)
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
    for (int i = 0; i < m_history.getSize(); i++)
    {
	const Turn& turn = m_history.getTurn(i);
        string word = turn.getRound().getWord();
        string coord = turn.getRound().getCoord().toString();
        sprintf(line, "%2d | %8s | %s%s | %3s | %3d | %1d | %c",
                i + 1,
		turn.getPlayedRack().toString().c_str(),    /* pldrack     */
		word.c_str(),                               /* word        */
                string(15 - word.size(), ' ').c_str(),      /* fill spaces */
                coord.c_str(),                              /* coord       */
		turn.getRound().getPoints(),
                turn.getPlayer(),
		turn.getRound().getBonus() ? '*' : ' ');

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
    for (int i = 0; i < getNPlayers(); i++)
    {
        string rack = m_players[i]->getCurrentRack().toString();
        out << "Rack " << i << ": " << rack << endl;
    }
}

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
