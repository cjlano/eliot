/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2012 Olivier Teulière
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

#ifndef AI_PERCENT_H_
#define AI_PERCENT_H_

#include "ai_player.h"
#include "results.h"
#include "logging.h"

/**
 * This kind of AI is parameterized by a percentage p.
 * The computation consists in finding all the N possible rounds for the
 * current rack/board, and sorting the list.
 * The chosen round is the one with the smallest score at least equal to
 * p * best_score.
 * A percentage of 1 should always return the best round (i.e. the one with
 * the highest score), while a percentage of 0 should return the worst one.
 * This kind of AI will never change letters (unless it cannot play anything,
 * in which case it just passes without changing letters).
 */
class AIPercent: public AIPlayer
{
    DEFINE_LOGGER();
public:
    /// Constructor, taking the percentage (0.0 <= iPercent <= 1.0)
    AIPercent(float iPercent);
    virtual ~AIPercent();

    float getPercent() const { return m_percent; }

    /**
     * This method does the actual computation. It will be called before any
     * of the following methods, so it must prepare everything for them.
     */
    virtual void compute(const Dictionary &iDic, const Board &iBoard, bool iFirstWord);

    /// Return the move played by the AI
    virtual Move getMove() const;

private:
    float m_percent;
    /// Container for all the found solutions
    Results *m_results;
};

#endif

