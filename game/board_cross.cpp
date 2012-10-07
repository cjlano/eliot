/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2012 Antoine Fraboulet & Olivier Teulière
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

#include <wctype.h>

#include <dic.h>
#include "tile.h"
#include "board.h"
#include "debug.h"


static void Board_checkout_tile(const Dictionary &iDic,
                                vector<Tile>& iTiles,
                                vector<bool> & iJoker,
                                Cross &oCross,
                                int& oPoints,
                                int index)
{
    oPoints = 0;

    /* Points on the left part */
    int left = index;
    while (!iTiles[left - 1].isEmpty())
    {
        left--;
        if (!iJoker[left])
            oPoints += iTiles[left].getPoints();
    }

    // FIXME: create temporary strings until the dictionary uses Tile objects
    wstring leftTiles;
    wstring rightTiles;

    for (int i = left; i < index; i++)
        leftTiles.push_back(towupper(iTiles[i].toChar()));

    for (int i = index + 1; !iTiles[i].isEmpty(); i++)
        rightTiles.push_back(towupper(iTiles[i].toChar()));

    /* Tiles that can be played */
    unsigned int node, succ;
    node = iDic.charLookup(iDic.getRoot(), leftTiles.c_str());
    if (node == 0)
    {
        oCross.setNone();
        return;
    }

    for (succ = iDic.getSucc(node); succ; succ = iDic.getNext(succ))
    {
        if (iDic.isEndOfWord(iDic.charLookup(succ, rightTiles.c_str())))
            oCross.insert(Tile(iDic.getChar(succ)));
        if (iDic.isLast(succ))
            break;
    }

    /* Points on the right part */
    /* yes, it is REALLY [index + 1] */
    while (!iTiles[index + 1].isEmpty())
    {
        index++;
        if (!iJoker[index])
            oPoints += iTiles[index].getPoints();
    }
}


static void Board_check(const Dictionary &iDic,
                        Matrix<Tile> &iTilesMx,
                        Matrix<bool> &iJokerMx,
                        Matrix<Cross> &iCrossMx,
                        Matrix<int> &iPointMx)
{
    for (int i = 1; i <= BOARD_DIM; i++)
    {
        for (int j = 1; j <= BOARD_DIM; j++)
        {
            iPointMx[j][i] = -1;
            if (!iTilesMx[i][j].isEmpty())
            {
                iCrossMx[j][i].setNone();
            }
            else if (!iTilesMx[i][j - 1].isEmpty() ||
                     !iTilesMx[i][j + 1].isEmpty())
            {
                iCrossMx[j][i].setNone();
                Board_checkout_tile(iDic,
                                    iTilesMx[i],
                                    iJokerMx[i],
                                    iCrossMx[j][i],
                                    iPointMx[j][i],
                                    j);
            }
            else
            {
                iCrossMx[j][i].setAny();
            }
        }
    }
}


void Board::buildCross(const Dictionary &iDic)
{
    Board_check(iDic, m_tilesRow, m_jokerRow, m_crossCol, m_pointCol);
    Board_check(iDic, m_tilesCol, m_jokerCol, m_crossRow, m_pointRow);
}

