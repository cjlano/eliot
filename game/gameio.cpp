/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: gameio.cpp,v 1.2 2005/02/17 20:01:59 ipkiss Exp $
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
#include <ctype.h>

#include "dic.h"
#include "tile.h"
#include "bag.h"
#include "rack.h"
#include "round.h"
#include "pldrack.h"
#include "results.h"
#include "board.h"
#include "player.h"

#include "game.h"
#include "training.h"

#include "debug.h"

void Game::printBoard(ostream &out) const
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
            char l = this->getBoardChar(row, col);
            out << setw(3) << (l ? l : '-');
        }
        out << endl;
    }
}


void Game::printBoardJoker(ostream &out) const
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
            char l = this->getBoardChar(row, col);
            bool j = (this->getBoardCharAttr(row, col) & ATTR_JOKER);

            out << " " << (j ? '.' : (l ? ' ' : '-'));
            out << (l ? l : '-');
        }
        out << endl;
    }
}


#if 0
void Game::printBoardPoint(ostream &out) const
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
            char l = this->getBoardChar(row, col);
            int p1 = this->m_board.m_pointRow[row][col];
            int p2 = this->m_board.m_pointCol[col][row];

            if (p1 > 0 && p2 > 0)
                out << setw(3) << p1 + p2;
            else if (p1 > 0)
                out << setw(3) << p1;
            else if (p2 > 0)
                out << setw(3) << p2;
            else if (l)
                out << setw(3) << l;
            else
                out << " --";
        }
        out << endl;
    }
}
#endif


void Game::printBoardMultipliers(ostream &out) const
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
            char l = this->getBoardChar(row, col);
            if (l != 0)
                out << "  " << l;
            else
            {
                int wm = this->getBoardWordMultiplier(row, col);
                int tm = this->getBoardLetterMultiplier(row, col);

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


void Game::printBoardMultipliers2(ostream &out) const
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
            char l = this->getBoardChar(row, col);
            int wm = this->getBoardWordMultiplier(row, col);
            int tm = this->getBoardLetterMultiplier(row, col);

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


void Game::printNonPlayed(ostream &out) const
{
    const list<Tile>& allTiles = Tile::getAllTiles();
    list<Tile>::const_iterator it;

    for (it = allTiles.begin(); it != allTiles.end(); it++)
    {
        if (this->m_bag.in(*it) > 9)
            out << " ";
        out << setw(2) << it->toChar();
    }
    out << endl;

    for (it = allTiles.begin(); it != allTiles.end(); it++)
    {
        out << " " << this->m_bag.in(*it);
    }
    out << endl;
}


void Game::printPlayedRack(ostream &out, int n) const
{
    out << this->getPlayerRack(this->currPlayer()) << endl;
}


void Game::printAllRacks(ostream &out) const
{
    for (int j = 0; j < this->getNPlayers(); j++)
    {
        out << "Joueur " << j << ": ";
        out << this->getPlayerRack(j) << endl;
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


void Game::printSearchResults(ostream &out, const Training &iGame, int num) const
{
    for (int i = 0; i < num && i < iGame.getNResults(); i++)
    {
        out << setw(3) << i + 1 << ": ";
        searchResultLine(out, iGame, i);
        out << endl;
    }
}


void Game::printPoints(ostream &out) const
{
    out << this->getPlayerPoints(0) << endl;
}


void Game::printAllPoints(ostream &out) const
{
    for (int i = 0; i < this->getNPlayers(); i++)
    {
        out << "Joueur " << i << ": "
            << setw(4) << this->getPlayerPoints(i) << endl;
    }
}

