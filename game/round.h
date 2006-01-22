/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
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

#ifndef _ROUND_H_
#define _ROUND_H_

#include <vector>
#include "tile.h"
#include "coord.h"

using namespace std;


/**
 * A Round is the representation of a played word (or going to be played).
 * It contains the word itself, of course, but also information of position
 * on the board, origin of letters (board for a letter already placed, rack
 * for a letter just being played), points, etc...
 */
class Round
{
public:

    /*************************
     *
     *************************/
    Round();
    virtual ~Round() {}
    void init();

    /*************************
     *
     *************************/
    void addRightFromBoard(Tile);
    void removeRightToBoard(Tile);
    void addRightFromRack(Tile, bool);
    void removeRightToRack(Tile, bool);

    /*************************
     * General setters
     *************************/
    void setPoints(int iPoints)    { m_points = iPoints; }
    void setBonus(bool iBonus)     { m_bonus = iBonus; }
    void setTile(int iIndex, const Tile &iTile) { m_word[iIndex] = iTile; }
    void setWord(const vector<Tile> &iTiles);
    void setFromRack(int iIndex);
    void setFromBoard(int iIndex);
    void setJoker(int iIndex, bool value = true);

    /*************************
     * General getters
     *************************/
    bool isJoker         (int iIndex) const;
    bool isPlayedFromRack(int iIndex) const;
    const Tile& getTile  (int iIndex) const;

    wstring getWord() const;
    int getWordLen()  const;
    int getPoints()   const       { return m_points; }
    int getBonus()    const       { return m_bonus; }

    /*************************
     * Coordinates
     *************************/
    const Coord& getCoord() const { return m_coord; }
    Coord& accessCoord()          { return m_coord; }


    wstring toString() const;

private:
    vector<Tile> m_word;
    vector<char> m_tileOrigin;
    Coord m_coord;
    int m_points;
    int m_bonus;
};

#endif

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
