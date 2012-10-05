/*****************************************************************************
 * Eliot
 * Copyright (C) 2004-2007 Olivier Teulière & Antoine Fraboulet
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
 *          Antoine Fraboulet <antoine.fraboulet @@ free.fr>
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

#include "tile.h"
#include "rack.h"
#include "pldrack.h"
#include "round.h"
#include "results.h"
#include "board.h"
#include "player.h"
#include "turn_data.h"
#include "history.h"
#include "encoding.h"
#include "settings.h"

#include "debug.h"


INIT_LOGGER(game, Player);


Player::Player()
    : m_id(0), m_tableNb(0)
{
}


void Player::setId(unsigned iId)
{
    m_id = iId;
    // Default table nb, equal to the player ID + 1
    if (m_tableNb == 0)
        m_tableNb = iId + 1;
}


const PlayedRack & Player::getCurrentRack() const
{
    return m_history.getCurrentRack();
}


void Player::setCurrentRack(const PlayedRack &iPld)
{
    m_history.setCurrentRack(iPld);
}


const PlayedRack & Player::getLastRack() const
{
    return m_history.getPreviousTurn().getPlayedRack();
}


const Move & Player::getLastMove() const
{
    return m_history.getPreviousTurn().getMove();
}


void Player::removeLastTurn()
{
    m_history.removeLastTurn();
}

unsigned Player::getWarningsNb() const
{
    unsigned total = 0;
    for (unsigned i = 0; i < m_history.getSize(); ++i)
    {
        total += m_history.getTurn(i).getWarningsNb();
    }
    return total;
}


int Player::getMovePoints() const
{
    int total = 0;
    for (unsigned i = 0; i < m_history.getSize(); ++i)
    {
        total += m_history.getTurn(i).getMove().getScore();
    }
    return total;
}


int Player::getPenaltyPoints() const
{
    int total = 0;
    for (unsigned i = 0; i < m_history.getSize(); ++i)
    {
        total += m_history.getTurn(i).getPenaltyPoints();
    }

    // Add penalties due to warnings
    unsigned warningsNb = getWarningsNb();
    int limit = Settings::Instance().getInt("arbitration.warnings-limit");
    if ((int)warningsNb > limit)
    {
        int penaltiesPoints =
            Settings::Instance().getInt("arbitration.penalty-value");
        total -= penaltiesPoints * (warningsNb - limit);
    }
    return total;
}


int Player::getSoloPoints() const
{
    int total = 0;
    for (unsigned i = 0; i < m_history.getSize(); ++i)
    {
        total += m_history.getTurn(i).getSoloPoints();
    }
    return total;
}


int Player::getEndGamePoints() const
{
    int total = 0;
    for (unsigned i = 0; i < m_history.getSize(); ++i)
    {
        total += m_history.getTurn(i).getEndGamePoints();
    }
    return total;
}


int Player::getTotalScore() const
{
    return getMovePoints() + getSoloPoints() + getPenaltyPoints() + getEndGamePoints();
}


wstring Player::toString() const
{
    wstring res = L"Player ";

    wchar_t buff[6];
    _swprintf(buff, 5, L"%d\n", m_id);
    res += wstring(buff);
    res += m_history.toString() + L"\n";
    res += L"score ";
    _swprintf(buff, 5, L"%d\n", getTotalScore());
    res += wstring(buff);
    return res;
}

