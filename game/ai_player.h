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

#ifndef _AI_PLAYER_H_
#define _AI_PLAYER_H_

#include "player.h"

class Dictionary;
class Round;
class Board;
class Tile;

/**
 * This class is a pure interface, that must be implemented by all the AI
 * (Artificial Intelligence) players
 *
 * Note: we could implement various strategies (some of which are already
 * implemented):
 *   - best: play the word with the best score (current default implementation)
 *   - second: play the word with the second best score (strictly lower than
 *        the best one)
 *   - random: randomly choose one of the possible words
 *   - handicap(p): in the array of the n possible words (sorted by
 *        decreasing scores), play the word number i, where i/n is nearest
 *        from a predefined percentage p.
 *        So 'handicap(0)' should be equivalent to 'best'.
 *        This strategy should make an interesting opponent, because you can
 *        adapt it to your level, with a careful choice of the p value.
 *
 * In fact, instead of working on the score of the words, these strategies
 * could work on any other value. In particular, some heuristics could
 * modulate the score with a value indicating the openings offered by the
 * word (if a word makes accessible a "word counts triple" square, it is
 * less interesting than another word with the same score or even with a
 * slightly lower score, but which does not offer such a square).
 *
 * More evolved heuristics could even take into account the remaining
 * letters in the bag to guess the 'statistical rack' of the opponent, and
 * play a word both maximizing the score and minimizing the opponent's
 * score...
 * Hmmm... i don't think this one will be implemented in a near future :)
 **************************/
class AIPlayer: public Player
{
public:
    virtual ~AIPlayer() {}

    /// No human here. Trespassers will be shot!
    virtual bool isHuman() const { return false; }

    /**
     * This method does the actual computation. It will be called before any
     * of the following methods, so it must prepare everything for them.
     */
    virtual void compute(const Dictionary &iDic, Board &iBoard, bool iFirstWord) = 0;

    /// Return the move played by the AI
    virtual Move getMove() const = 0;

protected:
    /// This class is a pure interface, forbid any direct instanciation
    AIPlayer(unsigned int iId): Player(iId) {}
};

#endif

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
