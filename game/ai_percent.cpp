/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2007 Olivier Teulière
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

#include "tile.h"
#include "rack.h"
#include "pldrack.h"
#include "round.h"
#include "move.h"
#include "results.h"
#include "board.h"
#include "ai_percent.h"


AIPercent::AIPercent(float iPercent)
    : m_percent(iPercent)
{
    // Ensure the decimal value of the percentage is between 0 and 1
    if (m_percent < 0)
        m_percent = 0;
    if (m_percent > 1)
        m_percent = 1;
}


void AIPercent::compute(const Dictionary &iDic, const Board &iBoard, bool iFirstWord)
{
    m_results.clear();

    Rack rack;
    getCurrentRack().getRack(rack);
    m_results.search(iDic, iBoard, rack, iFirstWord);
}


Move AIPercent::getMove() const
{
    if (m_results.size() == 0)
    {
        // If there is no result, change all the letters
        // XXX: it is forbidden in duplicate mode (even passing is forbidden),
        // but well, what else to do?
        Rack rack;
        getCurrentRack().getRack(rack);
        return Move(rack.toString());
    }
    else
    {
        // If there are results, apply the algorithm
        double wantedScore = m_percent * m_results.get(0).getPoints();
        // Look for the first round giving at least 'wantedScore' points
        // Browse the results 10 by 10 (a dichotomy would be better, but this
        // is not performance critical)
        unsigned int index = 0;
        while (index < m_results.size() &&
               m_results.get(index).getPoints() > wantedScore)
        {
            index += 10;
        }
        // Now the wanted round is in the last 10 indices
        if (index >= m_results.size())
            index = m_results.size() - 1;
        while (m_results.get(index).getPoints() < wantedScore)
        {
            --index;
        }
        return Move(m_results.get(index));
    }
}

