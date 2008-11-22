/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2007 Antoine Fraboulet & Olivier Teulière
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

#include <dic.h>
#include "tile.h"
#include "rack.h"
#include "round.h"
#include "results.h"
#include "board.h"
#include "encoding.h"
#include "debug.h"
#include "game.h"

/*
 * computes the score of a word, coordinates may be changed to reflect
 * the real direction of the word
 */
static void BoardSearchEvalMove(const Board &iBoard,
                                const Matrix<Tile> &iTilesMx,
                                const Matrix<int> &iPointsMx,
                                const Matrix<bool> &iJokerMx,
                                Results &iResults, Round &iWord)
{
    unsigned int fromrack = 0;
    int pts      = 0;
    int ptscross = 0;
    int wordmul  = 1;

    unsigned int len = iWord.getWordLen();

    int row = iWord.getCoord().getRow();
    int col = iWord.getCoord().getCol();

    for (unsigned int i = 0; i < len; i++)
    {
        if (!iTilesMx[row][col+i].isEmpty())
        {
            if (!iJokerMx[row][col+i])
                pts += iWord.getTile(i).getPoints();
        }
        else
        {
            int l;
            if (!iWord.isJoker(i))
                l = iWord.getTile(i).getPoints() *
                    iBoard.GetLetterMultiplier(row, col + i);
            else
                l = 0;
            pts += l;
            wordmul *= iBoard.GetWordMultiplier(row, col + i);

            int t = iPointsMx[row][col+i];
            if (t >= 0)
                ptscross += (t + l) * iBoard.GetWordMultiplier(row, col + i);
            fromrack++;
        }
    }

    pts = ptscross + pts * wordmul + Game::BONUS_POINTS * (fromrack == Game::RACK_SIZE);
    iWord.setBonus(fromrack == Game::RACK_SIZE);
    iWord.setPoints(pts);

    if (iWord.getCoord().getDir() == Coord::VERTICAL)
    {
        // Exchange the coordinates temporarily
        iWord.accessCoord().swap();
    }
    iResults.add(iWord);
    if (iWord.getCoord().getDir() == Coord::VERTICAL)
    {
        // Restore the coordinates
        iWord.accessCoord().swap();
    }
}


static void ExtendRight(const Board &iBoard,
                        const Dictionary &iDic,
                        const Matrix<Tile> &iTilesMx,
                        const Matrix<Cross> &iCrossMx,
                        const Matrix<int> &iPointsMx,
                        const Matrix<bool> &iJokerMx,
                        Rack &iRack, Round &ioPartialWord,
                        Results &iResults, unsigned int iNode,
                        int iRow, int iCol, int iAnchor)
{
    Tile l;
    unsigned int succ;

    if (iTilesMx[iRow][iCol].isEmpty())
    {
        if (iDic.isEndOfWord(iNode) && iCol > iAnchor)
        {
            BoardSearchEvalMove(iBoard, iTilesMx, iPointsMx, iJokerMx,
                                iResults, ioPartialWord);
        }

        // Optimization: avoid entering the for loop if no tile can match
        if (iCrossMx[iRow][iCol].isNone())
            return;

        bool hasJokerInRack = iRack.in(Tile::Joker());
        for (succ = iDic.getSucc(iNode); succ; succ = iDic.getNext(succ))
        {
            l = Tile(iDic.getChar(succ));
            if (iCrossMx[iRow][iCol].check(l))
            {
                if (iRack.in(l))
                {
                    iRack.remove(l);
                    ioPartialWord.addRightFromRack(l, false);
                    ExtendRight(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                                iJokerMx, iRack, ioPartialWord, iResults,
                                succ, iRow, iCol + 1, iAnchor);
                    ioPartialWord.removeRightToRack(l, false);
                    iRack.add(l);
                }
                if (hasJokerInRack)
                {
                    iRack.remove(Tile::Joker());
                    ioPartialWord.addRightFromRack(l, true);
                    ExtendRight(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                                iJokerMx, iRack, ioPartialWord, iResults,
                                succ, iRow, iCol + 1, iAnchor);
                    ioPartialWord.removeRightToRack(l, true);
                    iRack.add(Tile::Joker());
                }
            }
        }
    }
    else
    {
        l = iTilesMx[iRow][iCol];
        wint_t upperChar = towupper(l.toChar());
        for (succ = iDic.getSucc(iNode); succ ; succ = iDic.getNext(succ))
        {
            if ((wint_t)iDic.getChar(succ) == upperChar)
            {
                ioPartialWord.addRightFromBoard(l);
                ExtendRight(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                            iJokerMx, iRack, ioPartialWord,
                            iResults, succ, iRow, iCol + 1, iAnchor);
                ioPartialWord.removeRightToBoard(l);
                // The letter will be present only once in the dictionary,
                // so we can stop looping
                break;
            }
        }
    }
}


