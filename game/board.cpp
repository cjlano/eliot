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

#include <wctype.h>
#include "dic.h"
#include "tile.h"
#include "round.h"
#include "bag.h"
#include "rack.h"
#include "results.h"
#include "board.h"
#include "debug.h"

#define oo 0
#define __ 1
#define T2 2
#define T3 3
#define W2 2
#define W3 3


const int Board::m_tileMultipliers[BOARD_REALDIM][BOARD_REALDIM] =
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


const int Board::m_wordMultipliers[BOARD_REALDIM][BOARD_REALDIM] =
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


Board::Board():
    m_tilesRow(BOARD_REALDIM, Tile::dummy()),
    m_tilesCol(BOARD_REALDIM, Tile::dummy()),
    m_jokerRow(BOARD_REALDIM, false),
    m_jokerCol(BOARD_REALDIM, false),
    m_crossRow(BOARD_REALDIM, Cross()),
    m_crossCol(BOARD_REALDIM, Cross()),
    m_pointRow(BOARD_REALDIM, -1),
    m_pointCol(BOARD_REALDIM, -1),
    m_testsRow(BOARD_REALDIM, 0)
{
    // No cross check allowed around the board
    for (int i = 0; i < BOARD_REALDIM; i++)
    {
        m_crossRow[0][i].setNone();
        m_crossCol[0][i].setNone();
        m_crossRow[i][0].setNone();
        m_crossCol[i][0].setNone();
        m_crossRow[BOARD_REALDIM - 1][i].setNone();
        m_crossCol[BOARD_REALDIM - 1][i].setNone();
        m_crossRow[i][BOARD_REALDIM - 1].setNone();
        m_crossCol[i][BOARD_REALDIM - 1].setNone();
    }
}


wchar_t Board::getChar(int iRow, int iCol) const
{
    wchar_t letter = 0;
    Tile tile = getTile(iRow, iCol);
    if (!tile.isEmpty())
    {
        letter = tile.toChar();
        if (isJoker(iRow, iCol))
            letter = towlower(letter);
    }
    return letter;
}

int Board::getCharAttr(int iRow, int iCol) const
{
    int t = getTestChar(iRow, iCol);
    int j = isJoker(iRow, iCol);
    return  (t << 1) | j;
}


Tile Board::getTile(int iRow, int iCol) const
{
    return m_tilesRow[iRow][iCol];
}


bool Board::isJoker(int iRow, int iCol) const
{
    return m_jokerRow[iRow][iCol];
}


bool Board::isVacant(int iRow, int iCol) const
{
    if (iRow < 1 || iRow > BOARD_DIM ||
        iCol < 1 || iCol > BOARD_DIM)
    {
        return false;
    }
    return m_tilesRow[iRow][iCol].isEmpty();
}


void Board::addRound(const Dictionary &iDic, const Round &iRound)
{
    Tile t;
    int row, col;

    row = iRound.getCoord().getRow();
    col = iRound.getCoord().getCol();
    if (iRound.getCoord().getDir() == Coord::HORIZONTAL)
    {
        for (int i = 0; i < iRound.getWordLen(); i++)
        {
            if (m_tilesRow[row][col + i].isEmpty())
            {
                t = iRound.getTile(i);
                m_tilesRow[row][col + i] = t;
                m_jokerRow[row][col + i] = iRound.isJoker(i);
                m_tilesCol[col + i][row] = t;
                m_jokerCol[col + i][row] = iRound.isJoker(i);
            }
        }
    }
    else
    {
        for (int i = 0; i < iRound.getWordLen(); i++)
        {
            if (m_tilesRow[row + i][col].isEmpty())
            {
                t = iRound.getTile(i);
                m_tilesRow[row + i][col] = t;
                m_jokerRow[row + i][col] = iRound.isJoker(i);
                m_tilesCol[col][row + i] = t;
                m_jokerCol[col][row + i] = iRound.isJoker(i);
            }
        }
    }
    buildCross(iDic);
#ifdef DEBUG
    checkDouble();
#endif
}


