/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
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
 *  \file   results.cc
 *  \brief  Search result storage class
 *  \author Olivier Teulière & Antoine Fraboulet
 *  \date   2005
 */

#include <algorithm>
#include <functional>

#include "tile.h"
#include "round.h"
#include "board.h"
#include "results.h"
#include "debug.h"


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
    ASSERT(0 <= i && i < size(), "Results index out of bounds");
    return m_rounds[i];
}


void Results::search(const Dictionary &iDic, Board &iBoard,
                     const Rack &iRack, int iTurn)
{
    clear();

    if (iTurn == 0)
    {
        iBoard.searchFirst(iDic, iRack, *this);
    }
    else
    {
        iBoard.search(iDic, iRack, *this);
    }

    sort_by_points();
}


void Results::sort_by_points()
{
    less_points lp;
    std::sort(m_rounds.begin(), m_rounds.end(), lp);
}

/****************************************************************/
/****************************************************************/

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
