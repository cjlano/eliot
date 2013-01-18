/*****************************************************************************
 * Eliot
 * Copyright (C) 2013 Olivier Teulière
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

#include "move_selector.h"
#include "round.h"
#include "results.h"
#include "bag.h"

#include "dic.h"
#include "debug.h"


INIT_LOGGER(game, MoveSelector);

#define PLAYED_JOKER (-1000)
#define EXTENSION_1 50


MoveSelector::MoveSelector(const Bag &iBag, const Dictionary &iDic)
    : m_bag(iBag), m_dic(iDic)
{
}


Round MoveSelector::selectMaster(const BestResults &iResults) const
{
    ASSERT(!iResults.isEmpty(), "Nothing to select from");

    // Easy case
    if (iResults.size() == 1)
        return iResults.get(0);

    // Compare the rounds. The one with the highest score wins.
    // In case of equal scores, the first one wins.
    int bestIndex = 0;
    int bestScore = evalScore(iResults.get(0));
    for (unsigned num = 1; num < iResults.size(); ++num)
    {
        int score = evalScore(iResults.get(num));
        if (bestScore < score)
        {
            bestScore = score;
            bestIndex = num;
        }
    }

    return iResults.get(bestIndex);
}


int MoveSelector::evalScore(const Round &iRound) const
{
    int score = 0;
    score += evalForJokersInRack(iRound);
    // Deactivated for now, as it breaks a few non-regression tests,
    // and I don't have time to fix them at the moment... :)
#if 0
    score += evalForExtensions(iRound);
#endif
    // TODO: add more heuristics
    return score;
}


int MoveSelector::evalForJokersInRack(const Round &iRound) const
{
    return iRound.countJokersFromRack() * PLAYED_JOKER;
}


int MoveSelector::evalForExtensions(const Round &iRound) const
{
    // Find front and back extensions to the given round
    const wstring &roundWord = iRound.getWord();
    vector<wdstring> results;
    m_dic.searchRacc(roundWord, results);

    // Give a bonus for each extension
    // TODO: it would be better to give a bonus only for extensions
    // corresponding to letters still in the bag...
    return results.size() * EXTENSION_1;
}


