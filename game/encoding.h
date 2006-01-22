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
 *  \file   encoding.h
 *  \brief  Utility functions to ease manipulation of wide-character strings
 *  \author Olivier Teuliere
 *  \date   2005
 */

#ifndef _ENCODING_H_
#define _ENCODING_H_

#include <string>

using std::string;
using std::wstring;


/// Equivalent of atoi for wide-caracter strings
int _wtoi(const wchar_t *iWStr);

/// Equivalent of swprintf, but working also with mingw32
int _swprintf(wchar_t *wcs, size_t maxlen, const wchar_t *format, ...);

/// Convert a multi-byte string into a wide-character string
wstring convertToWc(const string& iStr);

/// Convert a wide-character string into a multi-byte string
string convertToMb(const wstring& iWStr);

/// Convert a wide character into a multi-byte string
string convertToMb(wchar_t iWChar);

#endif

