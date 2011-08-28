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

#include <vector>
#include <fstream>
#include <cmath>
#include <boost/foreach.hpp>

#include "xml_writer.h"
#include "encoding.h"
#include "turn_cmd.h"
#include "game_params.h"
#include "game.h"
#include "player.h"
#include "ai_percent.h"
#include "game_exception.h"
#include "turn_cmd.h"
#include "game_move_cmd.h"
#include "player_rack_cmd.h"
#include "player_move_cmd.h"
#include "player_points_cmd.h"
#include "mark_played_cmd.h"

using namespace std;

INIT_LOGGER(game, XmlWriter);

static void addIndent(string &s)
{
    s += "    ";
}

static void removeIndent(string &s)
{
    if (s.size() >= 4)
        s.resize(s.size() - 4);
}

static string toUtf8(const wstring &s)
{
    return writeInUTF8(s, "Saving game");
}

static void writeMove(ostream &out, const Move &iMove,
                      const string &iTag, unsigned int iPlayerId)
{
    out << "<" << iTag << " playerid=\"" << iPlayerId << "\" type=\"";
    if (iMove.getType() == Move::VALID_ROUND)
    {
        const Round &round = iMove.getRound();
        out << "valid\" word=\"" << toUtf8(round.getWord())
            << "\" coord=\"" << toUtf8(round.getCoord().toString()) << "\" />";
    }
    else if (iMove.getType() == Move::INVALID_WORD)
    {
        out << "invalid\" word=\"" << toUtf8(iMove.getBadWord())
            << "\" coord=\"" << toUtf8(iMove.getBadCoord()) << "\" />";
    }
    else if (iMove.getType() == Move::CHANGE_LETTERS)
        out << "change\" letters=\"" << toUtf8(iMove.getChangedLetters()) << "\" />";
    else if (iMove.getType() == Move::PASS)
        out << "pass\" />";
    else
        throw SaveGameException("Unsupported move: " + lfw(iMove.toString()));
}

void XmlWriter::write(const Game &iGame, const string &iFileName)
{
    LOG_INFO("Saving game into '" << iFileName << "'");
    ofstream out(iFileName.c_str());
    if (!out.is_open())
        throw SaveGameException("Cannot open file for writing: '" + iFileName + "'");


    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;

    string indent = "";
    out << indent << "<EliotGame format=\"1\">" << endl;
    addIndent(indent);

    // ------------------------
    // Write the header
    out << indent << "<Game>" << endl;
    addIndent(indent);
    // Game type
    out << indent << "<Mode>";
    if (iGame.getMode() == Game::kDUPLICATE)
        out << "duplicate";
    else if (iGame.getMode() == Game::kFREEGAME)
        out << "freegame";
    else
        out << "training";
    out << "</Mode>" << endl;

    // Game variant
    if (iGame.getParams().hasVariant(GameParams::kJOKER_VARIANT))
        out << indent << "<Variant>bingo</Variant>" << endl;
    if (iGame.getParams().hasVariant(GameParams::kEXPLOSIVE_VARIANT))
        out << indent << "<Variant>explosive</Variant>" << endl;
    if (iGame.getParams().hasVariant(GameParams::k7AMONG8_VARIANT))
        out << indent << "<Variant>7among8</Variant>" << endl;

    // Players
    for (unsigned int i = 0; i < iGame.getNPlayers(); ++i)
    {
        const Player &player = iGame.getPlayer(i);
        out << indent << "<Player id=\"" << player.getId() + 1 << "\">" << endl;
        addIndent(indent);
        out << indent << "<Name>" << toUtf8(player.getName()) << "</Name>" << endl;
        out << indent << "<Type>" << (player.isHuman() ? "human" : "computer") << "</Type>" << endl;
        if (!player.isHuman())
        {
            const AIPercent *ai = dynamic_cast<const AIPercent *>(&player);
            if (ai == NULL)
                throw SaveGameException("Invalid player type for player " + i);
            out << indent << "<Level>" << lrint(ai->getPercent() * 100) << "</Level>" << endl;
        }
        removeIndent(indent);
        out << indent << "</Player>" << endl;
    }

    // Number of turns
    out << indent << "<Turns>"
        << iGame.getNavigation().getNbTurns() << "</Turns>" << endl;

    removeIndent(indent);
    out << indent << "</Game>" << endl;
    // End of the header
    // ------------------------

    // ------------------------
    // Write the game history
    out << indent << "<History>" << endl;
    addIndent(indent);

#if 0
    iGame.getNavigation().print();
#endif
    const vector<TurnCmd *> &turnCmdVect = iGame.getNavigation().getCommands();
    BOOST_FOREACH(const TurnCmd *turn, turnCmdVect)
    {
        if (turn->getCommands().empty() && turn == turnCmdVect.back())
            continue;
        out << indent << "<Turn>" << endl;
        addIndent(indent);
        BOOST_FOREACH(const Command *cmd, turn->getCommands())
        {
            if (dynamic_cast<const PlayerRackCmd*>(cmd))
            {
                const PlayerRackCmd *rackCmd = static_cast<const PlayerRackCmd*>(cmd);
                unsigned int id = rackCmd->getPlayer().getId() + 1;
                out << indent << "<PlayerRack playerid=\"" << id << "\">"
                    << toUtf8(rackCmd->getRack().toString())
                    << "</PlayerRack>" << endl;
            }
            else if (dynamic_cast<const PlayerPointsCmd*>(cmd))
            {
                const PlayerPointsCmd *pointsCmd = static_cast<const PlayerPointsCmd*>(cmd);
                unsigned int id = pointsCmd->getPlayer().getId() + 1;
                out << indent << "<PlayerPoints playerid=\"" << id << "\">"
                    << pointsCmd->getPoints() << "</PlayerPoints>" << endl;
            }
            else if (dynamic_cast<const PlayerMoveCmd*>(cmd))
            {
                const PlayerMoveCmd *moveCmd = static_cast<const PlayerMoveCmd*>(cmd);
                unsigned int id = moveCmd->getPlayer().getId() + 1;
                out << indent;
                writeMove(out, moveCmd->getMove(), "PlayerMove", id);
                out << endl;
            }
            else if (dynamic_cast<const GameMoveCmd*>(cmd))
            {
                const GameMoveCmd *moveCmd = static_cast<const GameMoveCmd*>(cmd);
                unsigned int id = moveCmd->getPlayerId() + 1;
                out << indent;
                writeMove(out, moveCmd->getMove(), "GameMove", id);
                out << endl;
            }
            else if (dynamic_cast<const MarkPlayedCmd*>(cmd))
            {
                // Ignore this command, as it is an implementation detail
            }
            else
            {
                LOG_ERROR("Unsupported command: " << lfw(cmd->toString()));
                // XXX
                //throw SaveGameException("Unsupported command: " + lfw(cmd->toString()));
            }
            // TODO
        }
        removeIndent(indent);
        out << indent << "</Turn>" << endl;
    }

    removeIndent(indent);
    out << indent << "</History>" << endl;
    // End of the game history
    // ------------------------

    out << "</EliotGame>" << endl;
}

