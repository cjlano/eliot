/*****************************************************************************
 * Eliot
 * Copyright (C) 2012 Olivier Teulière
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

#ifndef CSV_HELPER_H_
#define CSV_HELPER_H_

#include <string>
#include <vector>
#include <iosfwd>

#include "logging.h"
#include "base_exception.h"

using std::string;
using std::vector;
using std::istream;
using std::ostream;


/**
 * Utility class to handle CSV (Comma Separated Value) files.
 * The CSV format respects the guidelines summary listed here (RFC 4180):
 * https://en.wikipedia.org/wiki/Comma-separated_values#Toward_standardization
 * except for the newlines in fields which are not supported when parsing.
 */
class CsvHelper
{
    DEFINE_LOGGER();

public:
    typedef vector<string> DataRow;

    static vector<DataRow> readStream(istream &input);

    static void writeStream(ostream &output, const vector<DataRow> &iData);

};


class CsvException: public BaseException
{
public:
    CsvException(const std::string &iMessage)
        : BaseException(iMessage) {}
};

#endif

