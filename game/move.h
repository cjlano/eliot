/*****************************************************************************
 * Eliot
 * Copyright (C) 2007 Olivier Teulière
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

#ifndef _MOVE_H
#define _MOVE_H

#include <string>

#include "round.h"

using std::wstring;


/**
 * A Move is what a player can do during the game:
 *  - play a valid word
 *  - play an invalid or misplaced word
 *  - pass the turn (freegame only)
 *  - change letters (freegame only)
 *  - play nothing (timeout) (not supported yet)
 *
 * Moves are useful to record what happened, even if the board doesn't keep
 * a trace of the move (e.g.: an invalid move which was rejected will still
 * be remembered in the player history).
 *
 * Currently, moves are not used by the interfaces (they are only used
 * internally), but this could change in the future.
 */
class Move
{
    public:
        /**
         * Constructor taking a (valid) round
         */
        explicit Move(const Round &iRound);

        /**
         * Constructor taking a word and its coordinates, corresponding
         * to an invalid move by the player (invalid word, invalid coordinates,
         * letters not corresponding to the rack, ...)
         */
        explicit Move(const wstring &iWord, const wstring &iCoord);

        /**
         * Constructor taking letters to change.
         * An empty string means that the player simply passes without
         * changing any letter.
         * The given letters must have been already validated for correctness.
         */
        explicit Move(const wstring &iLetters);

        enum Type
        {
            VALID_ROUND,
            INVALID_WORD,
            PASS,
            CHANGE_LETTERS
        };

        /// Return the type of move
        Type getType() const { return m_type; }

        /// Get the score of this move (0 unless the round is valid)
        int getScore() const { return m_score; };

        /**
         * Return the round associated with the move, or throw an exception
         * if this move was not constructed from a valid round
         */
        const Round & getRound() const;

        /**
         * Return the word played at this move associated with the move, or
         * throw an exception if this move was not constructed from an invalid
         * pair (word, coord)
         */
        const wstring & getBadWord() const;

        /**
         * Return the coordinates of the (incorrect) word played at this move,
         * or throw an exception if this move was not constructed from an
         * invalid pair (cord, coord)
         */
        const wstring & getBadCoord() const;

        /**
         * Return the changed letters (possibly an empty string if the player
         * simply wanted to pass the turn), or throw an exception if this move
         * does not correspond to a passed turn
         */
        const wstring & getChangedLetters() const;

        /// To help debugging
        wstring toString() const;

    private:
        /// Type of move
        Type m_type;

        /// Score associated with this move
        int m_score;

        /// Round played at this turn
        Round m_round;

        /// Word played (incorrectly)
        wstring m_word;

        /// Coordinates of the word played (incorrectly)
        wstring m_coord;

        /// Changed letters (or empty string for passed turn)
        wstring m_letters;
};

#endif

