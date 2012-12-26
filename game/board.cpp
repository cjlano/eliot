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

#include <cwctype>
#include <cstdio>

#include "dic.h"

#include "board.h"
#include "board_search.h"
#include "game_params.h"
#include "tile.h"
#include "round.h"
#include "rack.h"
#include "results.h"
#include "encoding.h"
#include "debug.h"

#define oo 0
#define __ 1
#define T2 2
#define T3 3
#define W2 2
#define W3 3


INIT_LOGGER(game, Board);


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


Board::Board(const GameParams &iParams):
    m_params(iParams),
    m_tilesRow(BOARD_REALDIM, Tile()),
    m_tilesCol(BOARD_REALDIM, Tile()),
    m_jokerRow(BOARD_REALDIM, false),
    m_jokerCol(BOARD_REALDIM, false),
    m_crossRow(BOARD_REALDIM, Cross()),
    m_crossCol(BOARD_REALDIM, Cross()),
    m_pointRow(BOARD_REALDIM, -1),
    m_pointCol(BOARD_REALDIM, -1),
    m_testsRow(BOARD_REALDIM, Tile()),
    m_isEmpty(true)
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


const Tile& Board::getTile(int iRow, int iCol) const
{
    return m_tilesRow[iRow][iCol];
}


wstring Board::getDisplayStr(int iRow, int iCol) const
{
    ASSERT(!isVacant(iRow, iCol),
           "Trying to get the display string on an empty board square");
    wstring str = getTile(iRow, iCol).getDisplayStr();
    if (isJoker(iRow, iCol))
        str = toLower(str);
    return str;
}


bool Board::isJoker(int iRow, int iCol) const
{
    return m_jokerRow[iRow][iCol];
}


bool Board::isVacant(int iRow, int iCol) const
{
    ASSERT(iRow >= BOARD_MIN && iRow <= BOARD_MAX &&
           iCol >= BOARD_MIN && iCol <= BOARD_MAX, "Invalid coordinates");
    return m_tilesRow[iRow][iCol].isEmpty();
}


void Board::addRound(const Dictionary &iDic, const Round &iRound)
{
    int row = iRound.getCoord().getRow();
    int col = iRound.getCoord().getCol();
    if (iRound.getCoord().getDir() == Coord::HORIZONTAL)
    {
        for (unsigned int i = 0; i < iRound.getWordLen(); i++)
        {
            const Tile &t = iRound.getTile(i);
            if (isVacant(row, col + i))
            {
                ASSERT(iRound.isPlayedFromRack(i), "Invalid round (1)");
                m_tilesRow[row][col + i] = t;
                m_jokerRow[row][col + i] = iRound.isJoker(i);
                m_tilesCol[col + i][row] = t;
                m_jokerCol[col + i][row] = iRound.isJoker(i);
            }
            else
            {
                ASSERT(!iRound.isPlayedFromRack(i), "Invalid round (2)");
                ASSERT(t == m_tilesRow[row][col + i].toUpper(), "Invalid round (3)");
            }
        }
    }
    else
    {
        for (unsigned int i = 0; i < iRound.getWordLen(); i++)
        {
            const Tile &t = iRound.getTile(i);
            if (isVacant(row + i, col))
            {
                ASSERT(iRound.isPlayedFromRack(i), "Invalid round (1)");
                m_tilesRow[row + i][col] = t;
                m_jokerRow[row + i][col] = iRound.isJoker(i);
                m_tilesCol[col][row + i] = t;
                m_jokerCol[col][row + i] = iRound.isJoker(i);
            }
            else
            {
                ASSERT(!iRound.isPlayedFromRack(i), "Invalid round (2)");
                ASSERT(t == m_tilesRow[row + i][col].toUpper(), "Invalid round (3)");
            }
        }
    }
    buildCross(iDic);
#ifdef DEBUG
    checkDouble();
#endif

    removeTestRound();

    m_isEmpty = false;
}


