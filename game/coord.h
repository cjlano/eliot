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
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/* $Id: coord.h,v 1.1 2005/08/15 18:14:52 afrab Exp $ */

/**
 *  \file   coord.h
 *  \brief  Game coordinates system
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#ifndef _COORD_H
#define _COORD_H

enum Tdirection {VERTICAL, HORIZONTAL};
typedef enum Tdirection Direction;

class Coord
{
 public:

    Coord();
    ~Coord();
    
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
