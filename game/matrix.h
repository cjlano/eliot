/*****************************************************************************
 * Eliot
 * Copyright (C) 2003-2007 Olivier Teulière
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
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

#ifndef MATRIX_H_
#define MATRIX_H_

#include <vector>

using std::vector;


// Template matrix class for convenience.
template <class T>
class Matrix: public vector<vector<T> >
{
public:
    // Construct a matrix with an initial value
    Matrix(int iSize1, int iSize2, const T &iValue)
    {
        this->resize(iSize1, vector<T>(iSize2, iValue));
    }
    // Construct a square matrix with an initial value
    Matrix(int iSize, const T &iValue)
    {
        this->resize(iSize, vector<T>(iSize, iValue));
    }
};

#endif

