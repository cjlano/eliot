/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: pldrack.h,v 1.3 2005/02/05 11:14:56 ipkiss Exp $
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

#ifndef _PLAYEDRACK_H_
#define _PLAYEDRACK_H_

#include <vector>
#include "tile.h"

class Rack;

using namespace std;


  /*************************
   * A Playedrack is 
   * 
   *************************/

class PlayedRack
{
public:
    PlayedRack() {}
    virtual ~PlayedRack() {}

    void reset();
    void resetNew();

    void getOld(Rack &oRack) const;
    void getNew(Rack &oRack) const;
    void getRack(Rack &oRack) const;

    void setOld(const Rack &iRack);
    void setNew(const Rack &iRack);

    int nTiles() const      { return nNew() + nOld(); }
    int nNew() const        { return m_newTiles.size(); }
    int nOld() const        { return m_oldTiles.size(); }

    void addNew(const Tile &t);
    void addOld(const Tile &t);
    void getNewTiles(vector<Tile> &oTiles) const;
    void getOldTiles(vector<Tile> &oTiles) const;
    void getAllTiles(vector<Tile> &oTiles) const;

    bool checkRack(int iMin);

    void operator=(const PlayedRack &iOther);

private:
    vector<Tile> m_oldTiles;
    vector<Tile> m_newTiles;
};

#endif
