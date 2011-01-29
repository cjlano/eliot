/*******************************************************************
 * Eliot
 * Copyright (C) 2009 Olivier Teulière
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

#include <boost/foreach.hpp>

#include "navigation.h"
#include "turn_cmd.h"
#include "game_exception.h"
#include "debug.h"
#include "encoding.h"


INIT_LOGGER(game, Navigation);

Navigation::Navigation()
    : m_currTurn(0)
{
    newTurn();
}


Navigation::~Navigation()
{
    BOOST_FOREACH(Command *c, m_turnCommands)
    {
        delete c;
    }
}


void Navigation::newTurn()
{
    LOG_INFO("New turn");
    lastTurn();
    m_turnCommands.push_back(new TurnCmd);
    ++m_currTurn;
}


void Navigation::addAndExecute(Command *iCmd)
{
    if (!isLastTurn())
        throw GameException("Cannot add a command to an old turn");

    ASSERT(m_currTurn >= 1, "Bug in the turns vector (1)");
    ASSERT(m_currTurn - 1 < m_turnCommands.size(), "Bug in the turns vector (2)");
    m_turnCommands[m_currTurn - 1]->addAndExecute(iCmd);
}


unsigned int Navigation::getCurrTurn() const
{
    unsigned int currTurn = m_currTurn;
    if (isLastTurn() && m_turnCommands.back()->isEmpty())
        --currTurn;
    return currTurn - 1;
}


unsigned int Navigation::getNbTurns() const
{
    unsigned int count = m_turnCommands.size();
    if (m_turnCommands.back()->isEmpty())
        --count;
    return count - 1;
}


bool Navigation::isFirstTurn() const
{
    return m_currTurn == 1 ||
        (m_currTurn == 2 && m_turnCommands[1]->isEmpty());
}


bool Navigation::isLastTurn() const
{
    return m_currTurn == m_turnCommands.size();
}


void Navigation::prevTurn()
{
    if (m_currTurn > 1)
    {
        LOG_DEBUG("Navigating to the previous turn");
        --m_currTurn;
        m_turnCommands[m_currTurn]->undo();
        // Special case: when the last turn is empty, automatically
        // undo the previous turn as well
        if (m_currTurn + 1 == m_turnCommands.size() &&
            m_turnCommands[m_currTurn]->isEmpty())
        {
            prevTurn();
        }
    }
}


void Navigation::nextTurn()
{
    if (m_currTurn < m_turnCommands.size())
    {
        LOG_DEBUG("Navigating to the next turn");
        m_turnCommands[m_currTurn]->execute();
        ++m_currTurn;
        // Special case: when the last turn is empty, automatically
        // execute it
        if (m_currTurn + 1 == m_turnCommands.size() &&
            m_turnCommands[m_currTurn]->isEmpty())
        {
            nextTurn();
        }
    }
}


void Navigation::firstTurn()
{
    LOG_DEBUG("Navigating to the first turn");
    while (m_currTurn > 1)
    {
        prevTurn();
    }
}


void Navigation::lastTurn()
{
    LOG_DEBUG("Navigating to the last turn");
    while (m_currTurn < m_turnCommands.size())
    {
        nextTurn();
    }
}


void Navigation::clearFuture()
{
    LOG_DEBUG("Erasing all the future turns");

    // Replay the auto-execution turns
    // (i.e. turns where only the AI was involved)
    while (!isLastTurn() && m_turnCommands[m_currTurn]->isAutoExecution())
        nextTurn();

    // When there is no future, don't do anything
    if (isLastTurn())
        return;

    for (unsigned int i = m_currTurn; i < m_turnCommands.size(); ++i)
    {
        delete m_turnCommands[i];
    }
    while (m_turnCommands.size() > m_currTurn)
    {
        m_turnCommands.pop_back();
    }
    newTurn();
    // Sanity check
    ASSERT(isLastTurn(),
           "After removing the next turns, we should be at the last turn");
}


const vector<TurnCmd *> & Navigation::getCommands() const
{
    return m_turnCommands;
}


void Navigation::print() const
{
    LOG_DEBUG("=== Commands history ===");
    LOG_DEBUG("Current position right after turn " << m_currTurn  - 1);
    int index = 0;
    BOOST_FOREACH(const Command *c, m_turnCommands)
    {
        LOG_DEBUG(index << " " << convertToMb(c->toString()));
        ++index;
    }
}

