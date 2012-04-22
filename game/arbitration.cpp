/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2009 Olivier Teulière
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
#include <algorithm> // For transform
#include <cwctype> // For towupper

#include "arbitration.h"
#include "rack.h"
#include "player.h"
#include "turn_cmd.h"
#include "game_rack_cmd.h"
#include "results.h"
#include "player_move_cmd.h"
#include "player_event_cmd.h"
#include "settings.h"
#include "encoding.h"
#include "debug.h"


INIT_LOGGER(game, Arbitration);


Arbitration::Arbitration(const GameParams &iParams)
    : Duplicate(iParams)
{
}


void Arbitration::setRackRandom()
{
    undoCurrentRack();

    const PlayedRack &newRack =
        helperSetRackRandom(getHistory().getCurrentRack(), true, RACK_NEW);
    setGameAndPlayersRack(newRack);
}


void Arbitration::setRackManual(const wstring &iLetters)
{
    undoCurrentRack();

    // Letters can be lowercase or uppercase as they are
    // coming from user input. We do not consider a lowercase
    // letter to be a joker which has been assigned to a letter.
    // As a result, we simply make all the letters uppercase
    wstring upperLetters = iLetters;
    std::transform(upperLetters.begin(), upperLetters.end(),
                   upperLetters.begin(), towupper);
    const PlayedRack &newRack = helperSetRackManual(false, upperLetters);
    setGameAndPlayersRack(newRack);
}


void Arbitration::search(LimitResults &oResults)
{
    const Rack &rack = getHistory().getCurrentRack().getRack();
    LOG_DEBUG("Performing search for rack " + lfw(rack.toString()));
    int limit = Settings::Instance().getInt("arbitration.search-limit");
    oResults.setLimit(limit);
    oResults.search(getDic(), getBoard(), rack, getHistory().beforeFirstRound());
    LOG_DEBUG("Found " << oResults.size() << " results");
}


Move Arbitration::checkWord(const wstring &iWord,
                            const wstring &iCoords) const
{
    Round round;
    int res = checkPlayedWord(iCoords, iWord, round, false);
    if (res == 0)
        return Move(round);
    return Move(iWord, iCoords);
}


/// Predicate to help retrieving commands
struct MatchingPlayerAndEventType : public unary_function<PlayerEventCmd, bool>
{
    MatchingPlayerAndEventType(unsigned iPlayerId, int iEventType)
        : m_playerId(iPlayerId), m_eventType(iEventType) {}

    bool operator()(const PlayerEventCmd &cmd)
    {
        return cmd.getPlayer().getId() == m_playerId
            && cmd.getEventType() == m_eventType;
    }

    const unsigned m_playerId;
    const int m_eventType;
};


void Arbitration::addWarning(unsigned iPlayerId)
{
    ASSERT(iPlayerId < getNPlayers(), "Wrong player number");
    Command *pCmd = new PlayerEventCmd(*m_players[iPlayerId],
                                       PlayerEventCmd::WARNING, 0);
    accessNavigation().insertCommand(pCmd);
}


void Arbitration::removeWarning(unsigned iPlayerId)
{
    ASSERT(iPlayerId < getNPlayers(), "Wrong player number");
    const PlayerEventCmd *cmd = getPlayerEvent(iPlayerId, PlayerEventCmd::WARNING);
    ASSERT(cmd != 0, "No matching PlayerEventCmd found");

    accessNavigation().dropCommand(*cmd);
}


bool Arbitration::hasWarning(unsigned iPlayerId) const
{
    ASSERT(iPlayerId < getNPlayers(), "Wrong player number");
    const PlayerEventCmd *cmd = getPlayerEvent(iPlayerId, PlayerEventCmd::WARNING);
    return cmd != 0;
}


void Arbitration::addPenalty(unsigned iPlayerId, int iPoints)
{
    ASSERT(iPlayerId < getNPlayers(), "Wrong player number");

    if (iPoints == 0)
    {
        // Retrieve the default value of the penalty
        iPoints = Settings::Instance().getInt("arbitration.default-penalty");
    }
    LOG_INFO("Giving a penalty of " << iPoints << " to player " << iPlayerId);

    // If an existing penalty exists, merge it with the new one
    const PlayerEventCmd *cmd = getPlayerEvent(iPlayerId, PlayerEventCmd::PENALTY);
    if (cmd == 0)
    {
        Command *pCmd = new PlayerEventCmd(*m_players[iPlayerId],
                                           PlayerEventCmd::PENALTY, iPoints);
        accessNavigation().insertCommand(pCmd);
    }
    else
    {
        Command *pCmd = new PlayerEventCmd(*m_players[iPlayerId],
                                           PlayerEventCmd::PENALTY,
                                           iPoints + cmd->getPoints());
        accessNavigation().replaceCommand(*cmd, pCmd);
    }
}


int Arbitration::getPenalty(unsigned iPlayerId) const
{
    ASSERT(iPlayerId < getNPlayers(), "Wrong player number");
    const PlayerEventCmd *cmd = getPlayerEvent(iPlayerId, PlayerEventCmd::PENALTY);
    if (cmd == 0)
        return 0;
    return cmd->getPoints();
}


void Arbitration::assignMove(unsigned int iPlayerId, const Move &iMove)
{
    ASSERT(iPlayerId < getNPlayers(), "Wrong player number");
    recordPlayerMove(*m_players[iPlayerId], iMove);
}


void Arbitration::finalizeTurn()
{
    tryEndTurn();
}


void Arbitration::undoCurrentRack()
{
    // The interface is supposed to make sure we are never in this case
    ASSERT(getNavigation().isLastTurn(),
           "Cannot change rack for an old turn");

    // Find the GameRackCmd we want to undo
    const GameRackCmd *cmd =
        getNavigation().getCurrentTurn().findMatchingCmd<GameRackCmd>();
    ASSERT(cmd != 0, "No matching GameRackCmd found");

    accessNavigation().dropFrom(*cmd);
}


const PlayerEventCmd * Arbitration::getPlayerEvent(unsigned iPlayerId,
                                                   int iEventType) const
{
    ASSERT(iPlayerId < getNPlayers(), "Wrong player number");
    MatchingPlayerAndEventType predicate(iPlayerId, iEventType);
    const TurnCmd &currTurn = getNavigation().getCurrentTurn();
    return currTurn.findMatchingCmd<PlayerEventCmd>(predicate);
}

