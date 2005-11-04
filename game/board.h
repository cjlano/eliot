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

#ifndef _BOARD_H_
#define _BOARD_H_

#include "tile.h"
#include "cross.h"
#include <string>
#include <vector>

typedef struct _Dictionary*Dictionary;
class Rack;
class Round;
class Results;

using namespace std;

#define BOARD_MIN 1
#define BOARD_MAX 15
#define BOARD_DIM 15
#define BOARD_REALDIM (BOARD_DIM + 2)


// Template matrix class for convenience.
template <class T>
class Matrix: public vector<vector<T> >
{
public:
    // Construct a matrix with an initial value
    Matrix(int iSize1, int iSize2, const T &iValue)
    {
        resize(iSize1, vector<T>(iSize2, iValue));
    }
    // Construct a square matrix with an initial value
    Matrix(int iSize, const T &iValue)
    {
        resize(iSize, vector<T>(iSize, iValue));
    }
};


class Board
{
public:
    Board();
    virtual ~Board() {}

    Tile getTile(int iRow, int iCol) const;
    bool isJoker(int iRow, int iCol) const;
    bool isVacant(int iRow, int iCol) const;
    /*int  score(Round);*/
    void addRound(const Dictionary &iDic, const Round &iRound);
    void removeRound(const Dictionary &iDic, const Round &iRound);
    int checkRound(Round &iRound, bool iFirstTurn);

    /*************************
     *
     *
     *************************/
    void testRound(const Round &iRound);
    void removeTestRound();
    char getTestChar(int iRow, int iCol) const;

    /*************************
     *
     * board_search.c
     *************************/
    void search(const Dictionary &iDic, const Rack &iRack, Results &oResults);
    void searchFirst(const Dictionary &iDic, const Rack &iRack, Results &oResults);

    /*************************
     *
     * board_cross.c
     *************************/
    void buildCross(const Dictionary &iDic);

    /*************************
     *
     *
     *************************/
    int getWordMultiplier(int iRow, int iCol) const;
    int getLetterMultiplier(int iRow, int iCol) const;

private:

    Matrix<Tile> m_tilesRow;
    Matrix<Tile> m_tilesCol;

    Matrix<bool> m_jokerRow;
    Matrix<bool> m_jokerCol;

    Matrix<Cross> m_crossRow;
    Matrix<Cross> m_crossCol;

    Matrix<int> m_pointRow;
    Matrix<int> m_pointCol;

    Matrix<char> m_testsRow;

    static const int m_tileMultipliers[BOARD_REALDIM][BOARD_REALDIM];
    static const int m_wordMultipliers[BOARD_REALDIM][BOARD_REALDIM];

    int checkRoundAux(Matrix<Tile> &iTilesMx,
                      Matrix<Cross> &iCrossMx,
                      Matrix<int> &iPointsMx,
                      Matrix<bool> &iJokerMx,
                      Round &iRound,
                      bool firstturn);
#ifdef DEBUG
    void checkDouble();
#endif

};

#endif
