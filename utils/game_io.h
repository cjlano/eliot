/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
 *
 * $Id: game_io.h,v 1.1 2005/02/26 22:57:34 ipkiss Exp $
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/

#ifndef _GAME_IO_H_
#define _GAME_IO_H_

#include <iostream>

class Game;
class Training;

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
    static void printBoard(ostream &out, const Game &iGame);
    static void printBoardJoker(ostream &out, const Game &iGame);
    static void printBoardMultipliers(ostream &out, const Game &iGame);
    static void printBoardMultipliers2(ostream &out, const Game &iGame);
    static void printNonPlayed(ostream &out, const Game &iGame);
    static void printPlayedRack(ostream &out, const Game &iGame, int n);
    static void printAllRacks(ostream &out, const Game &iGame);
    static void printSearchResults(ostream &out, const Training &iGame, int);
    static void printPoints(ostream &out, const Game &iGame);
    static void printAllPoints(ostream &out, const Game &iGame);

private:
    /// This class is a toolbox, and should not be instanciated
    GameIO();
};

#endif

