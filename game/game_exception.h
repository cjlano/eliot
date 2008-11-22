/*****************************************************************************
 * Eliot
 * Copyright (C) 2007 Olivier Teulière
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

#ifndef _GAME_EXCEPTION_H_
#define _GAME_EXCEPTION_H_

#include <exception>
#include <string>


/**
 * Exception class for the Game library.
 * It simply inherits from the standard exception and overrides
 * its what() method.
 */
class GameException: public std::exception
{
    public:
        GameException(const std::string &iMessage);
        ~GameException() throw() {}
        virtual const char *what() const throw();

    private:
        std::string m_message;
};


class EndGameException: public GameException
{
    public:
        EndGameException(const std::string &iMessage);
};

#endif
