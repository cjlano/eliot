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

#include <boost/foreach.hpp>
#include <algorithm>
#include <functional>
#include <cwctype>
#include <cmath>

#include "tile.h"
#include "round.h"
#include "board.h"
#include "results.h"
#include "debug.h"


bool tileCompare(const Tile &t1, const Tile &t2)
{
    return t1.toCode() < t2.toCode();
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
            // If the scores are equal, sort alphabetically (i.e. in the
            // order of the letters in the dictionary), ignoring the case
            const wstring &s1 = r1.getWord();
            const wstring &s2 = r2.getWord();
            if (std::lexicographical_compare(r1.getTiles().begin(),
                                             r1.getTiles().end(),
                                             r2.getTiles().begin(),
                                             r2.getTiles().end(),
                                             tileCompare))
            {
                return true;;
            }
            else if (std::lexicographical_compare(r2.getTiles().begin(),
                                                  r2.getTiles().end(),
                                                  r1.getTiles().begin(),
                                                  r1.getTiles().end(),
                                                  tileCompare))
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


void Results::sort()
{
    less_points lp;
    std::sort(m_rounds.begin(), m_rounds.end(), lp);
}


BestResults::BestResults()
    : m_bestScore(0)
{
}


void BestResults::search(const Dictionary &iDic, const Board &iBoard,
                         const Rack &iRack, bool iFirstWord)
{
    clear();

    if (iFirstWord)
        iBoard.searchFirst(iDic, iRack, *this);
    else
        iBoard.search(iDic, iRack, *this);

    sort();
}


void BestResults::add(const Round &iRound)
{
    // Ignore too low scores
    if (m_bestScore > iRound.getPoints())
        return;

    if (m_bestScore < iRound.getPoints())
    {
        // New best score: clear the stored results
        m_bestScore = iRound.getPoints();
        m_rounds.clear();
    }
    m_rounds.push_back(iRound);
}


void BestResults::clear()
{
    m_rounds.clear();
    m_bestScore = 0;
}



PercentResults::PercentResults(float iPercent)
    : m_percent(iPercent)
{
}

class Predicate
{
public:
    Predicate(int iPoints) : m_chosenPoints(iPoints) {}
    bool operator()(const Round &iRound) const
    {
        return iRound.getPoints() != m_chosenPoints;
    }

private:
    const int m_chosenPoints;
};


void PercentResults::search(const Dictionary &iDic, const Board &iBoard,
                            const Rack &iRack, bool iFirstWord)
{
    clear();

    if (iFirstWord)
        iBoard.searchFirst(iDic, iRack, *this);
    else
        iBoard.search(iDic, iRack, *this);

    if (m_rounds.empty())
        return;

    // At this point, add() has been called, so the best score is valid

    // Find the lowest score at least equal to the min_score
    int chosenPoints = m_bestScore;
    BOOST_FOREACH(const Round &iRound, m_rounds)
    {
        int points = iRound.getPoints();
        if (points >= m_minScore && points < chosenPoints)
        {
            chosenPoints = points;
        }
    }

    // Keep only the rounds with the "chosenPoints" score
    vector<Round>::iterator last =
        std::remove_if(m_rounds.begin(), m_rounds.end(), Predicate(chosenPoints));
    m_rounds.erase(last, m_rounds.end());
    ASSERT(!m_rounds.empty(), "Bug in PercentResults");

    // Sort the remaining rounds
    sort();
}


void PercentResults::add(const Round &iRound)
{
    // Ignore too low scores
    if (m_minScore > iRound.getPoints())
        return;

    if (m_bestScore < iRound.getPoints())
    {
        m_bestScore = iRound.getPoints();
        m_minScore = (int)ceil(m_bestScore * m_percent);
    }
    m_rounds.push_back(iRound);
}


void PercentResults::clear()
{
    m_rounds.clear();
    m_bestScore = 0;
    m_minScore = 0;
}



LimitResults::LimitResults(int iLimit)
    : m_limit(iLimit), m_total(0), m_minScore(-1)
{
}


void LimitResults::search(const Dictionary &iDic, const Board &iBoard,
                          const Rack &iRack, bool iFirstWord)
{
    clear();

    if (iFirstWord)
        iBoard.searchFirst(iDic, iRack, *this);
    else
        iBoard.search(iDic, iRack, *this);

    if (m_rounds.empty())
        return;

    // Sort the rounds
    sort();

    // Truncate the results to respect the limit
    if (m_limit != 0 && m_rounds.size() > (unsigned int) m_limit)
        m_rounds.resize(m_limit);
}


void LimitResults::add(const Round &iRound)
{
    // If we ignore the limit, simply add the round
    if (m_limit == 0)
    {
        m_rounds.push_back(iRound);
        return;
    }

    // Ignore too low scores
    if (m_minScore >= iRound.getPoints())
        return;

    // Add the round
    m_rounds.push_back(iRound);
    ++m_total;
    ++m_scoresCount[iRound.getPoints()];

    // Can we increase the minimum score required?
    if (m_total - m_scoresCount[m_minScore] >= m_limit)
    {
        // Yes! "Forget" the rounds of score m_minScore
        // They are still present in m_rounds, but they will be removed
        // for real later in the search() method
        m_total -= m_scoresCount[m_minScore];
        m_scoresCount.erase(m_minScore);

        // Find the new min score
        map<int, int>::const_iterator it =
            m_scoresCount.lower_bound(m_minScore);
        ASSERT(it != m_scoresCount.end(), "Bug in LimitResults::add())");
        m_minScore = it->first;
    }
}


void LimitResults::clear()
{
    m_rounds.clear();
    m_scoresCount.clear();
    m_minScore = -1;
    m_total = 0;
}