void Board::removeRound(const Dictionary &iDic, const Round &iRound)
{
    int row, col;

    row = iRound.getCoord().getRow();
    col = iRound.getCoord().getCol();
    if (iRound.getCoord().getDir() == Coord::HORIZONTAL)
    {
        for (int i = 0; i < iRound.getWordLen(); i++)
        {
            if (iRound.isPlayedFromRack(i))
            {
                m_tilesRow[row][col + i] = Tile::dummy();
                m_jokerRow[row][col + i] = false;
                m_crossRow[row][col + i].setAny();
                m_tilesCol[col + i][row] = Tile::dummy();
                m_jokerCol[col + i][row] = false;
                m_crossCol[col + i][row].setAny();
            }
        }
    }
    else
    {
        for (int i = 0; i < iRound.getWordLen(); i++)
        {
            if (iRound.isPlayedFromRack(i))
            {
                m_tilesRow[row + i][col] = Tile::dummy();
                m_jokerRow[row + i][col] = false;
                m_crossRow[row + i][col].setAny();
                m_tilesCol[col][row + i] = Tile::dummy();
                m_jokerCol[col][row + i] = false;
                m_crossCol[col][row + i].setAny();
            }
        }
    }
    buildCross(iDic);
#ifdef DEBUG
    checkDouble();
#endif
}


/* XXX: There is duplicated code with board_search.c.
 * We could probably factorize something... */
int Board::checkRoundAux(Matrix<Tile> &iTilesMx,
                         Matrix<Cross> &iCrossMx,
                         Matrix<int> &iPointsMx,
                         Matrix<bool> &iJokerMx,
                         Round &iRound,
                         bool firstturn)
{
    Tile t;
    int row, col, i;
    int l, p, fromrack;
    int pts, ptscross, wordmul;
    bool isolated = true;

    fromrack = 0;
    pts = 0;
    ptscross = 0;
    wordmul = 1;
    row = iRound.getCoord().getRow();
    col = iRound.getCoord().getCol();

    /* Is the word an extension of another word? */
    if (!iTilesMx[row][col - 1].isEmpty() ||
        !iTilesMx[row][col + iRound.getWordLen()].isEmpty())
    {
        return 1;
    }

    for (i = 0; i < iRound.getWordLen(); i++)
    {
        t = iRound.getTile(i);
        if (!iTilesMx[row][col + i].isEmpty())
        {
            /* There is already a letter on the board */
            if (iTilesMx[row][col + i] != t)
            {
                /* Check if it is only a joker */
                if ((iTilesMx[row][col+i].toCode() == t.toCode()) && iTilesMx[row][col+i].isJoker())
                {
                    // Do nothing, we don't need to change the tile in the round
                    // iRound.setJoker(i,true);
                    debug("load: play on joker for letter %d (%lc)\n", i, iRound.getTile(i).toChar());
                }
                else
                {
                    debug("load: overwriting tile %lc with %lc\n",
                          iTilesMx[row][col+i].toChar(),
                          t.toChar());
                    return 2;
                }
            }

            isolated = false;
            iRound.setFromBoard(i);

            if (!iJokerMx[row][col + i])
                pts += t.getPoints();
        }
        else
        {
            /* The letter is not yet on the board */
            if (iCrossMx[row][col + i].check(t))
            {
                /* A non-trivial cross-check means an anchor square */
                if (!iCrossMx[row][col + i].isAny())
                    isolated = false;

                if (!iRound.isJoker(i))
                    l = t.getPoints() * m_tileMultipliers[row][col + i];
                else
                    l = 0;
                pts += l;
                wordmul *= m_wordMultipliers[row][col + i];

                p = iPointsMx[row][col + i];
                if (p >= 0)
                {
                    ptscross += (p + l) * m_wordMultipliers[row][col + i];
                }
                fromrack++;
                iRound.setFromRack(i);
            }
            else
            {
                /* The letter is not in the crosscheck */
                return 3;
            }
        }
    }

    /* There must be at least 1 letter from the rack */
    if (fromrack == 0)
        return 4;

    /* The word must cover at least one anchor square, except
     * for the first turn */
    if (isolated && !firstturn)
        return 5;
    /* The first word must be horizontal */
    if (firstturn && iRound.getCoord().getDir() == Coord::VERTICAL)
        return 6;
    /* The first word must cover the H8 square */
    if (firstturn
        && (row != 8 || col > 8 || col + iRound.getWordLen() <= 8))
    {
        return 7;
    }

    /* Set the iPointsMx and bonus */
    pts = ptscross + pts * wordmul + 50 * (fromrack == 7);
    iRound.setPoints(pts);
    iRound.setBonus(fromrack == 7);

    return 0;
}


