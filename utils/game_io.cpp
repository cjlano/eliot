/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: game_io.cpp,v 1.1 2005/02/26 22:57:34 ipkiss Exp $
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
#include <string>

#include "game_io.h"
#include "game.h"
#include "training.h"

using namespace std;


void GameIO::printBoard(ostream &out, const Game &iGame)
{
    int row, col;

    out << "   ";
    for (col = BOARD_MIN; col <= BOARD_MAX; col++)
        out << setw(3) << col - BOARD_MIN + 1;
    out << endl;
    for (row = BOARD_MIN; row <= BOARD_MAX; row++)
    {
        out << " " << (char)(row - BOARD_MIN + 'A') << " ";
        for (col = BOARD_MIN; col <= BOARD_MAX; col++)
        {
            char l = iGame.getBoardChar(row, col);
            out << setw(3) << (l ? l : '-');
        }
        out << endl;
    }
}


void GameIO::printBoardJoker(ostream &out, const Game &iGame)
{
    int row,col;

    out << "   ";
    for (col = BOARD_MIN; col <= BOARD_MAX; col++)
        out << setw(3) << col - BOARD_MIN + 1;
    out << endl;

    for (row = BOARD_MIN; row <= BOARD_MAX; row++)
    {
        out << " " << (char)(row - BOARD_MIN + 'A') << " ";
        for (col = BOARD_MIN; col <= BOARD_MAX; col++)
        {
            char l = iGame.getBoardChar(row, col);
            bool j = (iGame.getBoardCharAttr(row, col) & ATTR_JOKER);

            out << " " << (j ? '.' : (l ? ' ' : '-'));
            out << (l ? l : '-');
        }
        out << endl;
    }
}


void GameIO::printBoardMultipliers(ostream &out, const Game &iGame)
{
    int row, col;

    out << "   ";
    for (col = BOARD_MIN; col <= BOARD_MAX; col++)
        out << setw(3) << col - BOARD_MIN + 1;
    out << endl;

    for (row = BOARD_MIN; row <= BOARD_MAX; row++)
    {
        out << " " << (char)(row - BOARD_MIN + 'A') << " ";
        for (col = BOARD_MIN; col <= BOARD_MAX; col++)
        {
            char l = iGame.getBoardChar(row, col);
            if (l != 0)
                out << "  " << l;
            else
            {
                int wm = iGame.getBoardWordMultiplier(row, col);
                int tm = iGame.getBoardLetterMultiplier(row, col);

                if (wm > 1)
                    out << "  " << ((wm == 3) ? '@' : '#');
                else if (tm > 1)
                    out << "  " << ((tm == 3) ? '*' : '+');
                else
                    out << "  -";
            }
        }
        out << endl;
    }
}


void GameIO::printBoardMultipliers2(ostream &out, const Game &iGame)
{
    int row, col;

    out << "   ";
    for (col = BOARD_MIN; col <= BOARD_MAX; col++)
        out << setw(3) << col - BOARD_MIN + 1;
    out << endl;

    for (row = BOARD_MIN; row <= BOARD_MAX; row++)
    {
        out << " " << (char)(row - BOARD_MIN + 'A') << " ";
        for (col = BOARD_MIN; col <= BOARD_MAX; col++)
        {
            char l = iGame.getBoardChar(row, col);
            int wm = iGame.getBoardWordMultiplier(row, col);
            int tm = iGame.getBoardLetterMultiplier(row, col);

            if (wm > 1)
                out << " " << ((wm == 3) ? '@' : '#');
            else if (tm > 1)
                out << " " << ((tm == 3) ? '*' : '+');
            else
                out << " -";
            out << (l ? l : '-');
        }
        out << endl;
    }
}


void GameIO::printNonPlayed(ostream &out, const Game &iGame)
{
    const list<Tile>& allTiles = Tile::getAllTiles();
    list<Tile>::const_iterator it;

    for (it = allTiles.begin(); it != allTiles.end(); it++)
    {
        if (iGame.getNCharInBag(it->toChar()) > 9)
            out << " ";
        out << setw(2) << it->toChar();
    }
    out << endl;

    for (it = allTiles.begin(); it != allTiles.end(); it++)
    {
        out << " " << iGame.getNCharInBag(it->toChar());
    }
    out << endl;
}


void GameIO::printPlayedRack(ostream &out, const Game &iGame, int n)
{
    out << iGame.getPlayerRack(iGame.currPlayer()) << endl;
}


void GameIO::printAllRacks(ostream &out, const Game &iGame)
{
    for (int j = 0; j < iGame.getNPlayers(); j++)
    {
        out << "Joueur " << j << ": ";
        out << iGame.getPlayerRack(j) << endl;
    }
}


static void searchResultLine(ostream &out, const Training &iGame, int num)
{
    string word = iGame.getSearchedWord(num);
    if (word.size() == 0)
        return;
    out << word << string(16 - word.size(), ' ')
        << (iGame.getSearchedBonus(num) ? '*' : ' ')
        << setw(4) << iGame.getSearchedPoints(num)
        << ' ' << iGame.getSearchedCoords(num);
}


void GameIO::printSearchResults(ostream &out, const Training &iGame, int num)
{
    for (int i = 0; i < num && i < iGame.getNResults(); i++)
    {
        out << setw(3) << i + 1 << ": ";
        searchResultLine(out, iGame, i);
        out << endl;
    }
}


void GameIO::printPoints(ostream &out, const Game &iGame)
{
    out << iGame.getPlayerPoints(0) << endl;
}


void GameIO::printAllPoints(ostream &out, const Game &iGame)
{
    for (int i = 0; i < iGame.getNPlayers(); i++)
    {
        out << "Joueur " << i << ": "
            << setw(4) << iGame.getPlayerPoints(i) << endl;
    }
}

