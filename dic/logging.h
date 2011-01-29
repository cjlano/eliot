/*****************************************************************************
 * Eliot
 * Copyright (C) 2011 Olivier Teulière
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

#ifndef DIC_LOGGING_H_
#define DIC_LOGGING_H_

#include <config.h>

#ifdef USE_LOGGING
#   include <log4cxx/logger.h>

#   define DEFINE_LOGGER() static log4cxx::LoggerPtr logger
#   define INIT_LOGGER(prefix, className) log4cxx::LoggerPtr className::logger(log4cxx::Logger::getLogger(#prefix "." #className))

#   define LOG_TRACE(a) LOG4CXX_TRACE(logger, a)
#   define LOG_DEBUG(a) LOG4CXX_DEBUG(logger, a)
#   define LOG_INFO(a) LOG4CXX_INFO(logger, a)
#   define LOG_WARN(a) LOG4CXX_WARN(logger, a)
#   define LOG_ERROR(a) LOG4CXX_ERROR(logger, a)
#   define LOG_FATAL(a) LOG4CXX_FATAL(logger, a)
#   define LOG_ROOT_ERROR(a) LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(), a)
#   define LOG_ROOT_FATAL(a) LOG4CXX_FATAL(log4cxx::Logger::getRootLogger(), a)
#else
#   define DEFINE_LOGGER()
#   define INIT_LOGGER(prefix, name)

#   define LOG_TRACE(a)
#   define LOG_DEBUG(a)
#   define LOG_INFO(a)
#   define LOG_WARN(a)
#   define LOG_ERROR(a)
#   define LOG_FATAL(a)
#   define LOG_ROOT_ERROR(a)
#   define LOG_ROOT_FATAL(a)
#endif // USE_LOGGING

#endif

