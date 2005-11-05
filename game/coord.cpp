/* Eliot                                                                     */
/* Copyright (C) 1999  Antoine Fraboulet                                     */
/*                                                                           */
/* This file is part of Eliot.                                               */
/*                                                                           */
/* Eliot is free software; you can redistribute it and/or modify             */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* Eliot is distributed in the hope that it will be useful,                  */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

/**
 *  \file   coord.cpp
 *  \brief  Eliot coordinate system
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#include <string>
#include "coord.h"
#include "board.h" // for BOARD_MIN and BOARD_MAX (TODO: remove this include)


Coord::Coord(int iRow, int iCol, Direction iDir)
{
    m_row = iRow;
    m_col = iCol;
    m_dir = iDir;
}

Coord::Coord(const string &iStr)
{
    setFromString(iStr);
}

bool Coord::isValid() const
{
    return (m_row >= BOARD_MIN && m_row <= BOARD_MAX &&
            m_col >= BOARD_MIN && m_col <= BOARD_MAX);
}

void Coord::operator=(const Coord &iOther)
{
    m_dir = iOther.m_dir;
    m_row = iOther.m_row;
    m_col = iOther.m_col;
}

void Coord::swap()
{
    int tmp = m_col;
    m_col = m_row;
    m_row = tmp;
}

void Coord::setFromString(const string &iStr)
{
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
}

string Coord::toString() const
{
    string rs;

    char s[5];
    sprintf(s, "%d", m_col);
    if (getDir() == HORIZONTAL)
    {
        rs = string(1, m_row + 'A' - 1) + s;
    }
    else
    {
        rs = s + string(1, m_row + 'A' - 1);
    }
    return rs;
}


/// Local Variables:
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
