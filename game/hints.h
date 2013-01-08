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

#ifndef HINTS_H_
#define HINTS_H_

#include <string>

#include "logging.h"


using std::string;

class Move;


/**
 * Interface common to all the hints.
 */
class AbstractHint
{
    DEFINE_LOGGER();
public:
    AbstractHint(const string &iName, const string &iDescription, int iCost);
    virtual ~AbstractHint() {}

    /// Get a name (or title) for the hint
    string getName() const { return m_name; }

    /// Get a longer description for the hint
    string getDescription() const { return m_description; }

    /// Get the cost (time penalty, in seconds) for using this hint
    int getCost() const { return m_cost; }

    /**
     * Actual hint processing, to help guess the given Move.
     * Since different hints can return different types of information,
     * a string is expected in all cases.
     */
    virtual string giveHint(const Move &iMove) const = 0;

private:
    string m_name;
    string m_description;
    int m_cost;
};


/**
 * Hint giving the score of the move
 */
class ScoreHint : public AbstractHint
{
public:
    ScoreHint();
    virtual string giveHint(const Move &iMove) const;
};


/**
 * Hint giving the orientation of the move (horizontal/vertical)
 */
class OrientationHint : public AbstractHint
{
public:
    OrientationHint();
    virtual string giveHint(const Move &iMove) const;
};


/**
 * Hint giving the position of the move
 */
class PositionHint : public AbstractHint
{
public:
    PositionHint();
    virtual string giveHint(const Move &iMove) const;
};


/**
 * Hint giving the length of the word
 */
class LengthHint : public AbstractHint
{
public:
    LengthHint();
    virtual string giveHint(const Move &iMove) const;
};


/**
 * Hint giving the letters of the move which come from the board
 */
class BoardLettersHint : public AbstractHint
{
public:
    BoardLettersHint();
    virtual string giveHint(const Move &iMove) const;
};


/**
 * Hint giving the letters of the word in alphabetical order
 */
class WordLettersHint : public AbstractHint
{
public:
    WordLettersHint();
    virtual string giveHint(const Move &iMove) const;
};


/**
 * Hint giving the first letter of the word
 */
class FirstLetterHint : public AbstractHint
{
public:
    FirstLetterHint();
    virtual string giveHint(const Move &iMove) const;
};


#endif

