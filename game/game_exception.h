/*****************************************************************************
 * Eliot
 * Copyright (C) 2007-2010 Olivier Teulière
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

#ifndef GAME_EXCEPTION_H_
#define GAME_EXCEPTION_H_

#include <string>
#include "base_exception.h"


/**
 * Exception class for the Game library.
 * It simply inherits from the base exception and overrides
 * its what() method.
 */
class GameException: public BaseException
{
    public:
        GameException(const std::string &iMessage);
};


class EndGameException: public GameException
{
    public:
        EndGameException(const std::string &iMessage);
};


class LoadGameException: public GameException
{
    public:
        LoadGameException(const std::string &iMessage);
};


class SaveGameException: public GameException
{
    public:
        SaveGameException(const std::string &iMessage);
};

#endif
