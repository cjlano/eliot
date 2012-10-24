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

#include <fstream>
#include <algorithm>
#include <boost/format.hpp>
#include <SAX/XMLReader.hpp>
#include <SAX/InputSource.hpp>

// Remove spurious defines from libarabica, to avoid compilation warnings.
// Libarabica should not export them via public headers...
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION

#include "config.h"
#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
#else
#   define _(String) String
#endif

#include "xml_reader.h"
#include "dic.h"
#include "game_exception.h"
#include "game_params.h"
#include "game_factory.h"
#include "training.h"
#include "duplicate.h"
#include "freegame.h"
#include "player.h"
#include "ai_percent.h"
#include "encoding.h"
#include "game_rack_cmd.h"
#include "game_move_cmd.h"
#include "player_rack_cmd.h"
#include "player_move_cmd.h"
#include "player_event_cmd.h"
#include "master_move_cmd.h"
#include "navigation.h"
#include "header.h"

// Current version of our save game format. Bump it when it becomes
// incompatible (and keep it in sync with xml_writer.cpp)
#define CURRENT_XML_VERSION "2"

#define FMT1(s, a1) (boost::format(s) % (a1)).str()
#define FMT2(s, a1, a2) (boost::format(s) % (a1) % (a2)).str()


using namespace std;

INIT_LOGGER(game, XmlReader);


Game * XmlReader::read(const string &iFileName, const Dictionary &iDic)
{
    LOG_INFO("Parsing savegame '" << iFileName << "'");

    ifstream is(iFileName.c_str());
    if (!is.is_open())
        throw LoadGameException(FMT1(_("Cannot open file '%1%'"), iFileName));

    XmlReader handler(iDic);

    // Set up of the parser
    Arabica::SAX::XMLReader<std::string> parser;
    parser.setContentHandler(handler);
    parser.setErrorHandler(handler);

    // Parsing
    Arabica::SAX::InputSource<std::string> source(is);
    parser.parse(source);

    Game *game = handler.getGame();
    if (game == NULL)
        throw LoadGameException(handler.errorMessage);

    LOG_INFO("Savegame parsed successfully");
    return game;
}


static wstring fromUtf8(const string &str)
{
    return readFromUTF8(str, "Loading game");
}


static int toInt(const string &str)
{
    if (str.empty())
        throw LoadGameException(_("Invalid string to int conversion: empty string received"));
    return atoi(str.c_str());
}


static Player & getPlayer(map<string, Player*> &players,
                          const string &id, const string &iTag)
{
    if (players.find(id) == players.end())
        throw LoadGameException(FMT2(_("Invalid player ID: %1% (processing tag '%2%')"), id, iTag));
    return *players[id];
}


static Move buildMove(const Game &iGame, map<string, string> &attr,
                      bool checkRack)
{
    // Build the Move object
    string type = attr["type"];
    if (type == "valid")
    {
        wstring word = iGame.getDic().convertFromInput(fromUtf8(attr["word"]));
        Round round;
        int res = iGame.checkPlayedWord(fromUtf8(attr["coord"]),
                                        word, round, checkRack);
        if (res != 0)
        {
            throw LoadGameException(FMT2(_("Invalid move marked as valid: %1% (%2%)"),
                                         attr["word"], attr["coord"]));
        }
        return Move(round);
    }
    else if (type == "invalid")
    {
        return Move(fromUtf8(attr["word"]),
                    fromUtf8(attr["coord"]));
    }
    else if (type == "change")
    {
        return Move(fromUtf8(attr["letters"]));
    }
    else if (type == "pass")
    {
        return Move(L"");
    }
    else if (type == "none")
    {
        return Move();
    }
    else
        throw LoadGameException(FMT1(_("Invalid move type: %1%"), type));
}


Game * XmlReader::getGame()
{
    return m_game;
}


void XmlReader::startElement(const string& namespaceURI,
                             const string& localName,
                             const string& qName,
                             const Arabica::SAX::Attributes<string>& atts)
{
    (void) namespaceURI;
    (void) qName;
    LOG_TRACE("Start Element: " << (localName.empty() ? qName : namespaceURI + ":" + localName));

    m_data.clear();
    const string &tag = localName;
    if (tag == "EliotGame")
    {
        // Make sure that we are loading the correct XML format
        for (int i = 0; i < atts.getLength(); ++i)
        {
            if (atts.getLocalName(i) == "format" &&
                atts.getValue(i) != CURRENT_XML_VERSION)
            {
                LOG_ERROR("Incompatible save game format: current="
                          << CURRENT_XML_VERSION
                          << " savegame=" << atts.getValue(i));
                throw LoadGameException(_("This saved game is not compatible with the current version of Eliot."));
            }
        }
    }
    else if (tag == "Dictionary")
    {
        m_context = "Dictionary";
    }
    else if (tag == "Player")
    {
        m_context = "Player";
        for (int i = 0; i < atts.getLength(); ++i)
        {
            m_attributes[atts.getLocalName(i)] = atts.getValue(i);
        }
    }
    else if (tag == "GameRack" || tag == "PlayerRack" ||
             tag == "PlayerMove" || tag == "GameMove" || tag == "MasterMove" ||
             tag == "Warning" || tag == "Penalty" || tag == "Solo" || tag == "EndGame")
    {
        m_attributes.clear();
        for (int i = 0; i < atts.getLength(); ++i)
        {
            m_attributes[atts.getLocalName(i)] = atts.getValue(i);
        }
    }
    else if (tag == "Turn")
    {
        if (m_firstTurn)
            m_firstTurn = false;
        else
            m_game->accessNavigation().newTurn();
    }
    // For backwards compatibility ("playerid" was used in saved games
    // before release 2.1, with XML version 2)
    if (m_attributes["playerid"] != "")
        m_attributes["playerId"] = m_attributes["playerid"];
}


