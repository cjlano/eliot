/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: ai_percent.cpp,v 1.1 2005/02/17 20:01:59 ipkiss Exp $
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

#include "tile.h"
#include "rack.h"
#include "pldrack.h"
#include "round.h"
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


void AIPercent::compute(const Dictionary &iDic, Board &iBoard, int turn)
{
    m_results.clear();

    Rack rack;
    getCurrentRack().getRack(rack);
    m_results.search(iDic, iBoard, rack, turn);
}


bool AIPercent::changesLetters() const
{
    return (m_results.size() == 0);
}


const Round & AIPercent::getChosenRound() const
{
    int index = (int)(m_percent * (m_results.size() - 1));
    return m_results.get(index);
}


vector<Tile> AIPercent::getChangedLetters() const
{
    return vector<Tile>();
}

