/* Eliot                                                                     */
/* Copyright (C) 1999  Antoine Fraboulet                                     */
/*                                                                           */
/* This file is part of Eliot.                                               */
/*                                                                           */
/* Eliot is free software; you can redistribute it and/or modify             */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* Eliot is distributed in the hope that it will be useful,                  */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

/**
 *  \file   encoding.cpp
 *  \brief  Utility functions to ease manipulation of wide-character strings
 *  \author Olivier Teuliere
 *  \date   2005
 */

#include <stdlib.h>
#include <stdarg.h>
#include <wchar.h>
#include <wctype.h>
#include "encoding.h"


int _wtoi(const wchar_t *iWStr)
{
    return wcstol(iWStr,NULL,10);
}


int _swprintf(wchar_t *wcs, size_t maxlen, const wchar_t *format, ...)
{
    int res;
    va_list argp;
    va_start(argp, format);
#ifdef WIN32
    // Mingw32 does not take the maxlen argument
    res = vswprintf(wcs, format, argp);
#else
    res = vswprintf(wcs, maxlen, format, argp);
#endif
    va_end(argp);
    return res;
}


wstring convertToWc(const string& iStr)
{
    // Get the needed length (we _can't_ use string::size())
    size_t len = mbstowcs(NULL, iStr.c_str(), 0);
    if (len == (size_t)-1)
        return L"";

    wchar_t *tmp = new wchar_t[len + 1];
    len = mbstowcs(tmp, iStr.c_str(), len + 1);
    wstring res = tmp;
    delete[] tmp;

    return res;
}


string convertToMb(const wstring& iWStr)
{
    // Get the needed length (we _can't_ use wstring::size())
    size_t len = wcstombs(NULL, iWStr.c_str(), 0);
    if (len == (size_t)-1)
        return "";

    char *tmp = new char[len + 1];
    len = wcstombs(tmp, iWStr.c_str(), len + 1);
    string res = tmp;
    delete[] tmp;

    return res;
}


string convertToMb(wchar_t iWChar)
{
    char res[MB_CUR_MAX + 1];
    int len = wctomb(res, iWChar);
    if (len == -1)
        return "";
    res[len] = '\0';

    return res;
}

