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
 *  \file   coord.h
 *  \brief  Game coordinates system
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#ifndef _COORD_H
#define _COORD_H

using std::string;

class Coord
{
public:

    enum Direction {VERTICAL, HORIZONTAL};

    // Construction, destruction
    Coord(int iRow = -1, int iCol = -1, Direction iDir = HORIZONTAL);
    Coord(const string &iStr);
    virtual ~Coord() {}

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

    void setFromString(const string &iStr);
    string toString() const;

private:
    Direction m_dir;
    int m_row, m_col;

};

#endif


/// Local Variables:
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
