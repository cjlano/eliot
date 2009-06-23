/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2007 Olivier Teulière
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

#ifndef _ENCODING_H_
#define _ENCODING_H_

#include <string>

using std::string;
using std::wstring;


/// Equivalent of atoi for wide-caracter strings
int _wtoi(const wchar_t *iWStr);

/// Equivalent of swprintf, but working also with mingw32
int _swprintf(wchar_t *wcs, size_t maxlen, const wchar_t *format, ...);

/// Equivalent of wcstok, but working also with mingw32
wchar_t *_wcstok(wchar_t *wcs, const wchar_t *delim, wchar_t **ptr);

/// Convert a multi-byte string into a wide-character string
wstring convertToWc(const string &iStr);

/// Convert a wide-character string into a multi-byte string
string convertToMb(const wstring &iWStr);

/// Convert a wide character into a multi-byte string
string convertToMb(wchar_t iWChar);

/**
 * Truncate the given string to ensure that the number of columns needed
 * to display it is at most iMaxWidth. If the string is already less wide,
 * it is returned without truncation
 */
string truncString(const string &iStr, unsigned int iMaxWidth);

/**
 * Convert the given string into a multi-byte one. If the number of columns
 * needed to display the resulting string is more than iMaxWidth, truncate it
 * on the right before conversion
 */
string truncAndConvert(const wstring &iWStr, unsigned int iMaxWidth);

/**
 * Make sure the displayed version of iStr has a width of iWidth.
 * If the string is too long, truncate it, if it is too short, pad it
 * with iChar
 */
string truncOrPad(const string &iStr, unsigned int iWidth, char iChar = ' ');

/**
 * Convert the given string into a multi-byte one. If the number of columns
 * needed to display the resulting string is less than iLength, pad it with
 * the given character (defaulting to space)
 */
string padAndConvert(const wstring &iWstr, unsigned int iLength,
                     bool iLeftPad = true, char c = ' ');

/**
 * Convert the given string into a multi-byte one. If the number of columns
 * needed to display the resulting string is less than iLength, pad it with
 * the given character (defaulting to space) on both left and right, trying
 * to keep the string centered
 */
string centerAndConvert(const wstring &iWstr, unsigned int iLength,
                        char c = ' ');

/**
 * Utility function to convert a char* buffer encoded in UTF-8 into a
 * wchar_t* string
 * @param oString: where to write the converted string
 * @param iWideSize: size available in oString (number of wchar_t)
 * @param iBuffer: UTF-8 string to convert
 * @param iBufSize: available size in iBuffer
 * @param iContext: free text used in case of exception
 * @return: number of wide chars actually written
 */
unsigned int readFromUTF8(wchar_t *oString, unsigned int iWideSize,
                          const char *iBuffer, unsigned int iBufSize,
                          const string &iContext);

/**
 * Same as the other readFromUTF8 function, dealing with a wstring
 * instead of a wchar_t*. Note that it performs an additional copy
 * of the output string...
 * @param iBuffer: UTF-8 string to convert
 * @param iBufSize: available size in iBuffer
 * @param iContext: free text used in case of exception
 * @return: the converted wide string
 */
wstring readFromUTF8(const char *iBuffer, unsigned int iBufSize,
                     const string &iContext);

/**
 * Utility function to convert a wstring into an UTF-8 char* buffer
 * @param iWString: the wide string to encode
 * @param oBuffer: where to write the encoded string
 * @param iBufSize: available size in oBuffer
 * @param iContext: free text used in case of exception
 * @return: number of bytes actually written
 */
unsigned int writeInUTF8(const wstring &iWString, char *oBuffer,
                         unsigned int iBufSize, const string &iContext);

#endif

