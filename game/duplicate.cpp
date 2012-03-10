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
#include <sstream>

#include "config.h"
#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
#else
#   define _(String) String
#endif

#include "duplicate.h"
#include "game_exception.h"
#include "dic.h"
#include "tile.h"
#include "rack.h"
#include "round.h"
#include "move.h"
#include "pldrack.h"
#include "results.h"
#include "player.h"
#include "player_move_cmd.h"
#include "player_rack_cmd.h"
#include "game_move_cmd.h"
#include "game_rack_cmd.h"
#include "mark_played_cmd.h"
#include "master_move_cmd.h"
#include "ai_player.h"
#include "navigation.h"
#include "turn_cmd.h"
#include "settings.h"
#include "encoding.h"
#include "debug.h"


INIT_LOGGER(game, Duplicate);


Duplicate::Duplicate(const GameParams &iParams)
    : Game(iParams)
{
}


int Duplicate::play(const wstring &iCoord, const wstring &iWord)
{
    ASSERT(!hasPlayed(m_currPlayer), "Human player has already played");

    // Perform all the validity checks, and try to fill a round
    Round round;
    int res = checkPlayedWord(iCoord, iWord, round);
    if (res != 0 && Settings::Instance().getBool("duplicate.reject-invalid"))
    {
        return res;
    }

    // If we reach this point, either the move is valid and we can use the
    // "round" variable, or it is invalid but played nevertheless
    Player &currPlayer = *m_players[m_currPlayer];
    if (res == 0)
    {
        // Everything is OK, we can play the word
        recordPlayerMove(Move(round), currPlayer, true);
    }
    else
    {
        // Convert the invalid word for display
        const wdstring &dispWord = getDic().convertToDisplay(iWord);
        // Record the invalid move of the player
        recordPlayerMove(Move(dispWord, iCoord), currPlayer, true);
    }

    // Little hack to handle duplicate games with only AI players.
    // This will have no effect when there is at least one human player
    tryEndTurn();

    return 0;
}


void Duplicate::playAI(unsigned int p)
{
    ASSERT(p < getNPlayers(), "Wrong player number");
    ASSERT(!hasPlayed(p), "AI player has already played");

    AIPlayer *player = dynamic_cast<AIPlayer*>(m_players[p]);
    ASSERT(player != NULL, "AI requested for a human player");

    player->compute(getDic(), getBoard(), getHistory().beforeFirstRound());
    const Move &move = player->getMove();
    if (move.getType() == Move::CHANGE_LETTERS ||
        move.getType() == Move::PASS)
    {
        // The AI player must be buggy...
        ASSERT(false, "AI tried to cheat!");
    }

    recordPlayerMove(move, *player, false);
}


void Duplicate::start()
{
    ASSERT(getNPlayers(), "Cannot start a game without any player");

    // Arbitrary player, since they should all have the same rack
    m_currPlayer = 0;

    // Complete the racks
    try
    {
        // Reset the master move
        setMasterMove(Move());

        const PlayedRack &newRack =
            helperSetRackRandom(getHistory().getCurrentRack(), true, RACK_NEW);
        setGameAndPlayersRack(newRack);
    }
    catch (EndGameException &e)
    {
        endGame();
        return;
    }

    // Little hack to handle duplicate games with only AI players.
    // This will have no effect when there is at least one human player
    tryEndTurn();
}


bool Duplicate::isFinished() const
{
    return m_finished;
}


void Duplicate::tryEndTurn()
{
    for (unsigned int i = 0; i < getNPlayers(); i++)
    {
        if (m_players[i]->isHuman() && !m_hasPlayed[i])
        {
            // A human player has not played...
            m_currPlayer = i;
            // So we don't finish the turn
            return;
        }
    }

    // Now that all the human players have played,
    // make AI players play their turn
    // Some may have already played, in arbitration mode, if the future turns
    // were removed (because of the isHumanIndependent() behaviour)
    for (unsigned int i = 0; i < getNPlayers(); i++)
    {
        if (!m_players[i]->isHuman() && !m_hasPlayed[i])
        {
            playAI(i);
        }
    }

    // Next turn
    endTurn();
}


