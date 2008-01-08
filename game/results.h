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
 *  \file   results.h
 *  \brief  Search result storage class
 *  \author Olivier Teulière & Antoine Fraboulet
 *  \date   2005
 */

#ifndef _RESULTS_H_
#define _RESULTS_H_

#include <vector>
#include "round.h"

using namespace std;

class Dictionary;
class Board;
class Rack;


/**
 * This class allows to perform a search on the board for a given rack,
 * and it offers accessors to the resulting rounds.
 * The rounds are sorted by decreasing number of points (but there is no
 * other ordering between 2 rounds with the same number of points).
 */
class Results
{
public:
    unsigned int size() const    { return m_rounds.size(); }
    void clear()        { m_rounds.clear(); }
    const Round & get(unsigned int) const;

    /// Perform a search on the board
    void search(const Dictionary &iDic, Board &iBoard,
                const Rack &iRack, bool iFirstWord);

    // FIXME: This method is used to fill the container with the rounds,
    // but it should not be part of the public interface
    void add(const Round &iRound)   { m_rounds.push_back(iRound); }

private:
    vector<Round> m_rounds;

    void sortByPoints();
};

#endif

/****************************************************************/
/****************************************************************/

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
