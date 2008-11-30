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

#include <boost/foreach.hpp>

#include <iomanip>
#include <string>
#include <stdlib.h>

#include <dic.h>
#include "game_io.h"
#include "public_game.h"
#include "bag.h"
#include "board.h"
#include "results.h"
#include "player.h"
#include "encoding.h"

using namespace std;

#define __UNUSED__ __attribute__((unused))

void GameIO::printBoard(ostream &out, const PublicGame &iGame)
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
            wchar_t l = iGame.getBoard().getChar(row, col);
            if (l == 0)
                out << "  -";
            else
                out << padAndConvert(wstring(1, l), 3);
        }
        out << endl;
    }
}

/* this mode is used for regression tests */
void GameIO::printBoardDebug(ostream &out, const PublicGame &iGame)
{
    int row, col;

    /* first printf row cell contents */
    for (row = BOARD_MIN; row <= BOARD_MAX; row++)
    {
        out << " " << (char)(row - BOARD_MIN + 'A') << "r ";
        for (col = BOARD_MIN; col <= BOARD_MAX; col++)
        {
            out << iGame.getBoard().getCellContent_row(row, col);
        }
        out << endl;
    }
    out << " -" << endl;
    for (row = BOARD_MIN; row <= BOARD_MAX; row++)
    {
        out << " " << (char)(row - BOARD_MIN + 'A') << "c ";
        for (col = BOARD_MIN; col <= BOARD_MAX; col++)
        {
            out << iGame.getBoard().getCellContent_col(row, col);
        }
        out << endl;
    }
}

void GameIO::printBoardJoker(ostream &out, const PublicGame &iGame)
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
            wchar_t l = iGame.getBoard().getChar(row, col);
            bool j = (iGame.getBoard().getCharAttr(row, col) & ATTR_JOKER);

            if (l == 0)
                out << " " << (j ? "." : "--");
            else
                out << " " << (j ? "." : " ") << convertToMb(l);
        }
        out << endl;
    }
}


void GameIO::printBoardMultipliers(ostream &out, const PublicGame &iGame)
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
            wchar_t l = iGame.getBoard().getChar(row, col);
            if (l != 0)
                out << padAndConvert(wstring(1, l), 3);
            else
            {
                int wm = iGame.getBoard().GetWordMultiplier(row, col);
                int tm = iGame.getBoard().GetLetterMultiplier(row, col);

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


void GameIO::printBoardMultipliers2(ostream &out, const PublicGame &iGame)
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
            wchar_t l = iGame.getBoard().getChar(row, col);
            int wm = iGame.getBoard().GetWordMultiplier(row, col);
            int tm = iGame.getBoard().GetLetterMultiplier(row, col);

            if (wm > 1)
                out << " " << ((wm == 3) ? '@' : '#');
            else if (tm > 1)
                out << " " << ((tm == 3) ? '*' : '+');
            else
                out << " -";
            out << (l ? convertToMb(l) : "-");
        }
        out << endl;
    }
}


void GameIO::printNonPlayed(ostream &out, const PublicGame &iGame)
{
    const Bag &bag = iGame.getBag();
    BOOST_FOREACH(const Tile &tile, iGame.getDic().getAllTiles())
    {
        if (bag.in(tile) > 9)
            out << " ";
        out << setw(2) << convertToMb(tile.toChar());
    }
    out << endl;

    BOOST_FOREACH(const Tile &tile, iGame.getDic().getAllTiles())
    {
        out << " " << bag.in(tile);
    }
    out << endl;
}


void GameIO::printPlayedRack(ostream &out, const PublicGame &iGame, int __UNUSED__ n)
{
    out << convertToMb(iGame.getCurrentPlayer().getCurrentRack().toString(PlayedRack::RACK_SIMPLE)) << endl;
}


void GameIO::printAllRacks(ostream &out, const PublicGame &iGame)
{
    for (unsigned int j = 0; j < iGame.getNbPlayers(); j++)
    {
        out << "Joueur " << j << ": ";
        out << convertToMb(iGame.getPlayer(j).getCurrentRack().toString(PlayedRack::RACK_SIMPLE)) << endl;
    }
}


static void searchResultLine(ostream &out, const Results &iResults, int num)
{
    const Round &r = iResults.get(num);
    const wstring &word = r.getWord();
    if (word.size() == 0)
        return;
    out << convertToMb(word) << string(16 - word.size(), ' ')
        << (r.getBonus() ? '*' : ' ')
        << setw(4) << r.getPoints()
        << ' ' << convertToMb(r.getCoord().toString());
}


void GameIO::printSearchResults(ostream &out, const Results &iResults, int num)
{
    for (int i = 0; i < num && i < (int)iResults.size(); i++)
    {
        out << setw(3) << i + 1 << ": ";
        searchResultLine(out, iResults, i);
        out << endl;
    }
}


void GameIO::printPoints(ostream &out, const PublicGame &iGame)
{
    out << iGame.getPlayer(0).getPoints() << endl;
}


void GameIO::printAllPoints(ostream &out, const PublicGame &iGame)
{
    for (unsigned int i = 0; i < iGame.getNbPlayers(); i++)
    {
        out << "Joueur " << i << ": "
            << setw(4) << iGame.getPlayer(i).getPoints() << endl;
    }
}


void GameIO::printGameDebug(ostream &out, const PublicGame &iGame)
{
    out << "Game:: joueur en cours " << iGame.getCurrentPlayer().getId()
        << " sur " << iGame.getNbPlayers() << endl;
    out << "Game:: mode " << iGame.getModeAsString() << endl;
    out << "Game:: variante ";
    switch (iGame.getVariant())
    {
        case PublicGame::kNONE:
            out << "aucune" << endl;
            break;
        case PublicGame::kJOKER:
            out << "joker" << endl;
            break;
        default:
            out << "inconnu" << endl;
            break;
    }
    out << "Game:: history --" << endl;
    out << convertToMb(iGame.getHistory().toString());
    out << "--" << endl;
    out << "" << endl;
}