void Duplicate::recordPlayerMove(const Move &iMove, Player &ioPlayer, bool isForHuman)
{
    LOG_INFO("Player " << ioPlayer.getId() << " plays: " << lfw(iMove.toString()));
    bool isArbitration = getParams().getMode() == GameParams::kARBITRATION;
    Command *pCmd = new PlayerMoveCmd(ioPlayer, iMove, isArbitration);
    pCmd->setHumanIndependent(!isForHuman);
    accessNavigation().addAndExecute(pCmd);

    Command *pCmd2 = new MarkPlayedCmd(*this, ioPlayer.getId(), true);
    pCmd2->setHumanIndependent(!isForHuman);
    accessNavigation().addAndExecute(pCmd2);
}


struct MatchingPlayer : public unary_function<PlayerMoveCmd, bool>
{
    MatchingPlayer(unsigned iPlayerId) : m_playerId(iPlayerId) {}

    bool operator()(const PlayerMoveCmd &cmd)
    {
        return cmd.getPlayer().getId() == m_playerId;
    }

    const unsigned m_playerId;
};


void Duplicate::undoPlayerMove(Player &ioPlayer)
{
    ASSERT(hasPlayed(ioPlayer.getId()), "The player has no assigned move yet!");
    // There must be no NAEC in the current (i.e. last) turn.
    // If there was, it might not be such a big deal, though.
    ASSERT(!getNavigation().getTurns().back()->hasNonAutoExecCmd(),
           "Cannot undo a player move when there are some NAEC commands");

    // Find the PlayerMoveCmd we want to undo
    MatchingPlayer predicate(ioPlayer.getId());
    const PlayerMoveCmd *cmd =
        getNavigation().getCurrentTurn().findMatchingCmd<PlayerMoveCmd>(predicate);
    ASSERT(cmd != 0, "No matching PlayerMoveCmd found");

    // Undo the player move
    Command *copyCmd = new PlayerMoveCmd(*cmd);
    accessNavigation().addAndExecute(new UndoCmd(copyCmd));

    // OK, now flag the player as "not played". We can do it more directly...
    Command *pCmd = new MarkPlayedCmd(*this, ioPlayer.getId(), false);
    accessNavigation().addAndExecute(pCmd);
}


Player * Duplicate::findBestPlayer() const
{
    Player *bestPlayer = NULL;
    int bestScore = -1;
    BOOST_FOREACH(Player *player, m_players)
    {
        const Move &move = player->getLastMove();
        if (move.getType() == Move::VALID_ROUND &&
            move.getScore() > bestScore)
        {
            bestScore = move.getScore();
            bestPlayer = player;
        }
    }
    return bestPlayer;
}


