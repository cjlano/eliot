/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
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

#ifndef _AI_PERCENT_H_
#define _AI_PERCENT_H_

#include "ai_player.h"
#include "results.h"

/**
 * This kind of AI is parameterized by a percentage p.
 * The computation consists in finding all the N possible rounds for the
 * current rack/board, and sorting the list.
 * The chosen round is the n'th element of the sorted list, such that n/N
 * is closest to the percentage p.
 * A percentage of 0 should always return the best round (i.e. the one with
 * the highest score), while a percentage of 1 should return the worst one.
 * This kind of AI will never change letters (unless it cannot play anything,
 * in which case it just passes without changing letters).
 */
class AIPercent: public AIPlayer
{
public:
    /// Constructor, taking the percentage (0.0 <= iPercent <= 1.0)
    AIPercent(float iPercent);
    virtual ~AIPercent() {}

    /**
     * This method does the actual computation. It will be called before any
     * of the following methods, so it must prepare everything for them.
     */
    virtual void compute(const Dictionary &iDic, Board &iBoard, int turn);
    /// Return true when the AI wants to change letters instead of playing a word
    virtual bool changesLetters() const;
    /// Return the round played by the AI (if changesLetters() returns false)
    virtual const Round & getChosenRound() const;
    /// Get the letters to change (if changesLetters() returns true)
    virtual vector<Tile> getChangedLetters() const;

private:
    /// Percentage used for this player
    float m_percent;
    /// Container for all the found solutions
    Results m_results;
};

#endif

