/*****************************************************************************
 * Eliot
 * Copyright (C) 2013 Olivier Teulière
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

#include "game_signals.h"

#include "public_game.h"
#include "encoding.h"
#include "debug.h"


INIT_LOGGER(qt, GameSignals);


GameSignals::GameSignals()
    : m_game(0), m_currentTurn(0), m_lastTurn(0)
{
}


void GameSignals::notifyGameChanged(PublicGame *iGame)
{
    m_game = iGame;
    emit gameChangedNonConst(iGame);
    emit gameChanged(iGame);
    if (iGame != NULL)
        notifyGameUpdated();
}


void GameSignals::notifyGameUpdated()
{
    emit gameUpdated();

    if (m_game == NULL)
        return;

    unsigned currTurn = m_game->getCurrTurn();
    bool isLastTurn = m_game->isLastTurn();

    // Emit the turnChanged() signal if needed
    if (currTurn != m_currentTurn)
    {
        m_currentTurn = currTurn;
        LOG_DEBUG("Emitting turnChanged(" << currTurn << ", " << isLastTurn << ")");
        emit turnChanged(currTurn, isLastTurn);
    }
    // Emit the newTurn() signal if needed
    if (currTurn > m_lastTurn)
    {
        m_lastTurn = currTurn;
        LOG_DEBUG("Emitting newTurn(" << currTurn << ")");
        emit newTurn(currTurn);
    }
    // Emit the gameRackChanged() signal if needed
    if (m_game->getCurrentRack().toString(PlayedRack::RACK_EXTRA) != m_lastGameRack.toString(PlayedRack::RACK_EXTRA))
    {
        m_lastGameRack = m_game->getCurrentRack();
        LOG_DEBUG("Emitting gameRackChanged(" << lfw(m_lastGameRack.toString()) << ")");
        emit gameRackChanged(m_lastGameRack);
    }
}


