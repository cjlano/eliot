/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: game_factory.h,v 1.1 2005/02/24 08:06:25 ipkiss Exp $
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

#ifndef _GAME_FACTORY_H_
#define _GAME_FACTORY_H_

#include "game.h"
#include "training.h"
#include "freegame.h"
#include "duplicate.h"


class GameFactory
{
public:
    /*************************
     * Functions to create and destroy a game
     * The dictionary does not belong to the
     * game (ie: it won't be destroyed by ~Game)
     *
     * The dictionary can be changed afterwards by setDic
     *************************/
    static GameFactory *Instance();
    static void Destroy();

    Training *createTraining(const Dictionary &iDic);
    FreeGame *createFreeGame(const Dictionary &iDic);
    Duplicate *createDuplicate(const Dictionary &iDic);
    //Game *loadGame(FILE *fin, const Dictionary &iDic);

    void releaseGame(Game &iGame);

private:

    GameFactory() {}
    virtual ~GameFactory() {}

    // The unique instance of the class
    static GameFactory *m_factory;
};

#endif // _GAME_FACTORY_H_
