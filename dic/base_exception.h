/*****************************************************************************
 * Eliot
 * Copyright (C) 2010 Olivier Teulière
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

#ifndef BASE_EXCEPTION_H_
#define BASE_EXCEPTION_H_

#include <exception>
#include <string>


/**
 * Base exception class for all the exception classes in Eliot.
 * It provides a stack trace.
 */
class BaseException: public std::exception
{
    public:
        BaseException(const std::string &iMessage);
        ~BaseException() throw() {}
        virtual const char *what() const throw();

        std::string getStackTrace() const;

    private:
        std::string m_message;
        std::string m_stack;
};

#endif
