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
public:
//     Turn();
    Turn(int iNum, int iPlayer,
         const PlayedRack& iPldRack, const Round& iRound);
    virtual ~Turn() {};

#if 0
    void setNum(int iNum)                          { m_num = iNum; }
    void setPlayer(int iPldRack)                   { m_player = iPldRack; }
    void setPlayedRack(const PlayedRack& iPldRack) { m_pldrack = iPldRack; }
    void setRound(const Round& iRound)             { m_round = iRound; }
#endif

    int               getNum()        const { return m_num; }
    int               getPlayer()     const { return m_player; }
    const PlayedRack& getPlayedRack() const { return m_pldrack; }
    const Round&      getRound()      const { return m_round; }

//     void operator=(const Turn &iOther);
    string toString(bool iShowExtraSigns = false) const;

private:
    int        m_num;
    int        m_player;
    PlayedRack m_pldrack;
    Round      m_round;

};

#endif


/// Local Variables:
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
