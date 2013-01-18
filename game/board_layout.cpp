/*****************************************************************************
 * Eliot
 * Copyright (C) 2012 Olivier Teulière
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
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

#include "board_layout.h"

#include "debug.h"

#define oo 0
#define __ 1
#define T2 2
#define T3 3
#define W2 2
#define W3 3

#define BOARD_DIM 15
#define BOARD_REALDIM (BOARD_DIM + 2)


INIT_LOGGER(game, BoardLayout);


static const int DefaultTileMultipliers[BOARD_REALDIM][BOARD_REALDIM] =
{
    { oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo },
    { oo,__,__,__,T2,__,__,__,__,__,__,__,T2,__,__,__,oo },
    { oo,__,__,__,__,__,T3,__,__,__,T3,__,__,__,__,__,oo },
    { oo,__,__,__,__,__,__,T2,__,T2,__,__,__,__,__,__,oo },
    { oo,T2,__,__,__,__,__,__,T2,__,__,__,__,__,__,T2,oo },
    { oo,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,oo },
    { oo,__,T3,__,__,__,T3,__,__,__,T3,__,__,__,T3,__,oo },
    { oo,__,__,T2,__,__,__,T2,__,T2,__,__,__,T2,__,__,oo },
    { oo,__,__,__,T2,__,__,__,__,__,__,__,T2,__,__,__,oo },
    { oo,__,__,T2,__,__,__,T2,__,T2,__,__,__,T2,__,__,oo },
    { oo,__,T3,__,__,__,T3,__,__,__,T3,__,__,__,T3,__,oo },
    { oo,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,oo },
    { oo,T2,__,__,__,__,__,__,T2,__,__,__,__,__,__,T2,oo },
    { oo,__,__,__,__,__,__,T2,__,T2,__,__,__,__,__,__,oo },
    { oo,__,__,__,__,__,T3,__,__,__,T3,__,__,__,__,__,oo },
    { oo,__,__,__,T2,__,__,__,__,__,__,__,T2,__,__,__,oo },
    { oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo }
};


static const int DefaultWordMultipliers[BOARD_REALDIM][BOARD_REALDIM] =
{
    { oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo },
    { oo,W3,__,__,__,__,__,__,W3,__,__,__,__,__,__,W3,oo },
    { oo,__,W2,__,__,__,__,__,__,__,__,__,__,__,W2,__,oo },
    { oo,__,__,W2,__,__,__,__,__,__,__,__,__,W2,__,__,oo },
    { oo,__,__,__,W2,__,__,__,__,__,__,__,W2,__,__,__,oo },
    { oo,__,__,__,__,W2,__,__,__,__,__,W2,__,__,__,__,oo },
    { oo,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,oo },
    { oo,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,oo },
    { oo,W3,__,__,__,__,__,__,W2,__,__,__,__,__,__,W3,oo },
    { oo,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,oo },
    { oo,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,oo },
    { oo,__,__,__,__,W2,__,__,__,__,__,W2,__,__,__,__,oo },
    { oo,__,__,__,W2,__,__,__,__,__,__,__,W2,__,__,__,oo },
    { oo,__,__,W2,__,__,__,__,__,__,__,__,__,W2,__,__,oo },
    { oo,__,W2,__,__,__,__,__,__,__,__,__,__,__,W2,__,oo },
    { oo,W3,__,__,__,__,__,__,W3,__,__,__,__,__,__,W3,oo },
    { oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo,oo }
};


// Initialize the static member
BoardLayout BoardLayout::m_defaultLayout;


BoardLayout::BoardLayout()
{
    setDefaultLayout();
}


unsigned BoardLayout::getRowCount() const
{
    ASSERT(m_wordMultipliers.size() > 2 && m_tileMultipliers.size() > 2,
           "Invalid board size");
    return m_wordMultipliers.size() - 2;
}


unsigned BoardLayout::getColCount() const
{
    ASSERT(m_wordMultipliers.size() > 2 && m_tileMultipliers.size() > 2,
           "Invalid board size");
    return m_wordMultipliers[0].size() - 2;
}


int BoardLayout::getWordMultiplier(unsigned iRow, unsigned iCol) const
{
    if (!isValidCoord(iRow, iCol))
        return 0;
    return m_wordMultipliers[iRow][iCol];
}


int BoardLayout::getLetterMultiplier(unsigned iRow, unsigned iCol) const
{
    if (!isValidCoord(iRow, iCol))
        return 0;
    return m_tileMultipliers[iRow][iCol];
}


bool BoardLayout::isValidCoord(unsigned iRow, unsigned iCol) const
{
    return (iRow >= 1 && iRow <= getRowCount() &&
            iCol >= 1 && iCol <= getColCount());
}


static void InitMatrixFromArray(Matrix<int> &oMatrix, const int iArray[BOARD_REALDIM][BOARD_REALDIM])
{
    oMatrix.resize(BOARD_REALDIM, BOARD_REALDIM, 0);
    for (unsigned i = 0; i < BOARD_REALDIM; ++i)
    {
        for (unsigned j = 0; j < BOARD_REALDIM; ++j)
        {
            oMatrix[i][j] = iArray[i][j];
        }
    }
}



void BoardLayout::setDefaultLayout()
{
    InitMatrixFromArray(m_wordMultipliers, DefaultWordMultipliers);
    InitMatrixFromArray(m_tileMultipliers, DefaultTileMultipliers);
}


const BoardLayout & BoardLayout::GetDefault()
{
    return m_defaultLayout;
}

