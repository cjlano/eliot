/*****************************************************************************
 * Copyright (C) 2009 Eliot
 * Authors: Olivier Teuliere  <ipkiss@via.ecp.fr>
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

#ifndef XML_READER_H_
#define XML_READER_H_

#include <map>

// Remove spurious defines, to avoid compilation warnings. They are defined
// in config.h (which is normal) and in libarabica (which is wrong:
// libarabica should not export them via public headers).
// We do it a first time before including the libarabica headers (in case
// config.h was included before the current header)...
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION

#include <SAX/helpers/DefaultHandler.hpp>
#include <SAX/Locator.hpp>
#include <SAX/Attributes.hpp>
#include <SAX/SAXException.hpp>

// ... and a second time after including them (because we include
// config.h right after)
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION
// We include config.h, because a cpp file which included it expects
// these defines
#include "config.h"

#include "logging.h"
#include "game_params.h"

class Dictionary;
class Game;
class Player;

using std::string;
using std::map;


class XmlReader : public Arabica::SAX::DefaultHandler<string>
{
    DEFINE_LOGGER();
public:
    virtual ~XmlReader() {}

    /**
     * Only entry point of the class.
     * Create a Game object, from a XML file created using the XmlWriter class.
     * The method throws an exception in case of problem.
     */
    static Game * read(const string &iFileName, const Dictionary &iDic);

    // Return the built game
    Game * getGame();

    ////////////////////////////////////////////////////
    // ContentHandler
    virtual void startElement(const string& namespaceURI,
                              const string& localName,
                              const string& qName,
                              const AttributesT& atts);
    virtual void endElement(const string& namespaceURI,
                            const string& localName,
                            const string& qName);
    virtual void characters(const string& ch);

    /////////////////////////////////////////////////////
    // ErrorHandler
    virtual void warning(const Arabica::SAX::SAXParseException<string>&);
    virtual void error(const Arabica::SAX::SAXParseException<string>&);
    virtual void fatalError(const Arabica::SAX::SAXParseException<string>& exception);

private:
    const Dictionary &m_dic;
    Game *m_game;
    string errorMessage;
    bool m_firstTurn;

    string m_context;
    string m_data;
    map<string, Player*> m_players;
    map<string, string> m_attributes;
    GameParams m_params;

    // Private constructor, because we only want the read() method
    // to be called externally
    XmlReader(const Dictionary &iDic) :
        m_dic(iDic), m_game(NULL), m_firstTurn(true), m_params(iDic) {}

    XmlReader(const XmlReader&);
    XmlReader& operator=(const XmlReader&);
    bool operator==(const XmlReader&);
};

#endif

