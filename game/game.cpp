/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: game.cpp,v 1.2 2005/02/09 22:33:56 ipkiss Exp $
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

#include <iomanip>
#include "dic.h"
#include "dic_search.h"
#include "tile.h"
#include "rack.h"
#include "round.h"
#include "pldrack.h"
#include "results.h"
#include "player.h"
#include "game.h"

// FIXME: this indicates a design flaw!
#include "training.h"
#include "duplicate.h"
#include "freegame.h"

#include "debug.h"


/*********************************************************
 *********************************************************/

Game::Game(const Dictionary &iDic):
    m_dic(&iDic)
{
    init();
    m_points = 0;
    m_currPlayer = -1;
    m_finished = false;
}


Game::~Game()
{
    for (unsigned int i = 0; i < m_roundHistory.size(); i++)
    {
        delete m_roundHistory[i];
    }
    for (unsigned int i = 0; i < m_rackHistory.size(); i++)
    {
        delete m_rackHistory[i];
    }
    for (int i = 0; i < getNPlayers(); i++)
    {
        delete m_players[i];
    }
}

/*********************************************************
 *********************************************************/

void Game::init()
{
}


Game * Game::load(FILE *fin, const Dictionary &iDic)
{
    char buff[4096];
    char delim[] = " \t\n|";
    char *token;

    // Check characteristic string
    if (fgets(buff, sizeof(buff), fin) == NULL)
        return NULL;
    if ((token = strtok(buff, delim)) == NULL)
        return NULL;
    if (string(token) != IDENT_STRING)
        return NULL;

    int num;
    char rack[20];
    char word[20];
    char ref[4];
    int pts;
    int player;
    char *pos;
    Tile tile;
    Game *pGame = NULL;

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
                pGame = new Training(iDic);
            else if (strstr(buff, "Duplicate"))
                pGame = new Duplicate(iDic);
            else if (strstr(buff, "Free game"))
                pGame = new FreeGame(iDic);
            else
                return NULL;
            // Read next line
            continue;
        }

        // Players type
        pos = strstr(buff, "Player ");
        if (pos != NULL && pGame != NULL)
        {
            int nb = 0;
            char type[20];
            if (sscanf(pos, "Player %d: %19s", &nb, type) > 1)
            {
                if (string(type) == "Human")
                    pGame->addHumanPlayer();
                else if (string(type) == "Computer")
                    pGame->addAIPlayer();
                else
                    ;
            }
            // Read next line
            continue;
        }

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
                char *r = letters;
                if (strchr(r, '+'))
                {
                    while (*r != '+')
                    {
                        pldrack.addOld(Tile(*r));
                        r++;
                    }
                    r++;
                }
                while (*r)
                {
                    pldrack.addNew(Tile(*r));
                    r++;
                }

                // Give the rack to the player
                pGame->m_players[nb]->setCurrentRack(pldrack);
            }
            // Read next line
            continue;
        }

        // Skip columns title
        if (strstr(buff, "==") != NULL ||
            strstr(buff, "| PTS | P |") != NULL)
        {
            continue;
        }

        if (string(buff) != "\n" && pGame != NULL)
        {
            char bonus = 0;
            int res = sscanf(buff, "   %2d | %8s | %s | %3s | %3d | %1d | %c",
                             &num, rack, word, ref, &pts, &player, &bonus);
            if (res < 6)
                continue;

            // Integrity checks
            // TODO: add more checks
            if (pts < 0)
                continue;
            if (player < 0 || player > pGame->getNPlayers())
                continue;
            if (bonus && bonus != '*')
                continue;

            // Build a rack for the correct player
            PlayedRack pldrack;
            char *r = rack;
            if (strchr(r, '+'))
            {
                while (*r != '+')
                {
                    pldrack.addOld(Tile(*r));
                    r++;
                }
                r++;
            }

            while (*r)
            {
                pldrack.addNew(Tile(*r));
                r++;
            }

            // Build a round
            Round round;
            round.setPoints(pts);
            if (bonus == '*')
                round.setBonus(1);

            if (isalpha(ref[0]))
            {
                // Horizontal word
                round.setDir(HORIZONTAL);
                round.setRow(ref[0] - 'A' + 1);
                round.setCol(atoi(ref + 1));

                for (unsigned int i = 0; i < strlen(word); i++)
                {
                    tile = Tile(word[i]);

                    if (pGame->m_board.getTile(round.getRow(), round.getCol() + i) != Tile::dummy())
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
                round.setDir(VERTICAL);
                round.setRow(ref[strlen(ref) - 1] - 'A' + 1);
                round.setCol(atoi(ref));

                for (unsigned int i = 0; i < strlen(word); i++)
                {
                    tile = Tile(word[i]);

                    if (pGame->m_board.getTile(round.getRow() + i, round.getCol()) != Tile::dummy())
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

            pGame->m_currPlayer = player;
            // Update the rack for the player
            pGame->m_players[player]->setCurrentRack(pldrack);
            // End the turn for the current player (this creates a new rack)
            pGame->m_players[player]->endTurn(round, num - 1);
            // Add the points
            pGame->m_players[player]->addPoints(pts);
            // Play the round
            pGame->helperPlayRound(round);
        }
    }

    // Finalize the game
    if (pGame)
    {
        // We don't really know whose turn it is, but at least we know that
        // the game was saved while a human was to play.
        pGame->nextHumanPlayer();
    }
    return pGame;
}


void Game::save(ostream &out) const
{
    const string decal = "   ";
    // "Header" of the game
    out << IDENT_STRING << endl << endl;
    out << "Game type: " << getModeAsString();
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
    for (int i = 0; i < getNRounds(); i++)
    {
        string word = getPlayedWord(i);
        string coord = getPlayedCoords(i);
        sprintf(line, "%2d | %8s | %s%s | %3s | %3d | %1d | %c",
                i + 1, getPlayedRack(i).c_str(), word.c_str(),
                string(15 - word.size(), ' ').c_str(),
                coord.c_str(), getPlayedPoints(i),
                getPlayedPlayer(i), getPlayedBonus(i) ? '*' : ' ');

        out << decal << line << endl;
    }
    out << endl << decal << "Total: " << m_points << endl;

    // Print current rack for all the players
    out << endl;
    for (int i = 0; i < getNPlayers(); i++)
    {
        string rack = formatPlayedRack(m_players[i]->getCurrentRack());
        out << "Rack " << i << ": " << rack << endl;
    }
}


/*********************************************************
 *********************************************************/

/* This function plays a round on the board */
int Game::helperPlayRound(const Round &iRound)
{
    /*
     * We remove tiles from the bag only when they are played
     * on the board. When going back in the game, we must only
     * replace played tiles.
     * We test a rack when it is set but tiles are left in the bag.
     */

    int i;

    /* History of the game */
    m_roundHistory.push_back(new Round(iRound));
    m_rackHistory.push_back(new PlayedRack(m_players[m_currPlayer]->getLastRack()));
    m_playerHistory.push_back(m_currPlayer);

    m_points += iRound.getPoints();

    /* Update the board and the bag */
    m_board.addRound(*m_dic, iRound);
    for (i = 0; i < iRound.getWordLen(); i++)
    {
        if (iRound.isPlayedFromRack(i))
        {
            if (iRound.isJoker(i))
                m_bag.takeTile(Tile::Joker());
            else
                m_bag.takeTile(iRound.getTile(i));
        }
    }

    return 0;
}


int Game::back(int n)
{
    int i, j;
    Player *player;

    for (i = 0; i < n; i++)
    {
        if (m_roundHistory.size())
        {
            prevPlayer();
            player = m_players[m_currPlayer];
            const Round &lastround = *m_roundHistory.back();

            /* Remove the points of this round */
            player->addPoints(- lastround.getPoints());
            m_points -= lastround.getPoints();

            /* Remove the word from the board, and put its letters back
             * into the bag */
            m_board.removeRound(*m_dic, lastround);
            for (j = 0; j < lastround.getWordLen(); j++)
            {
                if (lastround.isPlayedFromRack(j))
                {
                    if (lastround.isJoker(j))
                        m_bag.replaceTile(Tile::Joker());
                    else
                        m_bag.replaceTile(lastround.getTile(j));
                }
            }
            delete &lastround;
            m_roundHistory.pop_back();
            m_playerHistory.pop_back();
        }
    }
    return 0;
}


int Game::testPlay(int n)
{
    Round round;
    const Results &results = m_players[m_currPlayer]->aiGetResults();

    if (n >= results.in())
        return 2;
    m_board.testRound(results.get(n));
    return 0;
}


int Game::removeTestPlay()
{
    m_board.removeTestRound();
    return 0;
}

/*********************************************************
 *********************************************************/

char Game::getBoardChar(int iRow, int iCol) const
{
    char letter = 0;
    Tile tile = m_board.getTile(iRow, iCol);
    if (tile != Tile::dummy())
    {
        letter = tile.toChar();
        if (m_board.isJoker(iRow, iCol))
            letter = tolower(letter);
    }
    return letter;
}


int Game::getBoardCharAttr(int iRow, int iCol) const
{
    int t = m_board.getTestChar(iRow, iCol);
    int j = m_board.isJoker(iRow, iCol);
    return  (t << 1) | j;
}


int Game::getBoardWordMultiplier(int iRow, int iCol) const
{
    return m_board.getWordMultiplier(iRow, iCol);
}


int Game::getBoardLetterMultiplier(int iRow, int iCol) const
{
    return m_board.getLetterMultiplier(iRow, iCol);
}

/*********************************************************
 *********************************************************/

void Game::realBag(Bag &ioBag) const
{
    vector<Tile> tiles;

    /* Copy the bag */
    ioBag = m_bag;

    /* The real content of the bag depends of the game mode */
    if (getMode() == kFREEGAME)
    {
        /* In freegame mode, replace the letters from all the racks */
        for (int i = 0; i < getNPlayers(); i++)
        {
            PlayedRack pld = m_players[i]->getCurrentRack();
            pld.getAllTiles(tiles);
            for (unsigned int j = 0; j < tiles.size(); j++)
            {
                ioBag.takeTile(tiles[j]);
            }
        }
    }
    else
    {
        /* In training or duplicate mode, replace the rack of the current
         * player only */
        PlayedRack pld = m_players[m_currPlayer]->getCurrentRack();
        pld.getAllTiles(tiles);
        for (int j = 0; j < pld.nTiles(); j++)
        {
            ioBag.takeTile(tiles[j]);
        }
    }
}


bool Game::rackInBag(const Rack &iRack, const Bag &iBag) const
{
    const list<Tile>& allTiles = Tile::getAllTiles();
    list<Tile>::const_iterator it;
    for (it = allTiles.begin(); it != allTiles.end(); it++)
    {
        if (iRack.in(*it) > iBag.in(*it))
            return false;
    }
    return true;
}


int Game::helperSetRackRandom(int p, bool iCheck, set_rack_mode mode)
{
    Tile l;
    vector<Tile> tiles;
    int nold, min;

    PDEBUG(p < 0 || p >= getNPlayers(), "GAME: wrong player number\n");

    PlayedRack pld = m_players[p]->getCurrentRack();
    nold = pld.nOld();

    /* Create a copy of the bag in which we can do everything we want */
    Bag bag;
    /* Remove from the bag the tiles of the racks */
    realBag(bag);

    /* We may have removed too many letters from the bag (i.e. the 'new'
     * letters of the player) */
    if (mode == RACK_NEW && nold != 0)
    {
        pld.getNewTiles(tiles);
        for (unsigned int i = 0; i < tiles.size(); i++)
        {
            bag.replaceTile(tiles[i]);
        }
        pld.resetNew();
    }
    else
    {
        /* "Complement" with an empty rack is equivalent to ALL */
        pld.reset();
        /* Do not forget to update nold, for the RACK_NEW case */
        nold = 0;
    }

    /* Nothing in the rack, nothing in the bag --> end of the game */
    if (bag.nTiles() == 0 && pld.nTiles() == 0)
    {
        return 1;
    }

    /* Make sure there is a way to complete the rack */
    min = 0;
    if (iCheck)
    {
        int oldc, oldv;

        if (bag.nVowels() == 0 || bag.nConsonants() == 0)
        {
            return 1;
        }
        if (bag.nVowels() > 1 && bag.nConsonants() > 1
            && getNRounds() < 15)
            min = 2;
        else
            min = 1;

        /* Count the remaining consonants and vowels in the rack */
        pld.getOldTiles(tiles);
        oldc = 0;
        oldv = 0;
        for (unsigned int i = 0; i < tiles.size(); i++)
        {
            if (tiles[i].isConsonant())
                oldc++;
            if (tiles[i].isVowel())
                oldv++;
        }

        /* RACK_MAX - nold is the number of letters to add */
        if (min > oldc + RACK_MAX - nold ||
            min > oldv + RACK_MAX - nold)
        {
            return 3;
        }
    }

    /* Get new tiles from the bag */
    for (int i = nold; (bag.nTiles() != 0) && (i < RACK_MAX); i++)
    {
        l = bag.selectRandom();
        bag.takeTile(l);
        pld.addNew(l);
    }

    if (iCheck && !pld.checkRack(min))
        return 2;

    m_players[p]->setCurrentRack(pld);

    return 0;
}


int Game::helperSetRackManual(int p, bool iCheck, const string &iLetters)
{
    unsigned int i;
    int min;

    PlayedRack pld = m_players[p]->getCurrentRack();
    m_players[p]->clearResults();
    pld.reset();

    if (iLetters.size() == 0)
    {
        return 0;
    }

    for (i = 0; i < iLetters.size() && iLetters[i] != '+'; i++)
    {
        Tile tile(iLetters[i]);
        if (tile == Tile::dummy())
        {
            return 1;
        }
        pld.addOld(tile);
    }

    if (i < iLetters.size() && iLetters[i] == '+')
    {
        for (i++; i < iLetters.size(); i++)
        {
            Tile tile(iLetters[i]);
            if (tile == Tile::dummy())
            {
                return 1;
            }
            pld.addNew(tile);
        }
    }

    Rack rack;
    pld.getRack(rack);
    if (! rackInBag(rack, m_bag))
    {
        pld.reset();
        return 1;
    }

    if (iCheck)
    {
        if (m_bag.nVowels() > 1 && m_bag.nConsonants() > 1
            && getNRounds() < 15)
            min = 2;
        else
            min = 1;
        if (!pld.checkRack(min))
            return 2;
    }

    m_players[p]->setCurrentRack(pld);

    return 0;
}


/*********************************************************
 *********************************************************/

int Game::getNCharInBag(char c) const
{
    return m_bag.in(Tile(c));
}


string Game::formatCoords(const Round &iRound) const
{
    if (iRound.getDir() == HORIZONTAL)
    {
        char s[5];
        sprintf(s, "%d", iRound.getCol());
        return string(1, iRound.getRow() + 'A' - 1) + s;
    }
    else
    {
        char s[5];
        sprintf(s, "%d", iRound.getCol());
        return s + string(1, iRound.getRow() + 'A' - 1);
    }
}


string Game::formatPlayedRack(const PlayedRack &iRack, bool showExtraSigns) const
{
    vector<Tile> tiles;
    unsigned int i;
    string s;

    iRack.getOldTiles(tiles);
    for (i = 0; i < tiles.size(); i++)
        s += tiles[i].toChar();

    iRack.getNewTiles(tiles);
    if (showExtraSigns && i > 0 && tiles.size())
        s += '+';

    for (i = 0; i < tiles.size(); i++)
        s += tiles[i].toChar();
    return s;
}

/*********************************************************
 *********************************************************/

string Game::getPlayedRack(int num) const
{
    if (num < 0 || num > getNRounds())
        return "";
    return formatPlayedRack(*m_rackHistory[num]);
}


string Game::getPlayedWord(int num) const
{
    if (num < 0 || num >= getNRounds())
        return "";
    char c;
    string s;
    const Round &r = *m_roundHistory[num];
    for (int i = 0; i < r.getWordLen(); i++)
    {
        c = r.getTile(i).toChar();
        if (r.isJoker(i))
            c = tolower(c);
        s += c;
    }
    return s;
}


string Game::getPlayedCoords(int num) const
{
    if (num < 0 || num >= getNRounds())
        return "";
    return formatCoords(*m_roundHistory[num]);
}


int Game::getPlayedPoints(int num) const
{
    if (num < 0 || num >= getNRounds())
        return 0;
    return m_roundHistory[num]->getPoints();
}


int Game::getPlayedBonus(int num) const
{
    if (num < 0 || num >= getNRounds())
        return 0;
    return m_roundHistory[num]->getBonus();
}


int Game::getPlayedPlayer(int num) const
{
    if (num < 0 || num >= getNRounds())
        return 0;
    return m_playerHistory[num];
}

/*********************************************************
 *********************************************************/

int Game::getNResults() const
{
    return m_players[m_currPlayer]->aiGetResults().in();
}


string Game::getSearchedWord(int num) const
{
    const Results &results = m_players[m_currPlayer]->aiGetResults();
    if (num < 0 || num >= results.in())
        return "";
    char c;
    string s;
    const Round &r = results.get(num);
    for (int i = 0; i < r.getWordLen(); i++)
    {
        c = r.getTile(i).toChar();
        if (r.isJoker(i))
            c = tolower(c);
        s += c;
    }
    return s;
}


string Game::getSearchedCoords(int num) const
{
    const Results &results = m_players[m_currPlayer]->aiGetResults();
    if (num < 0 || num >= results.in())
        return "";
    return formatCoords(results.get(num));
}


int Game::getSearchedPoints(int num) const
{
    const Results &results = m_players[m_currPlayer]->aiGetResults();
    if (num < 0 || num >= results.in())
        return 0;
    return results.get(num).getPoints();
}


int Game::getSearchedBonus(int num) const
{
    const Results &results = m_players[m_currPlayer]->aiGetResults();
    if (num < 0 || num >= results.in())
        return 0;
    return results.get(num).getBonus();
}


int Game::getPlayerPoints(int num) const
{
    if (num < 0 || num >= getNPlayers())
        return 0;
    return m_players[num]->getPoints();
}


string Game::getPlayerRack(int num) const
{
    if (num < 0 || num >= getNPlayers())
        return "";
    return formatPlayedRack(m_players[num]->getCurrentRack(), false);
}


int Game::getNHumanPlayers() const
{
    int count = 0;
    for (int i = 0; i < getNPlayers(); i++)
        count += (m_players[i]->isHuman() ? 1 : 0);
    return count;
}


void Game::addHumanPlayer()
{
    m_players.push_back(new Player(true));
}


void Game::addAIPlayer()
{
    m_players.push_back(new Player(false));
}


void Game::prevPlayer()
{
    // TODO: assertion...
    if (getNPlayers() == 0)
        return;

    if (m_currPlayer == 0)
        m_currPlayer = getNPlayers() - 1;
    else
        m_currPlayer--;
}


void Game::nextPlayer()
{
    // TODO: assertion...
    if (getNPlayers() == 0)
        return;

    if (m_currPlayer == getNPlayers() - 1)
        m_currPlayer = 0;
    else
        m_currPlayer++;
}


void Game::prevHumanPlayer()
{
    if (getNHumanPlayers() == 0)
        return;
    do
    {
        prevPlayer();
    } while (!m_players[m_currPlayer]->isHuman() ||
             m_players[m_currPlayer]->getStatus() != Player::TO_PLAY);
}


void Game::nextHumanPlayer()
{
    if (getNHumanPlayers() == 0)
        return;
    do
    {
        nextPlayer();
    } while (!m_players[m_currPlayer]->isHuman() ||
             m_players[m_currPlayer]->getStatus() != Player::TO_PLAY);
}


/*
 * This function checks whether it is legal to play the given word at the
 * given coordinates. If so, the function fills a Round object, also given as
 * a parameter.
 * Possible return values:
 *  0: correct word, the Round can be used by the caller
 *  1: no dictionary set
 *  2: invalid coordinates (unreadable or out of the board)
 *  3: word not present in the dictionary
 *  4: not enough letters in the rack to play the word
 *  5: word is part of a longer one
 *  6: word overwriting an existing letter
 *  7: invalid crosscheck, or word going out of the board
 *  8: word already present on the board (no new letter from the rack)
 *  9: isolated word (not connected to the rest)
 * 10: first word not horizontal
 * 11: first word not covering the H8 square
 */
int Game::checkPlayedWord(const string &iCoord,
                          const string &iWord, Round &oRound)
{
    char l[COOR_SIZE_MAX];
    int col, row;
    int res;
    vector<Tile> tiles;
    Tile t;

    // TODO: check that there is at least 1 player.

    /* Init the round with the given coordinates */
    oRound.init();
    if (sscanf(iCoord.c_str(), "%1[a-oA-O]%2d", l, &col) == 2)
        oRound.setDir(HORIZONTAL);
    else if (sscanf(iCoord.c_str(), "%2d%1[a-oA-O]", &col, l) == 2)
        oRound.setDir(VERTICAL);
    else
        return 2;
    row = toupper(*l) - 'A' + 1;
    if (col < BOARD_MIN || col > BOARD_MAX ||
        row < BOARD_MIN || row > BOARD_MAX)
    {
        return 2;
    }
    oRound.setCol(col);
    oRound.setRow(row);

    /* Check the existence of the word */
    if (Dic_search_word(*m_dic, iWord.c_str()) == 0)
        return 3;

    /* Set the word */
    // TODO: make this a Round_ function (Round_setwordfromchar for example)
    // or a Tiles_ function (to transform a char* into a Tile*)
    // Adding a getter on the word could help too...
    for (unsigned int i = 0; i < iWord.size(); i++)
    {
        tiles.push_back(Tile(iWord[i]));
    }
    oRound.setWord(tiles);
    for (unsigned int i = 0; i < iWord.size(); i++)
    {
        if (islower(iWord[i]))
            oRound.setJoker(i);
    }

    /* Check the word position, compute its points,
     * and specify the origin of each letter (board or rack) */
    res = m_board.checkRound(oRound, getNRounds() == 0);
    if (res != 0)
        return res + 4;

    /* Check that the word can be formed with the tiles in the rack:
     * we first create a copy of the rack, then we remove the tiles
     * one by one */
    Rack rack;
    Player *player = m_players[m_currPlayer];
    player->getCurrentRack().getRack(rack);

    for (int i = 0; i < oRound.getWordLen(); i++)
    {
        if (oRound.isPlayedFromRack(i))
        {
            if (oRound.isJoker(i))
                t = Tile::Joker();
            else
                t = oRound.getTile(i);

            if (! rack.in(t))
            {
                return 4;
            }
            rack.remove(t);
        }
    }

    return 0;
}

