/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: results.cpp,v 1.3 2005/02/17 20:01:59 ipkiss Exp $
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/

#include <algorithm>
#include <functional>

#include "tile.h"
#include "round.h"
#include "board.h"
#include "results.h"


struct less_points : public binary_function<const Round&,
                     const Round&, bool>
{
    bool operator()(const Round &r1, const Round &r2)
    {
        // We want higher scores first, so we use '>' instead of '<'
        return r1.getPoints() > r2.getPoints();
    }
};


const Round & Results::get(int i) const
{
    // Use at() instead of [] to check bounds and throw an exception
    return m_rounds.at(i);
}


void Results::search(const Dictionary &iDic, Board &iBoard,
                     const Rack &iRack, int iTurn)
{
    clear();

    if (iTurn == 0)
        iBoard.searchFirst(iDic, iRack, *this);
    else
        iBoard.search(iDic, iRack, *this);
}


void Results::sort()
{
    less_points lp;
    std::sort(m_rounds.begin(), m_rounds.end(), lp);
}


