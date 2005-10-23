/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: results.h,v 1.5 2005/10/23 14:53:43 ipkiss Exp $
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

#ifndef _RESULTS_H_
#define _RESULTS_H_

#include <vector>
#include "round.h"

using namespace std;

class Board;
class Rack;
typedef struct _Dictionary * Dictionary;


/**
 * This class allows to perform a search on the board for a given rack,
 * and it offers accessors to the resulting rounds.
 * The rounds are sorted by decreasing number of points (but there is no
 * other ordering between 2 rounds with the same number of points).
 */
class Results
{
public:
    Results() {}
    virtual ~Results() {}

    int size() const    { return m_rounds.size(); }
    void clear()        { m_rounds.clear(); }
    const Round & get(int) const;

    // Perform a search on the board
    void search(const Dictionary &iDic, Board &iBoard,
                const Rack &iRack, int iTurn);

    // FIXME: These methods are used to fill the container with the rounds,
    // but they should not be part of the public interface
    void sort();
    void add(const Round &iRound)   { m_rounds.push_back(iRound); }

private:
    vector<Round> m_rounds;
};

#endif
