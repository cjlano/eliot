/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: round.h,v 1.4 2005/03/27 17:30:48 ipkiss Exp $
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/

#ifndef _ROUND_H_
#define _ROUND_H_

#include <vector>
#include "tile.h"

using namespace std;

enum Tdirection {VERTICAL, HORIZONTAL};
typedef enum Tdirection Direction;


/**
 * A Round is the representation of a played word (or going to be played).
 * It contains the word itself, of course, but also information of position
 * on the board, origin of letters (board for a letter already placed, rack
 * for a letter just being played), points, etc...
 */
class Round
{
public:

    /*************************
     * 
     *************************/
    Round();
    virtual ~Round() {}
    void init();

    /*************************
     * 
     *************************/
    void addRightFromBoard(Tile);
    void removeRightToBoard(Tile);
    void addRightFromRack(Tile, bool);
    void removeRightToRack(Tile, bool);

    /*************************
     * General setters
     *************************/
    void setRow(int iRow)          { m_row = iRow; }
    void setCol(int iCol)          { m_col = iCol; }
    void setPoints(int iPoints)    { m_points = iPoints; }
    void setDir(Direction iDir)    { m_dir = iDir; }
    void setBonus(bool iBonus)     { m_bonus = iBonus; }
    void setWord(const vector<Tile> &iTiles);
    void setFromRack(int iIndex);
    void setFromBoard(int iIndex);
    void setJoker(int iIndex);

    /*************************
     * 
     *************************/
    bool isJoker(int iIndex) const;
    const Tile& getTile(int iIndex) const;
    int getWordLen() const;
    bool isPlayedFromRack(int iIndex) const;

    /*************************
     * General getters
     *************************/
    int getRow() const          { return m_row; }
    int getCol() const          { return m_col; }
    int getPoints() const       { return m_points; }
    int getBonus() const        { return m_bonus; }
    Direction getDir() const    { return m_dir; }

private:
    vector<Tile> m_word;
    vector<char> m_tileOrigin;
    Direction m_dir;
    int m_row, m_col;
    int m_points;
    int m_bonus;
};

#endif
