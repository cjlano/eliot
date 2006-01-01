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

/**
 *  \file   pldrack.cpp
 *  \brief  Improved Rack class with old and new tiles
 *  \author Antoine Fraboulet & Olivier Teuliere
 *  \date   2002 - 2005
 */

#include "rack.h"
#include "pldrack.h"

#include "debug.h"


PlayedRack::PlayedRack()
{
  reject = false;
}

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

int PlayedRack::setManual(const string& iLetters)
{
    unsigned int i;
    reset();

    if (iLetters.size() == 0)
    {
        return 0; /* empty is ok */
    }

    for (i = 0; i < iLetters.size() && iLetters[i] != '+'; i++)
    {
        Tile tile(iLetters[i]);
        if (tile.isEmpty())
        {
            return 1; /* */ 
        }
        addOld(tile);
    }

    if (i < iLetters.size() && iLetters[i] == '+')
    {
        for (i++; i < iLetters.size(); i++)
        {
            Tile tile(iLetters[i]);
            if (tile.isEmpty())
            {
                return 1; /* */
            }
            addNew(tile);
        }
    }

    return 0;
}

bool PlayedRack::checkRack(int cMin, int vMin) const
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
    return (v >= vMin) && (c >= cMin);
}


void PlayedRack::operator=(const PlayedRack &iOther)
{
    m_oldTiles = iOther.m_oldTiles;
    m_newTiles = iOther.m_newTiles;
}


string PlayedRack::toString(display_mode mode) const
{
    string s("");
    vector<Tile>::const_iterator it;
  
    if (nOld() > 0)
    {
	for (it = m_oldTiles.begin(); it != m_oldTiles.end(); it++)
	    s += it->toChar();
    }

    if (mode > RACK_SIMPLE && nOld() > 0 && nNew() > 0)
    {
	s += "+";
    }

    if (mode > RACK_EXTRA  && reject)
    {
	s += "-";
	// nouveau tirage : rejet
	// pas après un scrabble
    }

    if (nNew() > 0)
    {
	for (it = m_newTiles.begin(); it != m_newTiles.end(); it++)
	    s += it->toChar();
    }

    return s;
}

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
