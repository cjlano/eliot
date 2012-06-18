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

#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include "csv_helper.h"

#include "debug.h"

#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
#else
#   define _(String) String
#endif

using namespace std;

INIT_LOGGER(qt, CsvHelper);


vector<CsvHelper::DataRow> CsvHelper::readStream(istream &input)
{
    vector<DataRow> data;
    size_t minLength = (size_t) -1;
    size_t maxLength = 0;
    string line;
    string currField;
    // XXX: Using getline() prevents from parsing newline chars in fields.
    // But this limitation is not very big, and it makes the code way simpler.
    while (getline(input, line))
    {
        DataRow row;
        // Parse the line
        bool inQuotes = false;
        size_t pos = 0;
        while (pos < line.size())
        {
            char c = line[pos];
            if (c == ',' && !inQuotes)
            {
                // Field separator
                row.push_back(currField);
                currField.clear();
            }
            else if (c == '"')
            {
                if (inQuotes)
                {
                    // Is the next char also a quote?
                    if (pos + 1 < line.size() && line[pos + 1] == '"')
                    {
                        // Escaped quote
                        currField.push_back(c);
                        ++pos;
                    }
                    else
                    {
                        // End of quotation
                        inQuotes = false;
                    }
                }
                else
                {
                    if (currField.empty())
                    {
                        // Beginning of quotation
                        inQuotes = true;
                    }
                    else
                    {
                        // Normal quote in an unquoted field
                        currField.push_back(c);
                    }
                }
            }
            else if (c != '\r' && c != '\n')
            {
                // Normal char
                currField.push_back(c);
            }

            // Next char
            ++pos;
        }

        // Add the last field of the line
        row.push_back(currField);
        currField.clear();

        data.push_back(row);

        if (minLength > row.size())
            minLength = row.size();
        if (maxLength < row.size())
            maxLength = row.size();
    }

    // Make sure we have a constant number of fields on the lines
    if (!data.empty() && minLength != maxLength)
    {
        boost::format fmt(_("Invalid CSV file (variable number of fields, from %1% to %2%)"));
        throw CsvException((fmt % minLength % maxLength).str());
    }

    return data;
}


void CsvHelper::writeStream(ostream &output, const vector<DataRow> &iData)
{
    if (iData.empty())
        return;

    // Make sure the rows have the same number of fields
    size_t firstLength = iData.front().size();
    BOOST_FOREACH(const DataRow &row, iData)
    {
        if (row.size() != firstLength)
            throw CsvException(_("Invalid CSV data (variable number of fields)"));
    }

    BOOST_FOREACH(const DataRow &row, iData)
    {
        bool first = true;
        BOOST_FOREACH(const string &field, row)
        {
            // Add the comma
            if (first)
                first = false;
            else
                output << ',';

            // Needs quoting?
            if (field.find_first_of(",\"\n") == string::npos)
                output << field;
            else
            {
                output << '"';
                BOOST_FOREACH(char c, field)
                {
                    output << c;
                    // Double the quote
                    if (c == '"')
                        output << '"';
                }
                output << '"';
            }
        }
        // End of the row (CR + LF)
        output << "\r\n";
    }
}

