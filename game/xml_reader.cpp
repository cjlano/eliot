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

#include <fstream>
#include <SAX/XMLReader.hpp>
#include <SAX/InputSource.hpp>

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
#include "game_move_cmd.h"
#include "player_rack_cmd.h"
#include "player_move_cmd.h"
#include "player_points_cmd.h"
#include "navigation.h"

using namespace std;

INIT_LOGGER(game, XmlReader);


Game * XmlReader::read(const string &iFileName, const Dictionary &iDic)
{
    // Try to load the old format first
    LOG_INFO("Parsing savegame '" << iFileName << "'");

    try
    {
        LOG_DEBUG("Trying old format");
        FILE *fin = fopen(iFileName.c_str(), "r");
        if (fin != NULL)
        {
            Game *game = Game::load(fin, iDic);
            fclose(fin);

            if (game != NULL)
            {
                LOG_INFO("Savegame parsed successfully");
                return game;
            }
        }
    }
    catch (const GameException &e)
    {
        // Ignore the exception
        LOG_DEBUG("This doesn't look like the old format");
    }

    LOG_DEBUG("Trying XML format");

    ifstream is(iFileName.c_str());
    if (!is.is_open())
        throw LoadGameException("Cannot open file '" + iFileName + "'");

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
        throw LoadGameException("Invalid string to int conversion: empty string received");
    return atoi(str.c_str());
}


static Player & getPlayer(map<string, Player*> &players, const string &id)
{
    if (players.find(id) == players.end())
        throw LoadGameException("Invalid player ID: " + id);
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
            throw LoadGameException("Invalid move marked as valid: " +
                                    attr["word"] + " (" + attr["coord"] + ")");
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
    else
        throw LoadGameException("Invalid move type: " + type);
}


Game * XmlReader::getGame()
{
    // TODO
    return m_game;
}


void XmlReader::startElement(const string& namespaceURI,
                             const string& localName,
                             const string& qName,
                             const Arabica::SAX::Attributes<string>& atts)
{
    (void) namespaceURI;
    (void) qName;
    LOG_DEBUG("Start Element: " << (localName.empty() ? qName : namespaceURI + ":" + localName));

    m_data.clear();
    const string &tag = localName;
    if (tag == "Player")
    {
        m_context = "Player";
        for (int i = 0; i < atts.getLength(); ++i)
        {
            m_attributes[atts.getLocalName(i)] = atts.getValue(i);
        }
    }
    else if (tag == "PlayerRack" || tag == "PlayerMove" || tag == "GameMove")
    {
        m_attributes.clear();
        for (int i = 0; i < atts.getLength(); ++i)
        {
            m_attributes[atts.getLocalName(i)] = atts.getValue(i);
        }
    }
}


void XmlReader::endElement(const string& namespaceURI,
                           const string& localName,
                           const string&)
{
    (void) namespaceURI;
    LOG_DEBUG("endElement: " << namespaceURI << ":" << localName << "(" << m_data << ")");

    const string &tag = localName;
    if (tag == "Mode")
    {
        // The game should not be created yet
        if (m_game != NULL)
            throw LoadGameException("The 'Mode' tag should be the first one to be closed");

        // Differ game creation until after we have read the variant
        m_attributes["mode"] = m_data;
        return;
    }

    if (tag == "Variant")
    {
        // The game should not be created yet
        if (m_game != NULL)
            throw LoadGameException("The 'Variant' tag should be right after the 'Mode' one");

        m_attributes["variant"] = m_data;
        return;
    }

    // Create the game
    if (m_game == NULL)
    {
        const string &variantStr = m_attributes["variant"];
        GameParams::GameVariant variant = GameParams::kNONE;
        if (variantStr == "bingo")
            variant = GameParams::kJOKER;
        else if (variantStr == "explosive")
            variant = GameParams::kEXPLOSIVE;
        else if (variantStr == "7among8")
            variant = GameParams::k7AMONG8;
        else if (variantStr != "")
            throw LoadGameException("Invalid game variant: " + variantStr);

        const string &mode = m_attributes["mode"];
        if (mode == "duplicate")
            m_game = GameFactory::Instance()->createDuplicate(m_dic, GameParams(variant));
        else if (mode == "freegame")
            m_game = GameFactory::Instance()->createFreeGame(m_dic, GameParams(variant));
        else if (mode == "training")
            m_game = GameFactory::Instance()->createTraining(m_dic, GameParams(variant));
        else
            throw LoadGameException("Invalid game mode: " + mode);
    }

    else if (m_context == "Player")
    {
        if (tag == "Name")
            m_attributes["name"] = m_data;
        else if (tag == "Type")
            m_attributes["type"] = m_data;
        else if (tag == "Level")
            m_attributes["level"] = m_data;
        else if (tag == "Player")
        {
            if (m_players.find(m_attributes["id"]) != m_players.end())
                throw LoadGameException("A player ID must be unique: " + m_attributes["id"]);
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
                throw LoadGameException("Invalid player type: " + m_attributes["type"]);
            m_players[m_attributes["id"]] = p;

            // Set the name
            p->setName(fromUtf8(m_attributes["name"]));

            m_game->addPlayer(p);

            m_context = "";
        }
    }

    else if (tag == "Turn")
    {
        m_game->accessNavigation().newTurn();
    }

    else if (tag == "PlayerRack")
    {
        // Build a rack for the correct player
        const wstring &rackStr = m_dic.convertFromInput(fromUtf8(m_data));
        PlayedRack pldrack;
        if (!m_dic.validateLetters(rackStr, L"-+"))
        {
            throw LoadGameException("Rack invalid for the current dictionary: " + m_data);
        }
        pldrack.setManual(rackStr);
        LOG_DEBUG("loaded rack: " << lfw(pldrack.toString()));

        Player &p = getPlayer(m_players, m_attributes["playerid"]);
        PlayerRackCmd *cmd = new PlayerRackCmd(p, pldrack);
        m_game->accessNavigation().addAndExecute(cmd);
        LOG_DEBUG("rack: " << lfw(pldrack.toString()));
    }

    else if (tag == "PlayerMove")
    {
        const Move &move = buildMove(*m_game, m_attributes, /*XXX:true*/false);
        Player &p = getPlayer(m_players, m_attributes["playerid"]);
        PlayerMoveCmd *cmd = new PlayerMoveCmd(p, move);
        m_game->accessNavigation().addAndExecute(cmd);
    }

    else if (tag == "GameMove")
    {
        const Move &move = buildMove(*m_game, m_attributes, false);
        Player &p = getPlayer(m_players, m_attributes["playerid"]);
        GameMoveCmd *cmd = new GameMoveCmd(*m_game, move, p.getId());
        m_game->accessNavigation().addAndExecute(cmd);
    }

}


void XmlReader::characters(const string& ch)
{
    m_data += ch;
    LOG_DEBUG("Characters: " << ch);
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

