/*****************************************************************************
 * Eliot
 * Copyright (C) 2009-2012 Olivier Teulière
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
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

#include "play_model.h"

#include "encoding.h"
#include "debug.h"


INIT_LOGGER(qt, PlayModel);


PlayModel::PlayModel()
{
}


void PlayModel::clear()
{
    setCoord(Coord());
    setMove(Move());
}


void PlayModel::setCoord(const Coord &iCoord)
{
    // Avoid useless work
    if (iCoord == m_currCoord)
        return;

    m_prevCoord = m_currCoord;
    m_currCoord = iCoord;
    emit coordChanged(iCoord, m_prevCoord);
}


void PlayModel::setMove(const Move &iMove)
{
    ASSERT(iMove.isValid() || iMove.isInvalid() || iMove.isNull(),
           "Unexpected move type");

    LOG_DEBUG("Setting PlayModel move to " << lfw(iMove.toString()));

    // Avoid useless work
    if (iMove == m_currMove)
        return;

    m_prevMove = m_currMove;
    m_currMove = iMove;
    emit moveChanged(iMove, m_prevMove);
}



void PlayModel::playWord(const wstring &iWord, const wstring &iCoord)
{
    emit movePlayed(iWord, iCoord);
}


