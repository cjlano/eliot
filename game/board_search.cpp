/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: board_search.cpp,v 1.4 2005/10/23 14:53:43 ipkiss Exp $
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

#include <dic.h>
#include "tile.h"
#include "rack.h"
#include "round.h"
#include "results.h"
#include "board.h"

#include "debug.h"

/*
 * computes the score of a word, coordinates may be changed to reflect
 * the real direction of the word
 */
static void BoardSearchEvalMove(const Board &iBoard,
                                Matrix<Tile> &iTilesMx,
                                Matrix<int> &iPointsMx,
                                Matrix<bool> &iJokerMx,
                                Results &iResults, Round &iWord)
{
    int i, pts, ptscross;
    int l, t, fromrack;
    int len, row, col, wordmul;

    fromrack = 0;
    pts      = 0;
    ptscross = 0;
    wordmul  = 1;

    len = iWord.getWordLen();

    row = iWord.getRow();
    col = iWord.getCol();

    for (i = 0; i < len; i++)
    {
        if (!iTilesMx[row][col+i].isEmpty())
        {
            if (!iJokerMx[row][col+i])
                pts += iWord.getTile(i).getPoints();
        }
        else
        {
            if (!iWord.isJoker(i))
                l = iWord.getTile(i).getPoints() *
                    iBoard.getLetterMultiplier(row, col + i);
            else
                l = 0;
            pts += l;
            wordmul *= iBoard.getWordMultiplier(row, col + i);

            t = iPointsMx[row][col+i];
            if (t >= 0)
                ptscross += (t + l) * iBoard.getWordMultiplier(row, col + i);
            fromrack++;
        }
    }
    pts = ptscross + pts * wordmul + 50 * (fromrack == 7);
    iWord.setBonus(fromrack == 7);
    iWord.setPoints(pts);

    if (iWord.getDir() == VERTICAL)
    {
        iWord.setRow(col);
        iWord.setCol(row);
    }
    iResults.add(iWord);
    if (iWord.getDir() == VERTICAL)
    {
        iWord.setRow(row);
        iWord.setCol(col);
    }
}


static void ExtendRight(const Board &iBoard,
                        const Dictionary &iDic,
                        Matrix<Tile> &iTilesMx,
                        Matrix<Cross> &iCrossMx,
                        Matrix<int> &iPointsMx,
                        Matrix<bool> &iJokerMx,
                        Rack &iRack, Round &partialword,
                        Results &iResults, unsigned int iNode,
                        int iRow, int iCol, int anchor)
{
    Tile l;
    unsigned int succ;

    if (iTilesMx[iRow][iCol].isEmpty())
    {
        if (Dic_word(iDic, iNode) && iCol > anchor)
            BoardSearchEvalMove(iBoard, iTilesMx, iPointsMx, iJokerMx,
                                iResults, partialword);

        for (succ = Dic_succ(iDic, iNode); succ; succ = Dic_next(iDic, succ))
        {
            l = Tile('A' - 1 + Dic_chr(iDic, succ));
            if (iCrossMx[iRow][iCol].check(l))
            {
                if (iRack.in(l))
                {
                    iRack.remove(l);
                    partialword.addRightFromRack(l, 0);
                    ExtendRight(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                                iJokerMx, iRack, partialword, iResults,
                                succ, iRow, iCol + 1, anchor);
                    partialword.removeRightToRack(l, 0);
                    iRack.add(l);
                }
                if (iRack.in(Tile::Joker()))
                {
                    iRack.remove(Tile::Joker());
                    partialword.addRightFromRack(l, 1);
                    ExtendRight(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                                iJokerMx, iRack, partialword, iResults,
                                succ, iRow, iCol + 1, anchor);
                    partialword.removeRightToRack(l, 1);
                    iRack.add(Tile::Joker());
                }
            }
        }
    }
    else
    {
        l = iTilesMx[iRow][iCol];
        for (succ = Dic_succ(iDic, iNode); succ ; succ = Dic_next(iDic, succ))
        {
            if (Tile('A' - 1 + Dic_chr(iDic, succ)) == l)
            {
                partialword.addRightFromBoard(l);
                ExtendRight(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                            iJokerMx, iRack, partialword,
                            iResults, succ, iRow, iCol + 1, anchor);
                partialword.removeRightToBoard(l);
            }
        }
    }
}


