/*****************************************************************************
 * Eliot
 * Copyright (C) 2002-2012 Antoine Fraboulet & Olivier Teulière
 * Authors: Antoine Fraboulet <antoine.fraboulet @@ free.fr>
 *          Olivier Teulière <ipkiss @@ gmail.com>
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

#include <boost/foreach.hpp>

#include <algorithm>
#include "pldrack.h"
#include "rack.h"


INIT_LOGGER(game, PlayedRack);


PlayedRack::PlayedRack()
    : m_reject(false)
{
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
    oTiles = m_oldTiles;
}


void PlayedRack::getNewTiles(vector<Tile> &oTiles) const
{
    oTiles.clear();
    oTiles = m_newTiles;
}


void PlayedRack::getAllTiles(vector<Tile> &oTiles) const
{
    oTiles.clear();
    oTiles = m_oldTiles;
    oTiles.insert(oTiles.end(), m_newTiles.begin(), m_newTiles.end());
}


void PlayedRack::reset()
{
    m_oldTiles.clear();
    m_newTiles.clear();
    m_reject = false;
}


void PlayedRack::resetNew()
{
    m_newTiles.clear();
}


Rack PlayedRack::getOld() const
{
    Rack rack;
    BOOST_FOREACH(const Tile &tile, m_oldTiles)
    {
        rack.add(tile);
    }
    return rack;
}


Rack PlayedRack::getNew() const
{
    Rack rack;
    BOOST_FOREACH(const Tile &tile, m_newTiles)
    {
        rack.add(tile);
    }
    return rack;
}


Rack PlayedRack::getRack() const
{
    Rack rack = getOld();
    BOOST_FOREACH(const Tile &tile, m_newTiles)
    {
        rack.add(tile);
    }
    return rack;
}


void PlayedRack::setOld(const Rack &iRack)
{
    m_oldTiles.clear();
    iRack.getTiles(m_oldTiles);
}


void PlayedRack::setNew(const Rack &iRack)
{
    m_newTiles.clear();
    iRack.getTiles(m_newTiles);
}


void PlayedRack::setManual(const wstring& iLetters)
{
    reset();

    // An empty rack is OK
    if (iLetters.empty())
        return;

    // Handle the reject sign
    unsigned int begin;
    if (iLetters[0] == L'-')
    {
        setReject();
        begin = 1;
    }
    else
        begin = 0;

    unsigned int i;
    for (i = begin; i < iLetters.size() && iLetters[i] != L'+'; i++)
    {
        addOld(Tile(iLetters[i]));
    }

    if (i < iLetters.size() && iLetters[i] == L'+')
    {
        for (i++; i < iLetters.size(); i++)
        {
            addNew(Tile(iLetters[i]));
        }
    }
}


bool PlayedRack::checkRack(unsigned int cMin, unsigned int vMin) const
{
    unsigned int v = 0;
    unsigned int c = 0;

    BOOST_FOREACH(const Tile &tile, m_oldTiles)
    {
        if (tile.isVowel()) v++;
        if (tile.isConsonant()) c++;
    }
    BOOST_FOREACH(const Tile &tile, m_newTiles)
    {
        if (tile.isVowel()) v++;
        if (tile.isConsonant()) c++;
    }
    return (v >= vMin) && (c >= cMin);
}


void PlayedRack::shuffleNew()
{
    std::random_shuffle(m_newTiles.begin(), m_newTiles.end());
}


void PlayedRack::shuffle()
{
    m_newTiles.insert(m_newTiles.end(),
                      m_oldTiles.begin(), m_oldTiles.end());
    m_oldTiles.clear();
    shuffleNew();
}


wstring PlayedRack::toString(display_mode mode) const
{
    wstring s;

    if (mode >= RACK_EXTRA && m_reject)
    {
        s += L"-";
    }

    BOOST_FOREACH(const Tile &tile, m_oldTiles)
    {
        s += tile.getDisplayStr();
    }

    if (mode > RACK_SIMPLE && getNbOld() > 0 && getNbNew() > 0)
    {
        s += L"+";
    }

    BOOST_FOREACH(const Tile &tile, m_newTiles)
    {
        s += tile.getDisplayStr();
    }

    return s;
}

