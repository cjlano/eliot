/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
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

#ifndef GAME_IO_H_
#define GAME_IO_H_

#include <iostream>

class PublicGame;
class Results;

using std::ostream;


/**
 * This class offers methods to display some information relative to a game
 * object. It is currently used by the 'eliottxt' interface, but is also useful
 * for debugging purposes. Feel free to add other printing methods that you may
 * need...
 *
 * TODO: Maybe we could also use this class as a basis for non-regression tests?
 */
class GameIO
{
public:
    static void printBoard(ostream &out, const PublicGame &iGame);
    static void printBoardDebug(ostream &out, const PublicGame &iGame);
    static void printBoardJoker(ostream &out, const PublicGame &iGame);
    static void printBoardMultipliers(ostream &out, const PublicGame &iGame);
    static void printBoardMultipliers2(ostream &out, const PublicGame &iGame);

    static void printNonPlayed(ostream &out, const PublicGame &iGame);
    static void printPlayedRack(ostream &out, const PublicGame &iGame, int n);
    static void printAllRacks(ostream &out, const PublicGame &iGame);
    static void printSearchResults(ostream &out, const Results &iResults, int);
    static void printPoints(ostream &out, const PublicGame &iGame);
    static void printAllPoints(ostream &out, const PublicGame &iGame);

    static void printGameDebug(ostream &out, const PublicGame &iGame);

private:
    /// This class is a toolbox, and should not be instanciated
    GameIO();
};

#endif

