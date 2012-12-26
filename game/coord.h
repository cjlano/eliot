/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2012 Antoine Fraboulet & Olivier Teulière
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

#ifndef COORD_H_
#define COORD_H_

#include <string>

#include "logging.h"

using std::wstring;


/**
 * This class handles coordinates of a square on the board.
 * The row and column start at 1.
 */
class Coord
{
    DEFINE_LOGGER();
public:

    enum Direction {VERTICAL, HORIZONTAL};

    // Construction
    Coord(int iRow = -1, int iCol = -1, Direction iDir = HORIZONTAL);
    Coord(const wstring &iStr);

    // Accessors
    void setRow(int iRow)       { m_row = iRow; }
    void setCol(int iCol)       { m_col = iCol; }
    void setDir(Direction iDir) { m_dir = iDir; }
    int getRow() const          { return m_row; }
    int getCol() const          { return m_col; }
    Direction getDir() const    { return m_dir; }

    bool isValid() const;
    bool operator==(const Coord &iOther) const;

    // Swap the coordinates (without changing the direction)
    void swap();


    void setFromString(const wstring &iStr);
    wstring toString() const;

private:
    Direction m_dir;
    int m_row, m_col;

};

#endif

