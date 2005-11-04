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

class Coord
{
public:

    enum Direction {VERTICAL, HORIZONTAL};

    Coord();
    Coord(const std::string& iCoord);
    virtual ~Coord();

    void setRow(int iRow);
    void setCol(int iCol);
    void setDir(Direction iDir);

    Direction getDir() const;
    int getRow() const;
    int getCol() const;

    void operator=(const Coord &iOther);
    std::string toString() const;

 private:
    Direction m_dir;
    int m_row, m_col;

};

#endif


/// Local Variables:
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
