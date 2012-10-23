/*******************************************************************
 * Eliot
 * Copyright (C) 2009-2012 Olivier Teulière
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
#include <boost/format.hpp>

#include "config.h"
#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
#else
#   define _(String) String
#endif

#include "xml_writer.h"
#include "encoding.h"
#include "turn.h"
#include "game_params.h"
#include "game.h"
#include "player.h"
#include "ai_percent.h"
#include "game_exception.h"
#include "turn.h"
#include "game_rack_cmd.h"
#include "game_move_cmd.h"
#include "player_rack_cmd.h"
#include "player_move_cmd.h"
#include "player_event_cmd.h"
#include "master_move_cmd.h"
#include "dic.h"
#include "header.h"

// Current version of our save game format. Bump it when it becomes
// incompatible (and keep it in sync with xml_reader.cpp)
#define CURRENT_XML_VERSION "2"

#define FMT1(s, a1) (boost::format(s) % (a1)).str()
#define FMT2(s, a1, a2) (boost::format(s) % (a1) % (a2)).str()


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
                      const string &iTag, int iPlayerId)
{
    out << "<" << iTag;
    if (iPlayerId != -1)
        out << " playerId=\"" << iPlayerId << "\"";
    out << " points=\"" << iMove.getScore() << "\" type=\"";
    if (iMove.isValid())
    {
        const Round &round = iMove.getRound();
        out << "valid\" word=\"" << toUtf8(round.getWord())
            << "\" coord=\"" << toUtf8(round.getCoord().toString()) << "\" />";
    }
    else if (iMove.isInvalid())
    {
        out << "invalid\" word=\"" << toUtf8(iMove.getBadWord())
            << "\" coord=\"" << toUtf8(iMove.getBadCoord()) << "\" />";
    }
    else if (iMove.isChangeLetters())
        out << "change\" letters=\"" << toUtf8(iMove.getChangedLetters()) << "\" />";
    else if (iMove.isPass())
        out << "pass\" />";
    else if (iMove.isNull())
        out << "none\" />";
    else
        throw SaveGameException(FMT1(_("Unsupported move: %1%"), lfw(iMove.toString())));
}


void XmlWriter::write(const Game &iGame, const string &iFileName)
{
    LOG_INFO("Saving game into '" << iFileName << "'");
    ofstream out(iFileName.c_str());
    if (!out.is_open())
        throw SaveGameException(FMT1(_("Cannot open file for writing: '%1%'"), iFileName));


    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;

    string indent = "";
    out << indent << "<EliotGame format=\"" << CURRENT_XML_VERSION << "\">" << endl;
    addIndent(indent);

    // ------------------------
    // Write the dictionary information
    out << indent << "<Dictionary>" << endl;
    addIndent(indent);
    const Header &header = iGame.getDic().getHeader();
    out << indent << "<Name>" << toUtf8(header.getName()) << "</Name>" << endl;
    out << indent << "<Type>";
    if (header.getType() == Header::kDAWG)
        out << "dawg";
    else if (header.getType() == Header::kGADDAG)
        out << "gaddag";
    else
        throw SaveGameException(_("Invalid dictionary type"));
    out << "</Type>" << endl;
    // Retrieve the dictionary letters, ans separate them with spaces
    wstring lettersWithSpaces = header.getLetters();
    for (size_t i = lettersWithSpaces.size() - 1; i > 0; --i)
        lettersWithSpaces.insert(i, 1, L' ');
    // Convert to a display string
    const wstring &displayLetters =
        iGame.getDic().convertToDisplay(lettersWithSpaces);
    out << indent << "<Letters>" << toUtf8(displayLetters) << "</Letters>" << endl;
    out << indent << "<WordNb>" << header.getNbWords() << "</WordNb>" << endl;
    removeIndent(indent);
    out << indent << "</Dictionary>" << endl;
    // End of dictionary information
    // ------------------------

    // ------------------------
    // Write the game header
    out << indent << "<Game>" << endl;
    addIndent(indent);
    // Game type
    out << indent << "<Mode>";
    if (iGame.getMode() == GameParams::kDUPLICATE)
        out << "duplicate";
    else if (iGame.getMode() == GameParams::kFREEGAME)
        out << "freegame";
    else if (iGame.getMode() == GameParams::kARBITRATION)
        out << "arbitration";
    else
        out << "training";
    out << "</Mode>" << endl;

    // Game variant
    if (iGame.getParams().hasVariant(GameParams::kJOKER))
        out << indent << "<Variant>bingo</Variant>" << endl;
    if (iGame.getParams().hasVariant(GameParams::kEXPLOSIVE))
        out << indent << "<Variant>explosive</Variant>" << endl;
    if (iGame.getParams().hasVariant(GameParams::k7AMONG8))
        out << indent << "<Variant>7among8</Variant>" << endl;

    // Players
    for (unsigned int i = 0; i < iGame.getNPlayers(); ++i)
    {
        const Player &player = iGame.getPlayer(i);
        out << indent << "<Player id=\"" << player.getId() << "\">" << endl;
        addIndent(indent);
        out << indent << "<Name>" << toUtf8(player.getName()) << "</Name>" << endl;
        out << indent << "<Type>" << (player.isHuman() ? "human" : "computer") << "</Type>" << endl;
        if (!player.isHuman())
        {
            const AIPercent *ai = dynamic_cast<const AIPercent *>(&player);
            if (ai == NULL)
                throw SaveGameException(FMT1(_("Invalid player type for player %1%"), i));
            out << indent << "<Level>" << lrint(ai->getPercent() * 100) << "</Level>" << endl;
        }
        out << indent << "<TableNb>" << player.getTableNb() << "</TableNb>" << endl;
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
    const vector<Turn *> &turnVect = iGame.getNavigation().getTurns();
    BOOST_FOREACH(const Turn *turn, turnVect)
    {
        if (turn->getCommands().empty() && turn == turnVect.back())
            continue;
        out << indent << "<Turn>" << endl;
        addIndent(indent);
        BOOST_FOREACH(const Command *cmd, turn->getCommands())
        {
            if (dynamic_cast<const GameRackCmd*>(cmd))
            {
                const GameRackCmd *rackCmd = static_cast<const GameRackCmd*>(cmd);
                out << indent << "<GameRack>"
                    << toUtf8(rackCmd->getRack().toString())
                    << "</GameRack>" << endl;
            }
            else if (dynamic_cast<const PlayerRackCmd*>(cmd))
            {
                const PlayerRackCmd *rackCmd = static_cast<const PlayerRackCmd*>(cmd);
                unsigned int id = rackCmd->getPlayer().getId();
                out << indent << "<PlayerRack playerId=\"" << id << "\">"
                    << toUtf8(rackCmd->getRack().toString())
                    << "</PlayerRack>" << endl;
            }
            else if (dynamic_cast<const PlayerMoveCmd*>(cmd))
            {
                const PlayerMoveCmd *moveCmd = static_cast<const PlayerMoveCmd*>(cmd);
                unsigned int id = moveCmd->getPlayer().getId();
                out << indent;
                writeMove(out, moveCmd->getMove(), "PlayerMove", id);
                out << endl;
            }
            else if (dynamic_cast<const GameMoveCmd*>(cmd))
            {
                const GameMoveCmd *moveCmd = static_cast<const GameMoveCmd*>(cmd);
                unsigned int id = moveCmd->getPlayerId();
                out << indent;
                writeMove(out, moveCmd->getMove(), "GameMove", id);
                out << endl;
            }
            else if (dynamic_cast<const MasterMoveCmd*>(cmd))
            {
                const MasterMoveCmd *moveCmd = static_cast<const MasterMoveCmd*>(cmd);
                out << indent;
                writeMove(out, moveCmd->getMove(), "MasterMove", -1);
                out << endl;

            }
            else if (dynamic_cast<const PlayerEventCmd*>(cmd))
            {
                const PlayerEventCmd *eventCmd = static_cast<const PlayerEventCmd*>(cmd);
                unsigned int id = eventCmd->getPlayer().getId();
                int value = eventCmd->getPoints();
                // Warnings
                if (eventCmd->getEventType() == PlayerEventCmd::WARNING)
                {
                    out << indent << "<Warning playerId=\"" << id << "\" />" << endl;
                }
                // Penalties
                else if (eventCmd->getEventType() == PlayerEventCmd::PENALTY)
                {
                    out << indent << "<Penalty playerId=\"" << id
                        << "\" points=\"" << value << "\" />" << endl;
                }
                // Solos
                else if (eventCmd->getEventType() == PlayerEventCmd::SOLO)
                {
                    out << indent << "<Solo playerId=\"" << id
                        << "\" points=\"" << value << "\" />" << endl;
                }
                // End game bonuses (freegame mode)
                else if (eventCmd->getEventType() == PlayerEventCmd::END_GAME)
                {
                    out << indent << "<EndGame playerId=\"" << id
                        << "\" points=\"" << value << "\" />" << endl;
                }
                else
                {
                    LOG_ERROR("Unknown event type: " << eventCmd->getEventType());
                }
            }
            else
            {
                LOG_ERROR("Unsupported command: " << lfw(cmd->toString()));
                out << indent << "<!-- FIXME: Unsupported command: " << lfw(cmd->toString()) << " -->" << endl;
                // XXX
                //throw SaveGameException(FMT1(_("Unsupported command: %1%"), lfw(cmd->toString())));
            }
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