void XmlReader::endElement(const string& namespaceURI,
                           const string& localName,
                           const string&)
{
    (void) namespaceURI;
    LOG_TRACE("endElement: " << namespaceURI << ":" << localName << "(" << m_data << ")");

    const string &tag = localName;

    // Dictionary section
    if (m_context == "Dictionary")
    {
        if (tag == "Letters")
        {
            const wdstring & displayLetters = m_dic.convertToDisplay(m_dic.getHeader().getLetters());
            // Remove spaces
            string::iterator it;
            it = remove(m_data.begin(), m_data.end(), L' ');
            m_data.erase(it, m_data.end());
            // Compare
            if (displayLetters != fromUtf8(m_data))
                throw LoadGameException(_("The current dictionary is different from the one used in the saved game"));
        }
        else if (tag == "WordNb")
        {
            if (m_dic.getHeader().getNbWords() != (unsigned)toInt(m_data))
                throw LoadGameException(_("The current dictionary is different from the one used in the saved game"));
        }
        else if (tag == "Dictionary")
            m_context = "";
        return;
    }

    if (tag == "Mode")
    {
        // The game should not be created yet
        if (m_game != NULL)
            throw LoadGameException(_("The 'Mode' tag should be the first one to be closed"));

        // Differ game creation until after we have read the variant
        if (m_data == "duplicate")
            m_params.setMode(GameParams::kDUPLICATE);
        else if (m_data == "freegame")
            m_params.setMode(GameParams::kFREEGAME);
        else if (m_data == "training")
            m_params.setMode(GameParams::kTRAINING);
        else if (m_data == "arbitration")
            m_params.setMode(GameParams::kARBITRATION);
        else
            throw GameException("Invalid game mode: " + m_data);
        return;
    }

    if (tag == "Variant")
    {
        // The game should not be created yet
        if (m_game != NULL)
            throw LoadGameException(_("The 'Variant' tag should be right after the 'Mode' one"));

        if (m_data == "bingo")
            m_params.addVariant(GameParams::kJOKER);
        else if (m_data == "explosive")
            m_params.addVariant(GameParams::kEXPLOSIVE);
        else if (m_data == "7among8")
            m_params.addVariant(GameParams::k7AMONG8);
        else if (m_data != "")
            throw LoadGameException(FMT1(_("Invalid game variant: %1%"), m_data));
        return;
    }

    // Create the game
    if (m_game == NULL)
    {
        m_game = GameFactory::Instance()->createGame(m_params);
    }

    if (m_context == "Player")
    {
        if (tag == "Name")
            m_attributes["name"] = m_data;
        else if (tag == "Type")
            m_attributes["type"] = m_data;
        else if (tag == "Level")
            m_attributes["level"] = m_data;
        else if (tag == "TableNb")
            m_attributes["tablenb"] = m_data;
        else if (tag == "Player")
        {
            if (m_players.find(m_attributes["id"]) != m_players.end())
                throw LoadGameException(FMT1(_("A player ID must be unique: %1%"), m_attributes["id"]));
            // Create the player
            Player *p;
            if (m_attributes["type"] == "human")
                p = new HumanPlayer();
            else if (m_attributes["type"] == "computer")
            {
                int level = toInt(m_attributes["level"]);
                p = new AIPercent(0.01 * level);
            }
            else
                throw LoadGameException(FMT1(_("Invalid player type: %1%"), m_attributes["type"]));
            m_players[m_attributes["id"]] = p;

            // Set the name
            p->setName(fromUtf8(m_attributes["name"]));
            // Ste the table number
            if (m_attributes["tablenb"] != "")
                p->setTableNb(toInt(m_attributes["tablenb"]));

            m_game->addPlayer(p);

            m_context = "";
        }
    }

    else if (tag == "GameRack")
    {
        // Build a rack for the correct player
        const wstring &rackStr = m_dic.convertFromInput(fromUtf8(m_data));
        PlayedRack pldrack;
        if (!m_dic.validateLetters(rackStr, L"-+"))
        {
            throw LoadGameException(FMT1(_("Rack invalid for the current dictionary: %1%"), m_data));
        }
        pldrack.setManual(rackStr);
        LOG_DEBUG("loaded rack: " << lfw(pldrack.toString()));

        GameRackCmd *cmd = new GameRackCmd(*m_game, pldrack);
        m_game->accessNavigation().addAndExecute(cmd);
        LOG_DEBUG("rack: " << lfw(pldrack.toString()));
    }

    else if (tag == "PlayerRack")
    {
        // Build a rack for the correct player
        const wstring &rackStr = m_dic.convertFromInput(fromUtf8(m_data));
        PlayedRack pldrack;
        if (!m_dic.validateLetters(rackStr, L"-+"))
        {
            throw LoadGameException(FMT1(_("Rack invalid for the current dictionary: %1%"), m_data));
        }
        pldrack.setManual(rackStr);
        LOG_DEBUG("loaded rack: " << lfw(pldrack.toString()));

        Player &p = getPlayer(m_players, m_attributes["playerId"], tag);
        PlayerRackCmd *cmd = new PlayerRackCmd(p, pldrack);
        m_game->accessNavigation().addAndExecute(cmd);
        LOG_DEBUG("rack: " << lfw(pldrack.toString()));
    }

    else if (tag == "MasterMove")
    {
        const Move &move = buildMove(*m_game, m_attributes, false);
        Duplicate *duplicateGame = dynamic_cast<Duplicate*>(m_game);
        if (duplicateGame == NULL)
        {
            throw LoadGameException(_("The 'MasterMove' tag should only be present for duplicate games"));
        }
        MasterMoveCmd *cmd = new MasterMoveCmd(*duplicateGame, move);
        m_game->accessNavigation().addAndExecute(cmd);
    }

    else if (tag == "PlayerMove")
    {
        // FIXME: this is game-related logic. It should not be done here.
        bool isArbitrationGame = m_game->getParams().getMode() == GameParams::kARBITRATION;

        const Move &move = buildMove(*m_game, m_attributes, /*XXX:true*/false);
        Player &p = getPlayer(m_players, m_attributes["playerId"], tag);
        PlayerMoveCmd *cmd = new PlayerMoveCmd(p, move, isArbitrationGame);
        m_game->accessNavigation().addAndExecute(cmd);
    }

    else if (tag == "GameMove")
    {
        const Move &move = buildMove(*m_game, m_attributes, false);
        Player &p = getPlayer(m_players, m_attributes["playerId"], tag);
        GameMoveCmd *cmd = new GameMoveCmd(*m_game, move, p.getId());
        m_game->accessNavigation().addAndExecute(cmd);
    }

    else if (tag == "Warning")
    {
        Player &p = getPlayer(m_players, m_attributes["playerId"], tag);
        PlayerEventCmd *cmd = new PlayerEventCmd(p, PlayerEventCmd::WARNING);
        m_game->accessNavigation().addAndExecute(cmd);
    }

    else if (tag == "Penalty")
    {
        Player &p = getPlayer(m_players, m_attributes["playerId"], tag);
        int points = toInt(m_attributes["points"]);
        LOG_ERROR("points=" << points);
        PlayerEventCmd *cmd = new PlayerEventCmd(p, PlayerEventCmd::PENALTY, points);
        m_game->accessNavigation().addAndExecute(cmd);
    }

    else if (tag == "Solo")
    {
        Player &p = getPlayer(m_players, m_attributes["playerId"], tag);
        int points = toInt(m_attributes["points"]);
        PlayerEventCmd *cmd = new PlayerEventCmd(p, PlayerEventCmd::SOLO, points);
        m_game->accessNavigation().addAndExecute(cmd);
    }

    else if (tag == "EndGame")
    {
        Player &p = getPlayer(m_players, m_attributes["playerId"], tag);
        int points = toInt(m_attributes["points"]);
        PlayerEventCmd *cmd = new PlayerEventCmd(p, PlayerEventCmd::END_GAME, points);
        m_game->accessNavigation().addAndExecute(cmd);
    }

}


void XmlReader::characters(const string& ch)
{
    m_data += ch;
    LOG_TRACE("Characters: " << ch);
}


void XmlReader::warning(const Arabica::SAX::SAXParseException<string>& exception)
{
    errorMessage = string("warning: ") + exception.what();
    LOG_WARN(errorMessage);
    //throw LoadGameException(string("warning: ") + exception.what());
}


void XmlReader::error(const Arabica::SAX::SAXParseException<string>& exception)
{
    errorMessage = string("error: ") + exception.what();
    LOG_ERROR(errorMessage);
    //throw LoadGameException(string("error: ") + exception.what());
}


void XmlReader::fatalError(const Arabica::SAX::SAXParseException<string>& exception)
{
    errorMessage = string("fatal error: ") + exception.what();
    LOG_FATAL(errorMessage);
    //throw LoadGameException(string("fatal error: ") + exception.what());
}

