/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: board_cross.cpp,v 1.2 2005/02/17 20:01:59 ipkiss Exp $
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
    unsigned int node, succ;
    int j;

    oPoints = 0;

    /* Points on the left part */
    int i = index;
    while (!iTiles[i - 1].isEmpty())
    {
        i--;
        if (!iJoker[i])
            oPoints += iTiles[i].getPoints();
    }

    /* Tiles that can be played */
    // FIXME: create a temporary string until the dictionary uses Tile objects
    char leftTiles[BOARD_DIM + 1];
    for (j = i; j < index; j++)
        leftTiles[j - i] = toupper(iTiles[j].toChar()) - 'A' + 1;
    leftTiles[index - i] = 0;
    node = Dic_lookup(iDic, Dic_root(iDic), leftTiles);
    if (node == 0)
    {
        oCross.clear();
        return;
    }

    // FIXME: same thing for the right part
    char rightTiles[BOARD_DIM + 1];
    for (j = index + 1; !iTiles[j].isEmpty(); j++)
        rightTiles[j - index - 1] = toupper(iTiles[j].toChar()) - 'A' + 1;
    rightTiles[j - index - 1] = 0;
    for (succ = Dic_succ(iDic, node); succ; succ = Dic_next(iDic, succ))
    {
        if (Dic_word(iDic, Dic_lookup(iDic, succ, rightTiles)))
            oCross.insert(Tile(Dic_chr(iDic, succ) + 'A' - 1));
        if (Dic_last(iDic, succ))
            break;
    }

     /* Points on the right part */
     while (!iTiles[index].isEmpty())
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
                iCrossMx[j][i].clear();
            else if (!iTilesMx[i][j - 1].isEmpty() ||
                     !iTilesMx[i][j + 1].isEmpty())
            {
                iCrossMx[j][i].clear();
                Board_checkout_tile(iDic,
                                    iTilesMx[i],
                                    iJokerMx[i],
                                    iCrossMx[j][i],
                                    iPointMx[j][i],
                                    j);
            }
            else
                iCrossMx[j][i].setAny();
        }
    }
}


void Board::buildCross(const Dictionary &iDic)
{
    Board_check(iDic, m_tilesRow, m_jokerRow, m_crossCol, m_pointCol);
    Board_check(iDic, m_tilesCol, m_jokerCol, m_crossRow, m_pointRow);
}