void Board::removeRound(const Dictionary &iDic, const Round &iRound)
{
    ASSERT(!m_isEmpty, "The board should not be empty");
    int row = iRound.getCoord().getRow();
    int col = iRound.getCoord().getCol();
    if (iRound.getCoord().getDir() == Coord::HORIZONTAL)
    {
        for (unsigned int i = 0; i < iRound.getWordLen(); i++)
        {
            ASSERT(iRound.getTile(i).toCode() == m_tilesRow[row][col + i].toCode(),
                   "Invalid round removal");
            if (iRound.isPlayedFromRack(i))
            {
                ASSERT(iRound.isJoker(i) == m_jokerRow[row][col + i],
                       "Invalid round removal");
                m_tilesRow[row][col + i] = Tile();
                m_jokerRow[row][col + i] = false;
                m_tilesCol[col + i][row] = Tile();
                m_jokerCol[col + i][row] = false;
            }
        }
    }
    else
    {
        for (unsigned int i = 0; i < iRound.getWordLen(); i++)
        {
            ASSERT(iRound.getTile(i).toCode() == m_tilesRow[row + i][col].toCode(),
                   "Invalid round removal");
            if (iRound.isPlayedFromRack(i))
            {
                ASSERT(iRound.isJoker(i) == m_jokerRow[row + i][col],
                       "Invalid round removal");
                m_tilesRow[row + i][col] = Tile();
                m_jokerRow[row + i][col] = false;
                m_tilesCol[col][row + i] = Tile();
                m_jokerCol[col][row + i] = false;
            }
        }
    }

    // Rebuild all the cross checks, because they are now invalid
    buildCross(iDic);
#ifdef DEBUG
    checkDouble();
#endif

    removeTestRound();

    // Update the m_isEmpty flag
    for (int i = 1; i <= BOARD_DIM; i++)
    {
        for (int j = 1; j <= BOARD_DIM; j++)
        {
            if (!isVacant(i, j))
                return;
        }
    }
    m_isEmpty = true;
}


/* XXX: There is duplicated code with board_search.c.
 * We could probably factorize something... */
int Board::checkRoundAux(const Matrix<Tile> &iTilesMx,
                         const Matrix<Cross> &iCrossMx,
                         const Matrix<int> &iPointsMx,
                         const Matrix<bool> &iJokerMx,
                         Round &iRound) const
{
    bool isolated = true;

    int fromrack = 0;
    int pts = 0;
    int ptscross = 0;
    int wordmul = 1;
    int row = iRound.getCoord().getRow();
    int col = iRound.getCoord().getCol();

    // Is the word going out of the board?
    if (col + iRound.getWordLen() > BOARD_MAX + 1)
        return 8;

    // Is the word an extension of another word?
    if (!iTilesMx[row][col - 1].isEmpty() ||
        !iTilesMx[row][col + iRound.getWordLen()].isEmpty())
    {
        return 1;
    }

    for (unsigned int i = 0; i < iRound.getWordLen(); i++)
    {
        const Tile &t = iRound.getTile(i);
        if (!iTilesMx[row][col + i].isEmpty())
        {
            // Using a joker tile to emulate the letter on the board is not allowed,
            // the plain letter should be used instead.
            // Also, make sure the played letter is the same as the one on the board
            if (iRound.isJoker(i) || iTilesMx[row][col+i].toCode() != t.toCode())
            {
                // Trying to overwrite a placed letter
                return 2;
            }

            isolated = false;
            iRound.setFromBoard(i);

            if (!iJokerMx[row][col + i])
                pts += t.getPoints();
        }
        else
        {
            // The letter is not yet on the board
            if (iCrossMx[row][col + i].check(t))
            {
                // A non-trivial cross-check means an anchor square
                if (!iCrossMx[row][col + i].isAny())
                    isolated = false;

                int l;
                if (!iRound.isJoker(i))
                    l = t.getPoints() * m_tileMultipliers[row][col + i];
                else
                    l = 0;
                pts += l;
                wordmul *= m_wordMultipliers[row][col + i];

                int p = iPointsMx[row][col + i];
                if (p >= 0)
                {
                    ptscross += (p + l) * m_wordMultipliers[row][col + i];
                }
                ++fromrack;
                iRound.setFromRack(i);
            }
            else
            {
                // The letter is not in the crosscheck
                return 3;
            }
        }
    }

    // There must be at least 1 letter from the rack
    if (fromrack == 0)
        return 4;

    // We may not be allowed to use so many letters from the rack
    // (cf. "7 among 8" variant)
    if (checkJunction && fromrack > m_params.getLettersToPlay())
        return 9;

    // The word must cover at least one anchor square, except
    // for the first turn
    if (isolated && !m_isEmpty)
        return 5;
    // The first word must be horizontal
    // Deactivated, as a vertical first word is allowed in free games,
    // and possibly in duplicate games as well (it depends on the sources)
#if 0
    if (m_isEmpty && iRound.getCoord().getDir() == Coord::VERTICAL)
        return 6;
#endif
    // The first word must cover the H8 square
    if (m_isEmpty
        && (row != 8 || col > 8 || col + iRound.getWordLen() <= 8))
    {
        return 7;
    }

    // Set the iPointsMx and bonus
    pts = ptscross + pts * wordmul;
    if (fromrack == m_params.getLettersToPlay())
    {
        pts += m_params.getBonusPoints();
        iRound.setBonus(true);
    }
    iRound.setPoints(pts);

    return 0;
}


