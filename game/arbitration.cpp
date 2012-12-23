/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2012 Olivier Teulière
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

#include "arbitration.h"
#include "rack.h"
#include "player.h"
#include "turn.h"
#include "results.h"
#include "cmd/game_rack_cmd.h"
#include "cmd/player_move_cmd.h"
#include "cmd/player_event_cmd.h"
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
    const wstring &upperLetters = toUpper(iLetters);
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
    Move move;
    checkPlayedWord(iCoords, iWord, move, true);
    return move;
}


void Arbitration::setSolo(unsigned iPlayerId, int iPoints)
{
    ASSERT(iPlayerId < getNPlayers(), "Wrong player number");
    ASSERT(iPoints >= 0, "Expected a positive value for the solo");

    if (iPoints == 0)
    {
        // Retrieve the default value of the solo
        iPoints = Settings::Instance().getInt("arbitration.solo-value");
    }
    LOG_INFO("Giving a solo of " << iPoints << " to player " << iPlayerId);

    // If an existing solo exists, get rid of it
    const PlayerEventCmd *cmd = getPlayerEvent(iPlayerId, PlayerEventCmd::SOLO);
    if (cmd != 0)
    {
        accessNavigation().dropCommand(*cmd);
    }

    Command *pCmd = new PlayerEventCmd(*m_players[iPlayerId],
                                       PlayerEventCmd::SOLO, iPoints);
    accessNavigation().insertCommand(pCmd);
}


void Arbitration::removeSolo(unsigned iPlayerId)
{
    ASSERT(iPlayerId < getNPlayers(), "Wrong player number");
    const PlayerEventCmd *cmd = getPlayerEvent(iPlayerId, PlayerEventCmd::SOLO);
    ASSERT(cmd != 0, "No matching PlayerEventCmd found");

    accessNavigation().dropCommand(*cmd);
}


int Arbitration::getSolo(unsigned iPlayerId) const
{
    ASSERT(iPlayerId < getNPlayers(), "Wrong player number");
    const PlayerEventCmd *cmd = getPlayerEvent(iPlayerId, PlayerEventCmd::SOLO);
    if (cmd == 0)
        return 0;
    return cmd->getPoints();
}


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
    ASSERT(iPoints <= 0, "Expected a negative value for the penalty");

    if (iPoints == 0)
    {
        // Retrieve the default value of the penalty
        iPoints = Settings::Instance().getInt("arbitration.penalty-value");

        // By convention, use negative values to indicate a penalty
        iPoints = -iPoints;
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
        // When the resulting value is 0, instead of merging we drop the existing one
        Command *pCmd = new PlayerEventCmd(*m_players[iPlayerId],
                                           PlayerEventCmd::PENALTY,
                                           iPoints + cmd->getPoints());
        accessNavigation().replaceCommand(*cmd, pCmd);
    }
}


void Arbitration::removePenalty(unsigned iPlayerId)
{
    ASSERT(iPlayerId < getNPlayers(), "Wrong player number");
    const PlayerEventCmd *cmd = getPlayerEvent(iPlayerId, PlayerEventCmd::PENALTY);
    ASSERT(cmd != 0, "No penalty found for player " << iPlayerId);
    accessNavigation().dropCommand(*cmd);
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

    // Automatically update the solos if requested
    bool useSoloAuto = Settings::Instance().getBool("arbitration.solo-auto");
    if (useSoloAuto)
    {
        unsigned minNbPlayers = Settings::Instance().getInt("arbitration.solo-players");
        int soloValue = Settings::Instance().getInt("arbitration.solo-value");
        setSoloAuto(minNbPlayers, soloValue);
    }
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

