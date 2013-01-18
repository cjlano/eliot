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

#ifndef MOVE_SELECTOR_H_
#define MOVE_SELECTOR_H_

#include "logging.h"

class Round;
class BestResults;
class Bag;
class Dictionary;


/**
 * This helper class uses various heuristics to determine which move
 * is the "best" for a given situation. At the moment, only one situation
 * is implemented, namely choosing an appropriate master move for duplicate
 * games.
 */
class MoveSelector
{
    DEFINE_LOGGER();
public:

    MoveSelector(const Bag &iBag, const Dictionary &iDic);

    /**
     * Return a move to be used as "master move" in a duplicate game.
     * The method takes the given moves and tries to find the optimal move,
     * i.e. the move which maximizes the following criteria:
     *  - it uses as few jokers from the rack as possible
     *  - it offers many prefixes and/or suffixes
     *  - it opens the game
     *  - it leaves good letters in the rack
     * Since these criteria may not reach their maximum for the same move,
     * some compromises must be done.
     */
    Round selectMaster(const BestResults &iResults) const;

private:
    const Bag &m_bag;
    const Dictionary &m_dic;

    int evalScore(const Round &iRound) const;
    int evalForJokersInRack(const Round &iRound) const;
    int evalForExtensions(const Round &iRound) const;

};

#endif

