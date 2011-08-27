/*****************************************************************************
 * Eliot
 * Copyright (C) 2009 Olivier Teulière
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

#ifndef BOARD_SEARCH_H_
#define BOARD_SEARCH_H_

#include "coord.h"
#include "matrix.h"

class Dictionary;
class GameParams;
class Tile;
class Rack;
class Results;
class Round;
class Cross;


class BoardSearch
{
public:
    BoardSearch(const Dictionary &iDic,
                const GameParams &iParams,
                const Matrix<Tile> &iTilesMx,
                const Matrix<Cross> &iCrossMx,
                const Matrix<int> &iPointsMx,
                const Matrix<bool> &iJokerMx,
                bool isFirstTurn = false);

    void search(Rack &iRack, Results &oResults, Coord::Direction iDir) const;

private:
    const Dictionary &m_dic;
    const GameParams &m_params;
    const Matrix<Tile> &m_tilesMx;
    const Matrix<Cross> &m_crossMx;
    const Matrix<int> &m_pointsMx;
    const Matrix<bool> &m_jokerMx;
    const bool m_firstTurn;

    void leftPart(Rack &iRack, Round &ioPartialWord,
                  Results &oResults, int n, int iRow,
                  int iAnchor, int iLimit) const;

    void extendRight(Rack &iRack, Round &ioPartialWord,
                     Results &oResults, unsigned int iNode,
                     int iRow, int iCol, int iAnchor) const;

    void evalMove(Results &oResults, Round &iWord) const;
};

#endif