static void LeftPart(const Board &iBoard,
                     const Dictionary &iDic,
                     const Matrix<Tile> &iTilesMx,
                     const Matrix<Cross> &iCrossMx,
                     const Matrix<int> &iPointsMx,
                     const Matrix<bool> &iJokerMx,
                     Rack &iRack, Round &ioPartialWord,
                     Results &iResults, int n, int iRow,
                     int iAnchor, int iLimit)
{
    Tile l;
    int succ;

    ExtendRight(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx, iJokerMx, iRack,
                ioPartialWord, iResults, n, iRow, iAnchor, iAnchor);

    if (iLimit > 0)
    {
        bool hasJokerInRack = iRack.in(Tile::Joker());
        for (succ = iDic.getSucc(n); succ; succ = iDic.getNext(succ))
        {
            l = Tile(iDic.getChar(succ));
            if (iRack.in(l))
            {
                iRack.remove(l);
                ioPartialWord.addRightFromRack(l, false);
                ioPartialWord.accessCoord().setCol(ioPartialWord.getCoord().getCol() - 1);
                LeftPart(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                         iJokerMx, iRack, ioPartialWord, iResults,
                         succ, iRow, iAnchor, iLimit - 1);
                ioPartialWord.accessCoord().setCol(ioPartialWord.getCoord().getCol() + 1);
                ioPartialWord.removeRightToRack(l, false);
                iRack.add(l);
            }
            if (hasJokerInRack)
            {
                iRack.remove(Tile::Joker());
                ioPartialWord.addRightFromRack(l, true);
                ioPartialWord.accessCoord().setCol(ioPartialWord.getCoord().getCol() - 1);
                LeftPart(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                         iJokerMx, iRack, ioPartialWord, iResults,
                         succ, iRow, iAnchor, iLimit - 1);
                ioPartialWord.accessCoord().setCol(ioPartialWord.getCoord().getCol() + 1);
                ioPartialWord.removeRightToRack(l, true);
                iRack.add(Tile::Joker());
            }
        }
    }
}


static void BoardSearchAux(const Board &iBoard,
                           const Dictionary &iDic,
                           const Matrix<Tile> &iTilesMx,
                           const Matrix<Cross> &iCrossMx,
                           const Matrix<int> &iPointsMx,
                           const Matrix<bool> &iJokerMx,
                           Rack &iRack, Results &iResults,
                           Coord::Direction iDir)
{
    int row, col, lastanchor;
    Round partialword;

    vector<Tile> rackTiles;
    iRack.getTiles(rackTiles);
    vector<Tile>::const_iterator it;
    vector<Tile>::const_iterator itEnd;

    for (row = 1; row <= BOARD_DIM; row++)
    {
        partialword.init();
        partialword.accessCoord().setDir(iDir);
        partialword.accessCoord().setRow(row);
        lastanchor = 0;
        for (col = 1; col <= BOARD_DIM; col++)
        {
            if (iTilesMx[row][col].isEmpty() &&
                (!iTilesMx[row][col - 1].isEmpty() ||
                 !iTilesMx[row][col + 1].isEmpty() ||
                 !iTilesMx[row - 1][col].isEmpty() ||
                 !iTilesMx[row + 1][col].isEmpty()))
            {
#if defined(DONT_USE_SEARCH_OPTIMIZATION)
                if (!iTilesMx[row][col - 1].isEmpty())
                {
                    partialword.accessCoord().setCol(lastanchor + 1);
                    ExtendRight(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                                iJokerMx, iRack, partialword, iResults,
                                Dic_root(iDic), row, lastanchor + 1, col);
                }
                else
                {
                    partialword.accessCoord().setCol(col);
                    LeftPart(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                             iJokerMx, iRack, partialword, iResults,
                             Dic_root(iDic), row, col, col -
                             lastanchor - 1);
                }
                lastanchor = col;
#else
                // Optimization compared to the original Appel & Jacobson
                // algorithm: skip Leftpart if none of the tiles of the rack
                // matches the cross mask for the current anchor
                bool match = false;
                for (it = rackTiles.begin(); it != rackTiles.end(); it++)
                {
                    if (iCrossMx[row][col].check(*it))
                    {
                        match = true;
                        break;
                    }
                }
                if (match)
                {
                    if (!iTilesMx[row][col - 1].isEmpty())
                    {
                        partialword.accessCoord().setCol(lastanchor + 1);
                        ExtendRight(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                                    iJokerMx, iRack, partialword, iResults,
                                    iDic.getRoot(), row, lastanchor + 1, col);
                    }
                    else
                    {
                        partialword.accessCoord().setCol(col);
                        LeftPart(iBoard, iDic, iTilesMx, iCrossMx, iPointsMx,
                                 iJokerMx, iRack, partialword, iResults,
                                 iDic.getRoot(), row, col, col -
                                 lastanchor - 1);
                    }
                }
                lastanchor = col;
#endif
            }
        }
    }
}


void Board::search(const Dictionary &iDic,
                   const Rack &iRack,
                   Results &oResults) const
{
    // Create a copy of the rack to avoid modifying the given one
    Rack copyRack = iRack;

    BoardSearchAux(*this, iDic, m_tilesRow, m_crossRow,
                   m_pointRow, m_jokerRow,
                   copyRack, oResults, Coord::HORIZONTAL);

    BoardSearchAux(*this, iDic, m_tilesCol, m_crossCol,
                   m_pointCol, m_jokerCol,
                   copyRack, oResults, Coord::VERTICAL);
}


void Board::searchFirst(const Dictionary &iDic,
                        const Rack &iRack,
                        Results &oResults) const
{
    int row = 8, col = 8;

    // Create a copy of the rack to avoid modifying the given one
    Rack copyRack = iRack;

    Round partialword;
    partialword.accessCoord().setRow(row);
    partialword.accessCoord().setCol(col);
    partialword.accessCoord().setDir(Coord::HORIZONTAL);
    LeftPart(*this, iDic, m_tilesRow, m_crossRow,
             m_pointRow, m_jokerRow,
             copyRack, partialword, oResults, iDic.getRoot(), row, col,
             copyRack.getNbTiles() - 1);
}

