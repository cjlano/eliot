/*******************************************************************
 * Eliot
 * Copyright (C) 2008 Olivier Teulière
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
#include <iostream>

#include "navigation.h"
#include "turn_cmd.h"
#include "debug.h"
#include "encoding.h"


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
    lastTurn();
    m_turnCommands.push_back(new TurnCmd);
    ++m_currTurn;
}


void Navigation::addAndExecute(Command *iCmd)
{
    ASSERT(isLastTurn(), "Trying to add a command to an old turn!");
    ASSERT(m_currTurn >= 1, "Bug in the turns vector");
    ASSERT(m_currTurn - 1 < m_turnCommands.size(), "Bug in the turns vector");
    m_turnCommands[m_currTurn - 1]->addAndExecute(iCmd);
}


void Navigation::prevTurn()
{
    if (m_currTurn > 0)
    {
        --m_currTurn;
        m_turnCommands[m_currTurn]->undo();
    }
}


void Navigation::nextTurn()
{
    if (m_currTurn < m_turnCommands.size())
    {
        m_turnCommands[m_currTurn]->execute();
        ++m_currTurn;
    }
}


void Navigation::firstTurn()
{
    while (m_currTurn > 0)
    {
        prevTurn();
    }
}


void Navigation::lastTurn()
{
    while (m_currTurn < m_turnCommands.size())
    {
        nextTurn();
    }
}


void Navigation::clearFuture()
{
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


void Navigation::print() const
{
    cout << "=== Commands history ===" << endl;
    BOOST_FOREACH(Command *c, m_turnCommands)
    {
        cout << convertToMb(c->toString()) << endl;
    }
}

