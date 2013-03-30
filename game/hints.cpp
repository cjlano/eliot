/*****************************************************************************
 * Eliot
 * Copyright (C) 2013 Olivier Teulière
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

#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include "config.h"
#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
#else
#   define _(String) String
#endif


#include "hints.h"
#include "move.h"
#include "encoding.h"
#include "debug.h"


INIT_LOGGER(game, AbstractHint);


AbstractHint::AbstractHint(const string &iName, const string &iDescription, int iCost)
    : m_name(iName), m_description(iDescription), m_cost(iCost)
{
}



ScoreHint::ScoreHint()
    : AbstractHint(_("Score"),
                   _("Get the score of the move"), 60)
{
}


string ScoreHint::giveHint(const Move &iMove) const
{
    return str(boost::format(_("Score: %1%")) % iMove.getScore());
}



OrientationHint::OrientationHint()
    : AbstractHint(_("Orientation"),
                   _("Get the orientation of the move (horizontal/vertical)"), 20)
{
}


string OrientationHint::giveHint(const Move &iMove) const
{
    ASSERT(iMove.isValid(), "Hints only make sense for valid moves");
    const Coord &coord = iMove.getRound().getCoord();
    boost::format fmt(_("Orientation: %1%"));
    if (coord.getDir() == Coord::HORIZONTAL)
        fmt % _("horizontal");
    else
        fmt % _("vertical");
    return fmt.str();
}



PositionHint::PositionHint()
    : AbstractHint(_("Position"),
                   _("Get the coordinates of the move"), 80)
{
}


string PositionHint::giveHint(const Move &iMove) const
{
    ASSERT(iMove.isValid(), "Hints only make sense for valid moves");
    return str(boost::format(_("Position: %1%"))
               % lfw(iMove.getRound().getCoord().toString()));
}



LengthHint::LengthHint()
    : AbstractHint(_("Length"),
                   _("Get the length of the word"), 40)
{
}


string LengthHint::giveHint(const Move &iMove) const
{
    ASSERT(iMove.isValid(), "Hints only make sense for valid moves");
    return str(boost::format(_("Length: %1% letters"))
               % iMove.getRound().getWordLen());
}



BoardLettersHint::BoardLettersHint()
    : AbstractHint(_("Letters from board"),
                   _("Get the letters of the word coming from the board"), 70)
{
}


string BoardLettersHint::giveHint(const Move &iMove) const
{
    ASSERT(iMove.isValid(), "Hints only make sense for valid moves");
    // Retrieve the letters coming from the board
    wstring fromBoard;
    for (unsigned i = 0; i < iMove.getRound().getWordLen(); ++i)
    {
        if (!iMove.getRound().isPlayedFromRack(i))
            fromBoard.push_back(iMove.getRound().getTile(i).toChar());
    }

    return str(boost::format(_("Letters from board: %1%"))
               % (fromBoard == L"" ? _("(none)") : lfw(fromBoard)));
}



WordLettersHint::WordLettersHint()
    : AbstractHint(_("Word letters"),
                   _("Get the letters of the word in alphabetical order"), 120)
{
}


string WordLettersHint::giveHint(const Move &iMove) const
{
    ASSERT(iMove.isValid(), "Hints only make sense for valid moves");
    vector<Tile> tiles = iMove.getRound().getTiles();
    // Sort the letters (we cannot sort directly the wstring from
    // Round::getWord(), because it would break digraph characters)
    std::sort(tiles.begin(), tiles.end());

    // Get the word
    wstring word;
    BOOST_FOREACH(const Tile &tile, tiles)
    {
        word += tile.getDisplayStr();
    }

    return str(boost::format(_("Word letters: %1%"))
               % lfw(word));
}



FirstLetterHint::FirstLetterHint()
    : AbstractHint(_("First letter"),
                   _("Get the first letter of the word"), 120)
{
}


string FirstLetterHint::giveHint(const Move &iMove) const
{
    ASSERT(iMove.isValid(), "Hints only make sense for valid moves");
    return str(boost::format(_("First letter: %1%"))
               % lfw(iMove.getRound().getTile(0).toChar()));
}