int Board::checkRound(Round &iRound, bool firstturn)
{
    if (iRound.getCoord().getDir() == Coord::HORIZONTAL)
    {
        return checkRoundAux(m_tilesRow, m_crossRow,
                             m_pointRow, m_jokerRow,
                             iRound, firstturn);
    }
    else
    {
        // XXX: ugly!
        // Exchange the coordinates temporarily
        iRound.accessCoord().swap();

        int res = checkRoundAux(m_tilesCol, m_crossCol,
                                m_pointCol, m_jokerCol,
                                iRound, firstturn);

        // Restore the coordinates
        iRound.accessCoord().swap();

        return res;
    }
}


void Board::testRound(const Round &iRound)
{
    Tile t;
    int row, col;

    row = iRound.getCoord().getRow();
    col = iRound.getCoord().getCol();
    if (iRound.getCoord().getDir() == Coord::HORIZONTAL)
    {
        for (int i = 0; i < iRound.getWordLen(); i++)
        {
            if (m_tilesRow[row][col + i].isEmpty())
            {
                t = iRound.getTile(i);
                m_tilesRow[row][col + i] = t;
                m_jokerRow[row][col + i] = iRound.isJoker(i);
                m_testsRow[row][col + i] = 1;

                m_tilesCol[col + i][row] = t;
                m_jokerCol[col + i][row] = iRound.isJoker(i);
            }
        }
    }
    else
    {
        for (int i = 0; i < iRound.getWordLen(); i++)
        {
            if (m_tilesRow[row + i][col].isEmpty())
            {
                t = iRound.getTile(i);
                m_tilesRow[row + i][col] = t;
                m_jokerRow[row + i][col] = iRound.isJoker(i);
                m_testsRow[row + i][col] = 1;

                m_tilesCol[col][row + i] = t;
                m_jokerCol[col][row + i] = iRound.isJoker(i);
            }
        }
    }
}


void Board::removeTestRound()
{
    for (int row = 1; row <= BOARD_DIM; row++)
    {
        for (int col = 1; col <= BOARD_DIM; col++)
        {
            if (m_testsRow[row][col])
            {
                m_tilesRow[row][col] = Tile::dummy();
                m_testsRow[row][col] = 0;
                m_jokerRow[row][col] = false;

                m_tilesCol[col][row] = Tile::dummy();
                m_jokerCol[col][row] = false;
            }
        }
    }
}


char Board::getTestChar(int iRow, int iCol) const
{
    return m_testsRow[iRow][iCol];
}


int Board::getWordMultiplier(int iRow, int iCol) const
{
    if (iRow < BOARD_MIN || iRow > BOARD_MAX ||
        iCol < BOARD_MIN || iCol > BOARD_MAX)
        return 0;
    return m_wordMultipliers[iRow][iCol];
}


int Board::getLetterMultiplier(int iRow, int iCol) const
{
    if (iRow < BOARD_MIN || iRow > BOARD_MAX ||
        iCol < BOARD_MIN || iCol > BOARD_MAX)
        return 0;
    return m_tileMultipliers[iRow][iCol];
}


#define CELL_STRING_FORMAT "[%s:%2d]"

string Board::getCellContent_row(int row, int col) const
{
    char buff[1024];  /* [ joker, mask, point, tiles ] */
    sprintf(buff,CELL_STRING_FORMAT,
            // m_jokerRow[row][col] ? 'j':'.',
            m_crossRow[row][col].getHexContent().c_str(),
            m_pointRow[row][col]);
    return string(buff);
}

string Board::getCellContent_col(int row, int col) const
{
    char buff[1024];
    sprintf(buff,CELL_STRING_FORMAT,
            // m_jokerCol[col][row] ? 'j':'.',
            m_crossCol[col][row].getHexContent().c_str(),
            m_pointCol[col][row]);
    return string(buff);
}


#ifdef DEBUG
void Board::checkDouble()
{
    for (int row = BOARD_MIN; row <= BOARD_MAX; row++)
    {
        for (int col = BOARD_MIN; col <= BOARD_MAX; col++)
        {
            if (m_tilesRow[row][col] != m_tilesCol[col][row])
                printf("tiles diff %d %d\n", row, col);

            // The crossckecks and the points have no reason to be the same
            // in both directions
            /*
            if (m_crossRow[row][col] != m_crossCol[col][row])
            {
                printf("cross diff %d %d\n",row,col);
            }

            if (m_pointRow[row][col] != m_pointCol[col][row])
                printf("point diff %d %d\n",row,col);
            */

            if (m_jokerRow[row][col] != m_jokerCol[col][row])
                printf("joker diff %d %d\n", row, col);
        }
    }
}
#endif

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
