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
 *  \brief  Board coordinate system
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#include <string>
#include "coord.h"
#include "board.h" // for BOARD_MIN and BOARD_MAX (TODO: remove this include)
#include "debug.h"


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

string Coord::toString(coord_mode_t mode) const
{
    ASSERT(isValid(), "Invalid coordinates");

    char res[7];
    char srow[3];
    char scol[3];

    sprintf(scol, "%d", m_col);
    sprintf(srow, "%c", m_row + 'A' - 1);

    switch (mode)
    {
    case COORD_MODE_COMPACT:
	if (getDir() == HORIZONTAL)
	    sprintf(res,"%s%s",srow,scol);
	else
	    sprintf(res,"%s%s",scol,srow);
	break;
    case COORD_MODE_LONG:
	if (getDir() == HORIZONTAL)
	    sprintf(res,"%2s %2s",srow,scol);
	else
	    sprintf(res,"%2s %2s",scol,srow);
	break;
    }

    return string(res);
}

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
