/*****************************************************************************
 * Copyright (C) 2005 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: game_factory.cpp,v 1.1 2005/02/24 08:06:25 ipkiss Exp $
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

#include "game_factory.h"


GameFactory *GameFactory::m_factory = NULL;


GameFactory *GameFactory::Instance()
{
    if (m_factory == NULL)
        m_factory = new GameFactory;
    return m_factory;
}


void GameFactory::Destroy()
{
    if (m_factory)
        delete m_factory;
    m_factory = NULL;
}


Training *GameFactory::createTraining(const Dictionary &iDic)
{
    Training *game = new Training(iDic);
    return game;
}


FreeGame *GameFactory::createFreeGame(const Dictionary &iDic)
{
    FreeGame *game = new FreeGame(iDic);
    return game;
}


Duplicate *GameFactory::createDuplicate(const Dictionary &iDic)
{
    Duplicate *game = new Duplicate(iDic);
    return game;
}


void GameFactory::releaseGame(Game &iGame)
{
    delete &iGame;
}

