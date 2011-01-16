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

#   define DEFINE_LOGGER(logger) static log4cxx::LoggerPtr logger;
#   define INIT_LOGGER(logger, name) log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(name))

#   define LOG_TRACE(a, b) LOG4CXX_TRACE(a, b)
#   define LOG_DEBUG(a, b) LOG4CXX_DEBUG(a, b)
#   define LOG_INFO(a, b) LOG4CXX_INFO(a, b)
#   define LOG_WARN(a, b) LOG4CXX_WARN(a, b)
#   define LOG_ERROR(a, b) LOG4CXX_ERROR(a, b)
#   define LOG_FATAL(a, b) LOG4CXX_FATAL(a, b)
#else
#   define DEFINE_LOGGER(logger)
#   define INIT_LOGGER(logger, name)

#   define LOG_TRACE(a, b)
#   define LOG_DEBUG(a, b)
#   define LOG_INFO(a, b)
#   define LOG_WARN(a, b)
#   define LOG_ERROR(a, b)
#   define LOG_FATAL(a, b)
#endif // USE_LOGGING

#endif

