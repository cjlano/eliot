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

#include <cwctype> // For towupper

#include "board_search.h"
#include "dic.h"
#include "game_params.h"
#include "board.h"
#include "tile.h"
#include "rack.h"
#include "round.h"
#include "results.h"


BoardSearch::BoardSearch(const Dictionary &iDic,
                         const GameParams &iParams,
                         const Matrix<Tile> &iTilesMx,
                         const Matrix<Cross> &iCrossMx,
                         const Matrix<int> &iPointsMx,
                         const Matrix<bool> &iJokerMx,
                         bool isFirstTurn)
    : m_dic(iDic), m_params(iParams), m_tilesMx(iTilesMx), m_crossMx(iCrossMx),
      m_pointsMx(iPointsMx), m_jokerMx(iJokerMx), m_firstTurn(isFirstTurn)
{
}


void BoardSearch::search(Rack &iRack, Results &oResults, Coord::Direction iDir) const
{
    // Handle the first turn specifically
    if (m_firstTurn)
    {
        const int row = 8, col = 8;
        Round tmpRound;
        tmpRound.accessCoord().setRow(row);
        tmpRound.accessCoord().setCol(col);
        tmpRound.accessCoord().setDir(Coord::HORIZONTAL);
        leftPart(iRack, tmpRound, oResults, m_dic.getRoot(),
                 row, col, std::min(iRack.getNbTiles(), (unsigned)col) - 1);
        return;
    }


    vector<Tile> rackTiles;
    iRack.getTiles(rackTiles);
    vector<Tile>::const_iterator it;

    for (int row = 1; row <= BOARD_DIM; row++)
    {
        Round partialWord;
        partialWord.accessCoord().setDir(iDir);
        partialWord.accessCoord().setRow(row);
        int lastanchor = 0;
        for (int col = 1; col <= BOARD_DIM; col++)
        {
            if (m_tilesMx[row][col].isEmpty() &&
                (!m_tilesMx[row][col - 1].isEmpty() ||
                 !m_tilesMx[row][col + 1].isEmpty() ||
                 !m_tilesMx[row - 1][col].isEmpty() ||
                 !m_tilesMx[row + 1][col].isEmpty()))
            {
#ifdef DONT_USE_SEARCH_OPTIMIZATION
                if (!m_tilesMx[row][col - 1].isEmpty())
                {
                    partialWord.accessCoord().setCol(lastanchor + 1);
                    extendRight(iRack, partialWord, oResults,
                                m_dic.getRoot(), row, lastanchor + 1, col);
                }
                else
                {
                    partialWord.accessCoord().setCol(col);
                    leftPart(iRack, partialWord, oResults,
                             m_dic.getRoot(), row, col, col - lastanchor - 1);
                }
                lastanchor = col;
#else
                // Optimization compared to the original Appel & Jacobson
                // algorithm: skip leftPart if none of the tiles of the rack
                // matches the cross mask for the current anchor
                bool match = false;
                for (it = rackTiles.begin(); it != rackTiles.end(); it++)
                {
                    if (m_crossMx[row][col].check(*it))
                    {
                        match = true;
                        break;
                    }
                }
                if (match)
                {
                    if (!m_tilesMx[row][col - 1].isEmpty())
                    {
                        partialWord.accessCoord().setCol(lastanchor + 1);
                        extendRight(iRack, partialWord, oResults,
                                    m_dic.getRoot(), row, lastanchor + 1, col);
                    }
                    else
                    {
                        partialWord.accessCoord().setCol(col);
                        leftPart(iRack, partialWord, oResults,
                                 m_dic.getRoot(), row, col, col - lastanchor - 1);
                    }
                }
                lastanchor = col;
#endif
            }
        }
    }
}


void BoardSearch::leftPart(Rack &iRack, Round &ioPartialWord,
                           Results &oResults, int n, int iRow,
                           int iAnchor, int iLimit) const
{
    extendRight(iRack, ioPartialWord, oResults, n, iRow, iAnchor, iAnchor);

    if (iLimit > 0)
    {
        bool hasJokerInRack = iRack.contains(Tile::Joker());
        for (unsigned int succ = m_dic.getSucc(n); succ; succ = m_dic.getNext(succ))
        {
            const Tile &l = Tile(m_dic.getChar(succ));
            if (iRack.contains(l))
            {
                iRack.remove(l);
                ioPartialWord.addRightFromRack(l, false);
                ioPartialWord.accessCoord().setCol(ioPartialWord.getCoord().getCol() - 1);
                leftPart(iRack, ioPartialWord, oResults,
                         succ, iRow, iAnchor, iLimit - 1);
                ioPartialWord.accessCoord().setCol(ioPartialWord.getCoord().getCol() + 1);
                ioPartialWord.removeRight();
                iRack.add(l);
            }
            if (hasJokerInRack)
            {
                iRack.remove(Tile::Joker());
                ioPartialWord.addRightFromRack(l, true);
                ioPartialWord.accessCoord().setCol(ioPartialWord.getCoord().getCol() - 1);
                leftPart(iRack, ioPartialWord, oResults,
                         succ, iRow, iAnchor, iLimit - 1);
                ioPartialWord.accessCoord().setCol(ioPartialWord.getCoord().getCol() + 1);
                ioPartialWord.removeRight();
                iRack.add(Tile::Joker());
            }
        }
    }
}


