/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: duplicate.h,v 1.1 2005/02/05 11:14:56 ipkiss Exp $
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

#ifndef _DUPLICATE_H_
#define _DUPLICATE_H_

#include "game.h"

using std::string;


class Duplicate: public Game
{
public:
    /*************************
     * Functions to create and destroy a game
     * the dictionary does not belong to the
     * game (ie: it won't be destroyed by ~Game)
     *
     * The dictionary can be changed afterwards by setDic
     *************************/
    Duplicate(const Dictionary &iDic);
    virtual ~Duplicate();

    virtual GameMode getMode() const { return kDUPLICATE; }

    /*************************
     * Game handling
     *************************/
    virtual int start();
    virtual int setRackRandom(int, bool, set_rack_mode);
    virtual int play(const string &iCoord, const string &iWord);
    virtual int endTurn();
    int setPlayer(int);

private:

    void playRound(const Round &iRound, int n);
    int  endTurnForReal();
    void end();
    int  duplicateAI(int n);
};

#endif /* _DUPLICATE_H_ */
