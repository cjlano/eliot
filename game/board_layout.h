/*****************************************************************************
 * Eliot
 * Copyright (C) 2012 Olivier Teulière
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

#ifndef BOARD_LAYOUT_H_
#define BOARD_LAYOUT_H_

#include "matrix.h"
#include "logging.h"


/**
 * Board layout (size and special squares)
 */
class BoardLayout
{
    DEFINE_LOGGER();
public:
    BoardLayout();

    unsigned getRowCount() const;
    unsigned getColCount() const;
    int getWordMultiplier(unsigned iRow, unsigned iCol) const;
    int getLetterMultiplier(unsigned iRow, unsigned iCol) const;

    static const BoardLayout & GetDefault();

private:

    static BoardLayout m_defaultLayout;

    Matrix<int> m_wordMultipliers;
    Matrix<int> m_tileMultipliers;

    bool isValidCoord(unsigned iRow, unsigned iCol) const;

    void setDefaultLayout();

};

#endif