void BoardSearch::extendRight(Rack &iRack, Round &ioPartialWord,
                              Results &oResults, unsigned int iNode,
                              int iRow, int iCol, int iAnchor) const
{
    if (m_tilesMx[iRow][iCol].isEmpty())
    {
        if (m_dic.isEndOfWord(iNode) && iCol > iAnchor)
        {
            evalMove(oResults, ioPartialWord);
        }

        // Optimization: avoid entering the for loop if no tile can match
        if (m_crossMx[iRow][iCol].isNone())
            return;

        bool hasJokerInRack = iRack.contains(Tile::Joker());
        for (unsigned int succ = m_dic.getSucc(iNode); succ; succ = m_dic.getNext(succ))
        {
            const Tile &l = Tile(m_dic.getChar(succ));
            if (m_crossMx[iRow][iCol].check(l))
            {
                if (iRack.contains(l))
                {
                    iRack.remove(l);
                    ioPartialWord.addRightFromRack(l, false);
                    extendRight(iRack, ioPartialWord, oResults,
                                succ, iRow, iCol + 1, iAnchor);
                    ioPartialWord.removeRight();
                    iRack.add(l);
                }
                if (hasJokerInRack)
                {
                    iRack.remove(Tile::Joker());
                    ioPartialWord.addRightFromRack(l, true);
                    extendRight(iRack, ioPartialWord, oResults,
                                succ, iRow, iCol + 1, iAnchor);
                    ioPartialWord.removeRight();
                    iRack.add(Tile::Joker());
                }
            }
        }
    }
    else
    {
        const Tile &l = m_tilesMx[iRow][iCol];
        wint_t upperChar = towupper(l.toChar());
        for (unsigned int succ = m_dic.getSucc(iNode); succ ; succ = m_dic.getNext(succ))
        {
            if ((wint_t)m_dic.getChar(succ) == upperChar)
            {
                ioPartialWord.addRightFromBoard(l);
                extendRight(iRack, ioPartialWord,
                            oResults, succ, iRow, iCol + 1, iAnchor);
                ioPartialWord.removeRight();
                // The letter will be present only once in the dictionary,
                // so we can stop looping
                break;
            }
        }
    }
}


/*
 * Computes the score of a word, coordinates may be changed to reflect
 * the real direction of the word
 */
void BoardSearch::evalMove(Results &oResults, Round &iWord) const
{
    int fromrack = 0;
    int pts      = 0;
    int ptscross = 0;
    int wordmul  = 1;

    unsigned int len = iWord.getWordLen();

    int row = iWord.getCoord().getRow();
    int col = iWord.getCoord().getCol();

    const BoardLayout & boardLayout = m_params.getBoardLayout();
    for (unsigned int i = 0; i < len; i++)
    {
        if (!m_tilesMx[row][col+i].isEmpty())
        {
            if (!m_jokerMx[row][col+i])
                pts += iWord.getTile(i).getPoints();
        }
        else
        {
            int l;
            if (!iWord.isJoker(i))
                l = iWord.getTile(i).getPoints() *
                    boardLayout.getLetterMultiplier(row, col + i);
            else
                l = 0;
            pts += l;
            int wm = boardLayout.getWordMultiplier(row, col + i);
            wordmul *= wm;

            int t = m_pointsMx[row][col+i];
            if (t >= 0)
                ptscross += (t + l) * wm;
            fromrack++;
        }
    }

    // Ignore words using too many letters from the rack
    if (fromrack > m_params.getLettersToPlay())
        return;

    pts = ptscross + pts * wordmul;
    if (fromrack == m_params.getLettersToPlay())
    {
        pts += m_params.getBonusPoints();
        iWord.setBonus(true);
    }
    iWord.setPoints(pts);

    if (iWord.getCoord().getDir() == Coord::VERTICAL)
    {
        // Exchange the coordinates temporarily
        iWord.accessCoord().swap();
    }
    oResults.add(iWord);
    if (iWord.getCoord().getDir() == Coord::VERTICAL)
    {
        // Restore the coordinates
        iWord.accessCoord().swap();
    }
}