int Board::checkRound(Round &iRound) const
{
    if (iRound.getCoord().getDir() == Coord::HORIZONTAL)
    {
        return checkRoundAux(m_tilesRow, m_crossRow,
                             m_pointRow, m_jokerRow, iRound);
    }
    else
    {
        // XXX: ugly!
        // Exchange the coordinates temporarily
        iRound.accessCoord().swap();

        int res = checkRoundAux(m_tilesCol, m_crossCol,
                                m_pointCol, m_jokerCol, iRound);

        // Restore the coordinates
        iRound.accessCoord().swap();

        return res;
    }
}


void Board::testRound(const Round &iRound)
{
    removeTestRound();

    int row = iRound.getCoord().getRow();
    int col = iRound.getCoord().getCol();
    if (iRound.getCoord().getDir() == Coord::HORIZONTAL)
    {
        for (unsigned int i = 0; i < iRound.getWordLen(); i++)
        {
            if (isVacant(row, col + i))
            {
                const Tile &t = iRound.getTile(i);
                if (iRound.isJoker(i))
                    m_testsRow[row][col + i] = t.toLower();
                else
                    m_testsRow[row][col + i] = t;
            }
        }
    }
    else
    {
        for (unsigned int i = 0; i < iRound.getWordLen(); i++)
        {
            if (isVacant(row + i, col))
            {
                const Tile &t = iRound.getTile(i);
                if (iRound.isJoker(i))
                    m_testsRow[row + i][col] = t.toLower();
                else
                    m_testsRow[row + i][col] = t;
            }
        }
    }
}


void Board::removeTestRound()
{
    m_testsRow = m_tilesRow;
}


bool Board::isTestChar(int iRow, int iCol) const
{
    return !m_testsRow[iRow][iCol].isEmpty() && isVacant(iRow, iCol);
}


const Tile& Board::getTestTile(int iRow, int iCol) const
{
    ASSERT(isTestChar(iRow, iCol), "The requested tile is not a test tile");
    return m_testsRow[iRow][iCol];
}


int Board::GetWordMultiplier(int iRow, int iCol)
{
    if (iRow < BOARD_MIN || iRow > BOARD_MAX ||
        iCol < BOARD_MIN || iCol > BOARD_MAX)
        return 0;
    return m_wordMultipliers[iRow][iCol];
}


int Board::GetLetterMultiplier(int iRow, int iCol)
{
    if (iRow < BOARD_MIN || iRow > BOARD_MAX ||
        iCol < BOARD_MIN || iCol > BOARD_MAX)
        return 0;
    return m_tileMultipliers[iRow][iCol];
}


// #define CELL_STRING_FORMAT "[%c:%s:%2d]"
#define CELL_STRING_FORMAT "[%s:%2d]"

string Board::getCellContent_row(int row, int col) const
{
    char buff[1024];  // [ joker, mask, point, tiles ]
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
            ASSERT(m_tilesRow[row][col] == m_tilesCol[col][row],
                   "Tiles inconsistency at " << row << "x" << col);
            ASSERT(m_jokerRow[row][col] == m_jokerCol[col][row],
                   "Jokers inconsistency at " << row << "x" << col);
            // The crossckecks and the points have no reason to be the same
            // in both directions
        }
    }
}
#endif


void Board::search(const Dictionary &iDic,
                   const Rack &iRack,
                   Results &oResults) const
{
    // Create a copy of the rack to avoid modifying the given one
    Rack copyRack = iRack;

    // Search horizontal words
    BoardSearch horizSearch(iDic, m_params, m_tilesRow, m_crossRow,
                            m_pointRow, m_jokerRow);
    horizSearch.search(copyRack, oResults, Coord::HORIZONTAL);

    // Search vertical words
    BoardSearch vertSearch(iDic, m_params, m_tilesCol, m_crossCol,
                            m_pointCol, m_jokerCol);
    vertSearch.search(copyRack, oResults, Coord::VERTICAL);
}


void Board::searchFirst(const Dictionary &iDic,
                        const Rack &iRack,
                        Results &oResults) const
{
    // Create a copy of the rack to avoid modifying the given one
    Rack copyRack = iRack;

    // Search horizontal words
    BoardSearch horizSearch(iDic, m_params, m_tilesRow, m_crossRow,
                            m_pointRow, m_jokerRow, true);
    horizSearch.search(copyRack, oResults, Coord::HORIZONTAL);
}

