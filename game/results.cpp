/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2007 Antoine Fraboulet & Olivier Teulière
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

/**
 *  \file   results.cc
 *  \brief  Search result storage class
 *  \author Olivier Teulière & Antoine Fraboulet
 *  \date   2005
 */

#include <algorithm>
#include <functional>
#include <cwctype>

#include "tile.h"
#include "round.h"
#include "board.h"
#include "results.h"
#include "debug.h"


bool wcharCompare(wchar_t c1, wchar_t c2)
{
    return towlower(c1) < towlower(c2);
}

struct less_points : public binary_function<const Round&, const Round&, bool>
{
    bool operator()(const Round &r1, const Round &r2)
    {
        // We want higher scores first, so we use '>' instead of '<'
        if (r1.getPoints() > r2.getPoints())
            return true;
        else if (r1.getPoints() < r2.getPoints())
            return false;
        else
        {
            // If the scores are equal, sort alphabetically, ignoring
            // the case
            const wstring &s1 = r1.getWord();
            const wstring &s2 = r2.getWord();
            if (std::lexicographical_compare(s1.begin(),
                                             s1.end(),
                                             s2.begin(),
                                             s2.end(),
                                             wcharCompare))
            {
                return true;;
            }
            else if (std::lexicographical_compare(s2.begin(),
                                                  s2.end(),
                                                  s1.begin(),
                                                  s1.end(),
                                                  wcharCompare))
            {
                return false;
            }
            else
            {
                // If the rounds are still equal, compare the coordinates
                const wstring &c1 = r1.getCoord().toString();
                const wstring &c2 = r2.getCoord().toString();
                if (c1 < c2)
                    return true;
                else if (c2 < c1)
                    return false;
                else
                {
                    // Still equal? This time compare taking the case into
                    // account. After that, we are sure that the rounds will
                    // be different...
                    return std::lexicographical_compare(s1.begin(),
                                                        s1.end(),
                                                        s2.begin(),
                                                        s2.end());
                }
            }
        }
    }
};


const Round & Results::get(unsigned int i) const
{
    ASSERT(i < size(), "Results index out of bounds");
    return m_rounds[i];
}


void Results::search(const Dictionary &iDic, Board &iBoard,
                     const Rack &iRack, bool iFirstWord)
{
    clear();

    if (iFirstWord)
    {
        iBoard.searchFirst(iDic, iRack, *this);
    }
    else
    {
        iBoard.search(iDic, iRack, *this);
    }

    sortByPoints();
}


void Results::sortByPoints()
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
