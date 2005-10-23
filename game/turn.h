/* Eliot                                                                     */
/* Copyright (C) 1999  Antoine Fraboulet                                     */
/*                                                                           */
/* This file is part of Eliot.                                               */
/*                                                                           */
/* Eliot is free software; you can redistribute it and/or modify             */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* Eliot is distributed in the hope that it will be useful,                  */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

/* $Id: turn.h,v 1.2 2005/10/23 14:53:43 ipkiss Exp $ */

/**
 *  \file   turn.h
 *  \brief  Game turn (= id + pldrack + round) 
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#ifndef _TURN_H
#define _TURN_H

class Turn
{
 protected:
    int        num;
    int        player;
    PlayedRack pldrack;
    Round      round;
    
 public:
    Turn();
    ~Turn();

    void setNum(int);
    void setPlayer(int);
    void setPlayedRack(const PlayedRack&);
    void setRound(const Round&);

    int        getNum()        const;
    int        getPlayer()     const;
    PlayedRack getPlayedRack() const;
    Round      getRound()      const;

    void operator=(const Turn &iOther);
    string toString(bool showExtraSigns = false) const;
};

#endif


/// Local Variables:
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
