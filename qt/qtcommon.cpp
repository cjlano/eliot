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

#include "qtcommon.h"

using namespace std;


wstring qtw(const QString &q)
{
#ifdef QT_NO_STL
    wchar_t *array = new wchar_t[q.size()];
    int size = q.toWCharArray(array);
    wstring ws(array, size);
    delete[] array;
    return ws;
#else
    return q.toStdWString();
#endif
}


QString qfw(const wstring &wstr)
{
#ifdef QT_NO_STL
    return QString::fromWCharArray(wstr.c_str());
#else
    return QString::fromStdWString(wstr);
#endif
}