static void LeftPart(const Board &iBoard,
                     const Dictionary &iDic,
                     Matrix<Tile> &iTilesMx,
                     Matrix<Cross> &iCrossMx,
                     Matrix<int> &iPointsMx,
                     Matrix<bool> &iJokerMx,
                     Rack &iRack, Round &iPartialWord,
                     Results &iResults, int n, int iRow, int anchor, int limit)
{
    Tile l;
    int succ;

    ExtendRight(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx, iJokerMx, iRack,
                iPartialWord, iResults, n, iRow, anchor, anchor);

    if (limit > 0)
    {
        for (succ = Dic_succ(iDic, n); succ; succ = Dic_next(iDic, succ))
        {
            l = Tile('A' - 1 + Dic_chr(iDic, succ));
            if (iRack.in(l))
            {
                iRack.remove(l);
                iPartialWord.addRightFromRack(l, 0);
                iPartialWord.setCol(iPartialWord.getCol() - 1);
                LeftPart(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                         iJokerMx, iRack, iPartialWord, iResults,
                         succ, iRow, anchor, limit - 1);
                iPartialWord.setCol(iPartialWord.getCol() + 1);
                iPartialWord.removeRightToRack(l, 0);
                iRack.add(l);
            }
            if (iRack.in(Tile::Joker()))
            {
                iRack.remove(Tile::Joker());
                iPartialWord.addRightFromRack(l, 1);
                iPartialWord.setCol(iPartialWord.getCol() - 1);
                LeftPart(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                         iJokerMx, iRack, iPartialWord, iResults,
                         succ, iRow, anchor, limit - 1);
                iPartialWord.setCol(iPartialWord.getCol() + 1);
                iPartialWord.removeRightToRack(l, 1);
                iRack.add(Tile::Joker());
            }
        }
    }
}


static void BoardSearchAux(const Board &iBoard,
                           const Dictionary &iDic,
                           Matrix<Tile> &iTilesMx,
                           Matrix<Cross> &iCrossMx,
                           Matrix<int> &iPointsMx,
                           Matrix<bool> &iJokerMx,
                           Rack &iRack, Results &iResults, Direction iDir)
{
    int row, col, lastanchor;
    Round partialword;
    for (row = 1; row <= BOARD_DIM; row++)
    {
        partialword.init();
        partialword.setDir(iDir);
        partialword.setRow(row);
        lastanchor = 0;
        for (col = 1; col <= BOARD_DIM; col++)
        {
            if (iTilesMx[row][col].isEmpty() &&
                (!iTilesMx[row][col - 1].isEmpty() ||
                 !iTilesMx[row][col + 1].isEmpty() ||
                 !iTilesMx[row - 1][col].isEmpty() ||
                 !iTilesMx[row + 1][col].isEmpty()))
            {
                if (!iTilesMx[row][col - 1].isEmpty())
                {
                    partialword.setCol(lastanchor + 1);
                    ExtendRight(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                                iJokerMx, iRack, partialword, iResults,
                                Dic_root(iDic), row, lastanchor + 1, col);
                }
                else
                {
                    partialword.setCol(col);
                    LeftPart(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                             iJokerMx, iRack, partialword, iResults,
                             Dic_root(iDic), row, col, col -
                             lastanchor - 1);
                }
                lastanchor = col;
            }
        }
    }
}


void Board::search(const Dictionary &iDic,
                   const Rack &iRack,
                   Results &oResults)
{
    // Create a copy of the rack to avoid modifying the given one
    Rack copyRack = iRack;

    BoardSearchAux(*this, iDic, m_tilesRow, m_crossRow,
                   m_pointRow, m_jokerRow,
                   copyRack, oResults, HORIZONTAL);

    BoardSearchAux(*this, iDic, m_tilesCol, m_crossCol,
                   m_pointCol, m_jokerCol,
                   copyRack, oResults, VERTICAL);
    oResults.sort();
}


void Board::searchFirst(const Dictionary &iDic,
                        const Rack &iRack,
                        Results &oResults)
{
    Round partialword;
    int row = 8, col = 8;

    // Create a copy of the rack to avoid modifying the given one
    Rack copyRack = iRack;

    partialword.setRow(row);
    partialword.setCol(col);
    partialword.setDir(HORIZONTAL);
    LeftPart(*this, iDic, m_tilesRow, m_crossRow,
             m_pointRow, m_jokerRow,
             copyRack, partialword, oResults, Dic_root(iDic), row, col,
             copyRack.nTiles() - 1);
    oResults.sort();
}
