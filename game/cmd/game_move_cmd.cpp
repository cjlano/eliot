/*******************************************************************
 * Eliot
 * Copyright (C) 2008-2012 Olivier Teulière
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

#include <sstream>

#include "cmd/game_move_cmd.h"
#include "player.h"
#include "game_params.h"
#include "game.h"
#include "rack.h"
#include "turn_data.h"


INIT_LOGGER(game, GameMoveCmd);


GameMoveCmd::GameMoveCmd(Game &ioGame, const Move &iMove,
                         unsigned int iPlayerId)
    : m_game(ioGame), m_move(iMove),
    m_moveRack(ioGame.getHistory().getCurrentRack()),
    m_playerId(iPlayerId)
{
    setAutoExecutable(false);
}


void GameMoveCmd::doExecute()
{
    // Get the original rack from the player history
    const PlayedRack &newRack = Move::ComputeRackForMove(m_moveRack, m_move);

    // History of the game
    History &history = m_game.accessHistory();
    history.playMove(m_playerId, m_move, newRack);

    // Points
    m_game.addPoints(m_move.getScore());

    // For moves corresponding to a valid round, we have much more
    // work to do...
    if (m_move.isValid())
    {
        playRound();
    }
}


void GameMoveCmd::doUndo()
{
    // Undo playing the round on the board
    if (m_move.isValid())
    {
        unplayRound();
    }

    // Points
    m_game.addPoints(- m_move.getScore());

    // History
    m_game.accessHistory().removeLastTurn();
}


void GameMoveCmd::playRound()
{
    // Copy the round, because we may need to modify it (case of
    // the joker games). It will also be convenient for the unplayRound()
    // method.
    m_round = m_move.getRound();

    // Update the bag
    // We remove tiles from the bag only when they are played
    // on the board. When going back in the game, we must only
    // replace played tiles.
    // We test a rack when it is set but tiles are left in the bag.
    Bag & bag = m_game.accessBag();
    for (unsigned int i = 0; i < m_round.getWordLen(); i++)
    {
        if (m_round.isPlayedFromRack(i))
        {
            if (m_round.isJoker(i))
            {
                bag.takeTile(Tile::Joker());
            }
            else
            {
                bag.takeTile(m_round.getTile(i));
            }
        }
    }

    if (m_game.getParams().hasVariant(GameParams::kJOKER))
    {
        for (unsigned int i = 0; i < m_round.getWordLen(); i++)
        {
            if (m_round.isPlayedFromRack(i) && m_round.isJoker(i))
            {
                // Is the represented letter still available in the bag?
                const Tile &t = m_round.getTile(i).toUpper();
                if (bag.in(t))
                {
                    bag.replaceTile(Tile::Joker());
                    bag.takeTile(t);
                    m_round.setTile(i, t);
                }

                // In a joker game we should have only 1 joker in the rack
                break;
            }
        }
    }

    // Update the board
    m_game.accessBoard().addRound(m_game.getDic(), m_round);
}


void GameMoveCmd::unplayRound()
{
    // Update the board
    m_game.accessBoard().removeRound(m_game.getDic(), m_round);

    // Update the bag
    // We remove tiles from the bag only when they are played
    // on the board. When going back in the game, we must only
    // replace played tiles.
    // We test a rack when it is set but tiles are left in the bag.
    Bag & bag = m_game.accessBag();
    for (unsigned int i = 0; i < m_round.getWordLen(); i++)
    {
        if (m_round.isPlayedFromRack(i))
        {
            if (m_round.isJoker(i))
            {
                bag.replaceTile(Tile::Joker());
            }
            else
            {
                bag.replaceTile(m_round.getTile(i));
            }
        }
    }
}


wstring GameMoveCmd::toString() const
{
    wostringstream oss;
    oss << L"GameMoveCmd (move: " << m_move.toString() << L", rack: "
        << m_moveRack.toString() << L")";
    return oss.str();
}

