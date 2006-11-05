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
 *  \file   board_cross.cpp
 *  \brief  Build cross information used to speed up search
 *  \author Antoine Fraboulet & Olivier Teulière
 *  \date   2005
 */

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
    int i,left;
    unsigned int node, succ;

    oPoints = 0;

    /* Points on the left part */
    left = index;
    while (!iTiles[left - 1].isEmpty())
    {
        left--;
        if (!iJoker[left])
            oPoints += iTiles[left].getPoints();
    }

    // FIXME: create temporary strings until the dictionary uses Tile objects
    char leftTiles [BOARD_DIM + 1];
    char rightTiles[BOARD_DIM + 1];

    for (i = left; i < index; i++)
        leftTiles[i - left] = toupper(iTiles[i].toChar());
    leftTiles[index - left] = 0;

    for (i = index + 1; !iTiles[i].isEmpty(); i++)
        rightTiles[i - index - 1] = toupper(iTiles[i].toChar());
    rightTiles[i - index - 1] = 0;

    /* Tiles that can be played */
    node = Dic_char_lookup(iDic, Dic_root(iDic), leftTiles);
    if (node == 0) 
    {
        oCross.setNone();
        return;
    }

    for (succ = Dic_succ(iDic, node); succ; succ = Dic_next(iDic, succ))
    {
        if (Dic_word(iDic, Dic_char_lookup(iDic, succ, rightTiles)))
            oCross.insert(Tile(Dic_char(iDic, succ)));
        if (Dic_last(iDic, succ))
            break;
    }

    /* Points on the right part */
    /* yes, it is REALLY [index+1] */
    while (!iTiles[index+1].isEmpty())
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


/****************************************************************/
/****************************************************************/

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
