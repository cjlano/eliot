/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: training.h,v 1.1 2005/02/05 11:14:56 ipkiss Exp $
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

#ifndef _TRAINING_H_
#define _TRAINING_H_

#include "game.h"

using std::string;


class Training: public Game
{
public:
    /*************************
     * Functions to create and destroy a game
     * the dictionary does not belong to the
     * game (ie: it won't be destroyed by ~Game)
     *
     * The dictionary can be changed afterwards by setDic
     *************************/
    Training(const Dictionary &iDic);
    virtual ~Training();

    virtual GameMode getMode() const { return kTRAINING; }

    /*************************
     * Game handling
     *************************/
    virtual int start();
    virtual int setRackRandom(int, bool, set_rack_mode);
    virtual int play(const string &iCoord, const string &iWord);
    virtual int endTurn();
    int search();
    int playResult(int);
    int setRackManual(bool iCheck, const string &iLetters);

};

#endif /* _TRAINING_H_ */
