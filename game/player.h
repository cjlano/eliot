/*****************************************************************************
 * Copyright (C) 2004-2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: player.h,v 1.3 2005/02/09 22:33:56 ipkiss Exp $
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

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <vector>

class Playedrack;
class Round;
class Board;
typedef struct _Dictionary * Dictionary;


class Player
{
public:
    Player(bool iHuman);
    virtual ~Player();

    /**************************
     * General getters
     **************************/
    // FIXME: we should have a const getter!
    const PlayedRack & getCurrentRack() const;
    const PlayedRack & getLastRack() const;
    const Round & getLastRound() const;

    void setCurrentRack(const PlayedRack &iPld);

    /**************************
     * Add (or remove, if the given value is negative) points
     * to the player's score.
     **************************/
    void addPoints(int iPoints) { m_score += iPoints; }
    int getPoints() const       { return m_score; }

    /**************************
     *
     **************************/
    void endTurn(const Round &iRound, int iTurn);

    /**************************
     *
     **************************/
    typedef enum {PLAYED, TO_PLAY} play_status;
    void setStatus(play_status iStatus)     { m_status = iStatus; }
    play_status getStatus() const           { return m_status; }

    /**************************
     * AI (Artificial Intelligence) handling
     * The int argument of Player_ai_search() is the 'turn' number
     * (starting from 0)
     * Note: we could implement various strategies:
     *   - best: play the word with the best score (current implementation)
     *   - second: play the word with the second best score (strictly lower than
     *        the best one)
     *   - random: randomly choose one of the possible words
     *   - handicap(p): in the array of the n possible words (sorted by
     *        decreasing scores), play the word number i, where i/n is nearest
     *        from a predefined percentage p.
     *        So 'handicap(0)' should be equivalent to 'best'.
     *        This strategy makes an interesting opponent, because you can adapt
     *        it to your level, with a careful choice of the p value.
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
    virtual bool isHuman() const    { return m_human; }
    int aiSearch(const Dictionary &iDic, Board &iBoard, int turn);
    const Round & aiBestRound(); // XXX: useful?
    const Results & aiGetResults() const    { return m_results; }
    void clearResults() { m_results.clear(); }

private:
    // Is the player human or AI?
    bool m_human;
    // Score of the player
    int m_score;
    // Play status (XXX: only used by duplicate mode currently)
    play_status m_status;

    // History of the racks and rounds for the player
    vector<PlayedRack *> m_playedRacks;
    vector<Round *> m_rounds;
    vector<int> m_turns;

    // Results of a search with the current round
    Results m_results;
};

#endif

