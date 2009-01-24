/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2009 Antoine Fraboulet & Olivier Teulière
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

#ifndef _RESULTS_H_
#define _RESULTS_H_

#include <vector>
#include <map>
#include "round.h"

using namespace std;

class Dictionary;
class Board;
class Rack;


/**
 * This abstract class defines the interface to perform a search on the board
 * for a given rack, and it offers accessors to the resulting rounds.
 * Not all the rounds found by the search are necessarily kept, it depends
 * on the implementation (see below in the file for the various
 * implementations).
 *
 * After the search, the rounds are sorted by decreasing number of points,
 * then by alphabetical order (case insensitive), then by coordinates,
 * then by alphabetical order again (case sensitive this time).
 */
class Results
{
public:
    unsigned int size() const { return m_rounds.size(); }
    const Round & get(unsigned int) const;

    /**
     * Perform a search on the board. Every time a word is found,
     * the add() method will be called. At the end of the search,
     * results are sorted.
     */
    virtual void search(const Dictionary &iDic, const Board &iBoard,
                        const Rack &iRack, bool iFirstWord) = 0;

    /** Add a round */
    virtual void add(const Round &iRound) = 0;

    /** Clear the stored rounds, and get ready for a new search */
    virtual void clear() = 0;

protected:
    vector<Round> m_rounds;
    void sort();
};

/**
 * This implementation keeps only the rounds corresponding to the best score.
 * If there are several rounds with the same score, they are all kept.
 * All other rounds are ignored.
 */
class BestResults: public Results
{
public:
    BestResults();
    virtual void search(const Dictionary &iDic, const Board &iBoard,
                        const Rack &iRack, bool iFirstWord);
    virtual void clear();
    virtual void add(const Round &iRound);

private:
    int m_bestScore;
};

/**
 * This implementation finds the best score possible, and keeps only
 * the rounds whose score is closest to (but not lower than) the given
 * percentage of the best score.
 * All the rounds with this closest score are kept, rounds with a different
 * score are ignored.
 */
class PercentResults: public Results
{
public:
    /** The percentage is given as a float between 0 (0%) and 1 (100%) */
    PercentResults(float iPercent);
    virtual void search(const Dictionary &iDic, const Board &iBoard,
                        const Rack &iRack, bool iFirstWord);
    virtual void clear();
    virtual void add(const Round &iRound);

private:
    const float m_percent;
    int m_bestScore;
    int m_minScore;
};

/**
 * This implementation keeps the N best rounds, N being the given limit.
 * All other rounds are ignored.
 * In the special case where the limit is 0, all rounds are kept (but you can
 * expect the sorting of the rounds to be much slower...)
 */
class LimitResults: public Results
{
public:
    LimitResults(int iLimit);
    virtual void search(const Dictionary &iDic, const Board &iBoard,
                        const Rack &iRack, bool iFirstWord);
    virtual void clear();
    virtual void add(const Round &iRound);

    void setLimit(int iNewLimit) { m_limit = iNewLimit; }

private:
    int m_limit;
    map<int, int> m_scoresCount;
    int m_total;
    int m_minScore;
};

#endif

