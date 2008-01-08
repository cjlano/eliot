/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2007 Antoine Fraboulet & Olivier Teulière
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

/**
 *  \file   coord.h
 *  \brief  Board coordinates system
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#ifndef _COORD_H
#define _COORD_H

#include <string>

using std::wstring;


/**
 * This class handles coordinates of a square on the board.
 * The row and column start at 1.
 */
class Coord
{
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
    void operator=(const Coord &iOther);

    // Swap the coordinates (without changing the direction)
    void swap();


    enum coord_mode_t
    {
        COORD_MODE_COMPACT,
        COORD_MODE_LONG
    };
    void setFromString(const wstring &iStr);
    wstring toString(coord_mode_t mode = COORD_MODE_COMPACT) const;

private:
    Direction m_dir;
    int m_row, m_col;

};

#endif

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
