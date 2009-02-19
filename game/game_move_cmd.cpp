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

#include <sstream>

#include "game_move_cmd.h"
#include "player.h"
#include "game.h"
#include "rack.h"


GameMoveCmd::GameMoveCmd(Game &ioGame, const Move &iMove,
                         const PlayedRack &iMoveRack, unsigned int iPlayerId)
    : m_game(ioGame), m_move(iMove), m_moveRack(iMoveRack),
    m_playerId(iPlayerId)
{
}


void GameMoveCmd::doExecute()
{
    // Get the original rack from the player history
    const Rack &newRack = Move::ComputeRackForMove(m_moveRack, m_move);

    // History of the game
    History &history = m_game.accessHistory();
    history.setCurrentRack(m_moveRack);
    history.playMove(m_playerId, m_move, newRack);

    // Points
    m_game.addPoints(m_move.getScore());

    // For moves corresponding to a valid round, we have much more
    // work to do...
    if (m_move.getType() == Move::VALID_ROUND)
    {
        playRound();
    }
}


void GameMoveCmd::doUndo()
{
    // Undo playing the round on the board
    if (m_move.getType() == Move::VALID_ROUND)
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

    if (m_game.getVariant() == Game::kJOKER)
    {
        for (unsigned int i = 0; i < m_round.getWordLen(); i++)
        {
            if (m_round.isPlayedFromRack(i) && m_round.isJoker(i))
            {
                // Get the real bag
                Bag tmpBag(m_game.getDic());
                m_game.realBag(tmpBag);

                // Is the represented letter still available in the bag?
                // XXX: this way to get the represented letter sucks...
                Tile t(towupper(m_round.getTile(i).toChar()));
                if (tmpBag.in(t))
                {
                    bag.replaceTile(Tile::Joker());
                    bag.takeTile(t);
                    m_round.setTile(i, t);
                    // FIXME: This shouldn't be necessary, this is only
                    // needed because of the stupid way of handling jokers in
                    // rounds
                    m_round.setJoker(i, false);
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

