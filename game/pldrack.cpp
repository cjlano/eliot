/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: pldrack.cpp,v 1.1 2005/02/05 11:14:56 ipkiss Exp $
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

#include "rack.h"
#include "pldrack.h"

#include "debug.h"


void PlayedRack::addOld(const Tile &t)
{
    m_oldTiles.push_back(t);
}


void PlayedRack::addNew(const Tile &t)
{
    m_newTiles.push_back(t);
}


void PlayedRack::getOldTiles(vector<Tile> &oTiles) const
{
    oTiles.clear();
    for (int i = 0; i < nOld(); i++)
        oTiles.push_back(m_oldTiles[i]);
}


void PlayedRack::getNewTiles(vector<Tile> &oTiles) const
{
    oTiles.clear();
    for (int i = 0; i < nNew(); i++)
        oTiles.push_back(m_newTiles[i]);
}


void PlayedRack::getAllTiles(vector<Tile> &oTiles) const
{
    oTiles.clear();
    for (int i = 0; i < nOld(); i++)
        oTiles.push_back(m_oldTiles[i]);
    for (int j = 0; j < nNew(); j++)
        oTiles.push_back(m_newTiles[j]);
}


void PlayedRack::reset()
{
    m_oldTiles.clear();
    m_newTiles.clear();
}


void PlayedRack::resetNew()
{
    m_newTiles.clear();
}


void PlayedRack::getOld(Rack &oRack) const
{
    vector<Tile>::const_iterator it;
    oRack.clear();
    for (it = m_oldTiles.begin(); it != m_oldTiles.end(); it++)
    {
        oRack.add(*it);
    }
}


void PlayedRack::getNew(Rack &oRack) const
{
    vector<Tile>::const_iterator it;
    oRack.clear();
    for (it = m_newTiles.begin(); it != m_newTiles.end(); it++)
    {
        oRack.add(*it);
    }
}


void PlayedRack::getRack(Rack &oRack) const
{
    vector<Tile>::const_iterator it;
    getOld(oRack);
    for (it = m_newTiles.begin(); it != m_newTiles.end(); it++)
    {
        oRack.add(*it);
    }
}


void PlayedRack::setOld(const Rack &iRack)
{
    list<Tile> l;
    iRack.getTiles(l);

    m_oldTiles.clear();
    list<Tile>::const_iterator it;
    for (it = l.begin(); it != l.end(); it++)
    {
        addOld(*it);
    }
}


void PlayedRack::setNew(const Rack &iRack)
{
    list<Tile> l;
    iRack.getTiles(l);

    m_newTiles.clear();
    list<Tile>::const_iterator it;
    for (it = l.begin(); it != l.end(); it++)
    {
        addNew(*it);
    }
}


bool PlayedRack::checkRack(int iMin)
{
    vector<Tile>::const_iterator it;
    int v = 0;
    int c = 0;

    for (it = m_oldTiles.begin(); it != m_oldTiles.end(); it++)
    {
        if (it->isVowel()) v++;
        if (it->isConsonant()) c++;
    }
    for (it = m_newTiles.begin(); it != m_newTiles.end(); it++)
    {
        if (it->isVowel()) v++;
        if (it->isConsonant()) c++;
    }
    return (v >= iMin) && (c >= iMin);
}


void PlayedRack::operator=(const PlayedRack &iOther)
{
    vector<Tile>::const_iterator it;
    m_oldTiles.clear();
    for (it = iOther.m_oldTiles.begin(); it != iOther.m_oldTiles.end(); it++)
        m_oldTiles.push_back(*it);
    m_newTiles.clear();
    for (it = iOther.m_newTiles.begin(); it != iOther.m_newTiles.end(); it++)
        m_newTiles.push_back(*it);
}