void Duplicate::endTurn()
{
    static const unsigned int REF_PLAYER_ID = 0;

    // Define the master move if it is not already defined
    if (m_masterMove.getType() != Move::VALID_ROUND)
    {
        // The chosen implementation is to find the best move among the players' moves.
        // It is more user-friendly than forcing the best move when nobody found it.
        // If you want the best move instead, simply add an AI player to the game!

        // Find the player with the best score
        const Player *bestPlayer = findBestPlayer();
        if (bestPlayer != NULL)
        {
            setMasterMove(bestPlayer->getLastMove());
        }
        else
        {
            // If nobody played a valid round, we are forced to play a valid move.
            // So let's take the best one...
            BestResults results;
            // Take the first player's rack
            const Rack &rack =
                m_players[REF_PLAYER_ID]->getLastRack().getRack();
            results.search(getDic(), getBoard(), rack, getHistory().beforeFirstRound());
            if (results.size() == 0)
            {
                // This would be very bad luck that no move is possible...
                // It's probably not even possible, but let's be safe.
                throw EndGameException(_("No possible move"));
            }
            setMasterMove(Move(results.get(0)));
        }
    }

    // Handle solo bonus
    // First check whether there are enough players in the game for the
    // bonus to apply
    unsigned int minNbPlayers = Settings::Instance().getInt("duplicate.solo-players");
    // Find the player with the best score
    Player *bestPlayer = findBestPlayer();
    if (getNPlayers() >= minNbPlayers && bestPlayer != NULL)
    {
        int bestScore = bestPlayer->getLastMove().getScore();
        // Find whether other players than imax have the same score
        bool otherWithSameScore = false;
        BOOST_FOREACH(const Player *player, m_players)
        {
            if (player != bestPlayer &&
                player->getLastMove().getScore() >= bestScore &&
                player->getLastMove().getType() == Move::VALID_ROUND)
            {
                otherWithSameScore = true;
                break;
            }
        }
        if (!otherWithSameScore)
        {
            // Give the bonus to the player of the best move
            int bonus = Settings::Instance().getInt("duplicate.solo-value");
            bestPlayer->addPoints(bonus);
            // TODO: keep a trace of the solo, so the interface
            // can be aware of it...
        }
    }

    // Play the master word on the board
    // We assign it to player 0 arbitrarily (this is only used
    // to retrieve the rack, which is the same for all players...)
    Command *pCmd = new GameMoveCmd(*this, m_masterMove, REF_PLAYER_ID);
    accessNavigation().addAndExecute(pCmd);

    // Change the turn after doing all the game changes.
    // The navigation system expects auto-executable commands (like setting
    // the players racks) at the beginning of the turn, to work properly.
    accessNavigation().newTurn();

    // Leave the same reliquate to all players
    // This is required by the start() method which will be called to
    // start the next turn
    const PlayedRack& pld = getHistory().getCurrentRack();
    BOOST_FOREACH(Player *player, m_players)
    {
        Command *pCmd = new PlayerRackCmd(*player, pld);
        accessNavigation().addAndExecute(pCmd);
    }

    // Start next turn...
    start();
}


void Duplicate::endGame()
{
    LOG_INFO("End of the game");
    // No more master move
    setMasterMove(Move());
    m_finished = true;
}


void Duplicate::setPlayer(unsigned int p)
{
    ASSERT(p < getNPlayers(), "Wrong player number");

    // Forbid switching to an AI player
    if (!m_players[p]->isHuman())
        throw GameException(_("Cannot switch to a non-human player"));

    // Forbid switching back to a player who has already played
    if (hasPlayed(p))
        throw GameException(_("Cannot switch to a player who has already played"));

    m_currPlayer = p;
}


bool Duplicate::hasPlayed(unsigned int p) const
{
    ASSERT(p < getNPlayers(), "Wrong player number");

    map<unsigned int, bool>::const_iterator it = m_hasPlayed.find(p);
    return it != m_hasPlayed.end() && it->second;
}


void Duplicate::setPlayedFlag(unsigned int iPlayerId, bool iNewFlag)
{
    ASSERT(iPlayerId < getNPlayers(), "Wrong player number");

    m_hasPlayed[iPlayerId] = iNewFlag;
}


void Duplicate::innerSetMasterMove(const Move &iMove)
{
    m_masterMove = iMove;
}


void Duplicate::setMasterMove(const Move &iMove)
{
    ASSERT(iMove.getType() == Move::VALID_ROUND ||
           iMove.getType() == Move::NO_MOVE,
           "Invalid move type");

    // If this method is called several times for the same turn, it will
    // result in many MasterMoveCmd commands in the command stack.
    // This shouldn't be a problem though.
    LOG_DEBUG("Setting master move: " + lfw(iMove.toString()));
    Command *pCmd = new MasterMoveCmd(*this, iMove);
    accessNavigation().addAndExecute(pCmd);
}


void Duplicate::setGameAndPlayersRack(const PlayedRack &iRack)
{
    // Set the game rack
    Command *pCmd = new GameRackCmd(*this, iRack);
    accessNavigation().addAndExecute(pCmd);
    LOG_INFO("Setting players rack to '" + lfw(iRack.toString()) + "'");
    // All the players have the same rack
    BOOST_FOREACH(Player *player, m_players)
    {
        Command *pCmd = new PlayerRackCmd(*player, iRack);
        accessNavigation().addAndExecute(pCmd);
        // Nobody has played yet in this round
        Command *pCmd2 = new MarkPlayedCmd(*this, player->getId(), false);
        accessNavigation().addAndExecute(pCmd2);
    }
}


