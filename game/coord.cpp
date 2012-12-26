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

#include <string>
#include <cstdio>
#include <wchar.h>
#include "coord.h"
#include "board.h" // for BOARD_MIN and BOARD_MAX (TODO: remove this include)
#include "debug.h"
#include "encoding.h"


INIT_LOGGER(game, Coord);


Coord::Coord(int iRow, int iCol, Direction iDir)
{
    m_row = iRow;
    m_col = iCol;
    m_dir = iDir;
}

Coord::Coord(const wstring &iStr)
{
    setFromString(iStr);
}

bool Coord::isValid() const
{
    return (m_row >= BOARD_MIN && m_row <= BOARD_MAX &&
            m_col >= BOARD_MIN && m_col <= BOARD_MAX);
}


bool Coord::operator==(const Coord &iOther) const
{
    if (!isValid() && !iOther.isValid())
        return true;
    return m_row == iOther.m_row
        && m_col == iOther.m_col
        && m_dir == iOther.m_dir;
}


void Coord::swap()
{
    int tmp = m_col;
    m_col = m_row;
    m_row = tmp;
}


void Coord::setFromString(const wstring &iWStr)
{
    // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
    // Temporary implementation: convert the wchar_t* string into a char* one
    string iStr = lfw(iWStr);

    char l[4];
    int col;

    if (sscanf(iStr.c_str(), "%1[a-oA-O]%2d", l, &col) == 2)
    {
        setDir(HORIZONTAL);
    }
    else if (sscanf(iStr.c_str(), "%2d%1[a-oA-O]", &col, l) == 2)
    {
        setDir(VERTICAL);
    }
    else
    {
        col = -1;
        l[0] = 'A' - 1;
    }
    int row = toupper(*l) - 'A' + 1;
    setCol(col);
    setRow(row);
    // Input such as "A12foo#bar&stuff" should be invalid
    if (isValid() && toString() != iWStr)
        setCol(-1);
}

wstring Coord::toString() const
{
    ASSERT(isValid(), "Invalid coordinates");

    wchar_t res[7];
    wchar_t srow[3];
    wchar_t scol[3];

    _swprintf(scol, 3, L"%d", m_col);
    _swprintf(srow, 3, L"%c", m_row + 'A' - 1);

    if (getDir() == HORIZONTAL)
        _swprintf(res, 7, L"%ls%ls", srow, scol);
    else
        _swprintf(res, 7, L"%ls%ls", scol, srow);

    return res;
}

