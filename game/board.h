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

#ifndef BOARD_H_
#define BOARD_H_

#include <string>

#include "matrix.h"
#include "tile.h"
#include "cross.h"
#include "logging.h"

class GameParams;
class Dictionary;
class Rack;
class Round;
class Results;

using namespace std;

#define BOARD_MIN 1
#define BOARD_MAX 15
#define BOARD_DIM 15
#define BOARD_REALDIM (BOARD_DIM + 2)


/**
 * Representation of the board.
 *
 * In all the methods, the given coordinates
 * have to be BOARD_MIN <= int <= BOARD_MAX.
 */
class Board
{
    DEFINE_LOGGER();
public:
    Board(const GameParams &iParams);

    bool isJoker(int iRow, int iCol) const;
    bool isVacant(int iRow, int iCol) const;

    const Tile& getTile(int iRow, int iCol) const;
    wstring getDisplayStr(int iRow, int iCol) const;

    void addRound(const Dictionary &iDic, const Round &iRound);
    void removeRound(const Dictionary &iDic, const Round &iRound);
    int  checkRound(Round &iRound, bool checkJunction = true) const;

    /**
     * Preview
     */
    bool isTestChar(int iRow, int iCol) const;
    const Tile& getTestTile(int iRow, int iCol) const;
    void testRound(const Round &iRound);
    void removeTestRound();

    void search(const Dictionary &iDic, const Rack &iRack, Results &oResults) const;
    void searchFirst(const Dictionary &iDic, const Rack &iRack, Results &oResults) const;

    /**
     *
     */
    static int GetWordMultiplier(int iRow, int iCol);
    static int GetLetterMultiplier(int iRow, int iCol);

    /**
     * 
     */
    string getCellContent_row(int row, int col) const;
    string getCellContent_col(int row, int col) const;

private:

    const GameParams &m_params;

    Matrix<Tile> m_tilesRow;
    Matrix<Tile> m_tilesCol;

    Matrix<bool> m_jokerRow;
    Matrix<bool> m_jokerCol;

    Matrix<Cross> m_crossRow;
    Matrix<Cross> m_crossCol;

    Matrix<int> m_pointRow;
    Matrix<int> m_pointCol;

    Matrix<Tile> m_testsRow;

    /// Flag indicating if the board is empty or if it has letters
    bool m_isEmpty;

    static const int m_tileMultipliers[BOARD_REALDIM][BOARD_REALDIM];
    static const int m_wordMultipliers[BOARD_REALDIM][BOARD_REALDIM];

    /**
     * board_cross.c
     */
    void buildCross(const Dictionary &iDic);

    int checkRoundAux(const Matrix<Tile> &iTilesMx,
                      const Matrix<Cross> &iCrossMx,
                      const Matrix<int> &iPointsMx,
                      const Matrix<bool> &iJokerMx,
                      Round &iRound,
                      bool checkJunction) const;
#ifdef DEBUG
    void checkDouble();
#endif

};

#endif

