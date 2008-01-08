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

/**
 *  \file   encoding.cpp
 *  \brief  Utility functions to ease handling of wide-character strings
 *  \author Olivier Teuliere
 *  \date   2005
 */

#include "config.h"

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <cerrno>
#include <iconv.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "encoding.h"
#include "dic_exception.h"

using namespace std;


#ifdef WIN32
// Utility function to get the last system error as a string
static string GetWin32Error()
{
    char *lpMsgBuf;
    DWORD dw = GetLastError();
    cerr << dw << endl;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, dw,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &lpMsgBuf,
                  0, NULL);
    string msg = lpMsgBuf;
    LocalFree(lpMsgBuf);
    return msg;
}
#endif


#if !HAVE_WCWIDTH
// wcwidth replacement (for win32 in particular)
// Inspired from the gnulib package, without some of the refinements
static inline int wcwidth(wchar_t c)
{
    // Assume all the printable characters have width 1
    return c == 0 ? 0 : (iswprint(c) ? 1 : -1);
}
#endif


int _wtoi(const wchar_t *iWStr)
{
    return wcstol(iWStr, NULL, 10);
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


wchar_t *_wcstok(wchar_t *wcs, const wchar_t *delim, wchar_t **ptr)
{
#ifdef WIN32
    // Mingw32 does not take the third argument
    return wcstok(wcs, delim);
#else
    return wcstok(wcs, delim, ptr);
#endif
}


#define _MAX_SIZE_FOR_STACK_ 30
wstring convertToWc(const string& iStr)
{
#ifdef WIN32
    // XXX: Assume the input is in UTF-8
    return readFromUTF8(iStr.c_str(), iStr.size(), "convertToWc");
#else
    // Get the needed length (we _can't_ use string::size())
    size_t len = mbstowcs(NULL, iStr.c_str(), 0);
    if (len == (size_t)-1)
        return L"";

    // Change the allocation method depending on the length of the string
    if (len < _MAX_SIZE_FOR_STACK_)
    {
        // Without multi-thread, we can use static storage
        static wchar_t tmp[_MAX_SIZE_FOR_STACK_];
        len = mbstowcs(tmp, iStr.c_str(), len + 1);
        return tmp;
    }
    else
    {
        wchar_t *tmp = new wchar_t[len + 1];
        len = mbstowcs(tmp, iStr.c_str(), len + 1);
        wstring res = tmp;
        delete[] tmp;
        return res;
    }
#endif
}


string convertToMb(const wstring& iWStr)
{
#ifdef WIN32
    const unsigned int size = iWStr.size() * 4;
    char buf[size];
    // XXX: Assume the output is in UTF-8
    int nb = writeInUTF8(iWStr, buf, size, "convertToMb");
    return string(buf, nb);
#else
    // Get the needed length (we _can't_ use wstring::size())
    size_t len = wcstombs(NULL, iWStr.c_str(), 0);
    if (len == (size_t)-1)
        return "";

    // Change the allocation method depending on the length of the string
    if (len < _MAX_SIZE_FOR_STACK_)
    {
        // Without multi-thread, we can use static storage
        static char tmp[_MAX_SIZE_FOR_STACK_];
        len = wcstombs(tmp, iWStr.c_str(), len + 1);
        return tmp;
    }
    else
    {
        char *tmp = new char[len + 1];
        len = wcstombs(tmp, iWStr.c_str(), len + 1);
        string res = tmp;
        delete[] tmp;
        return res;
    }
#endif
}
#undef _MAX_SIZE_FOR_STACK_


string convertToMb(wchar_t iWChar)
{
#ifdef WIN32
    return convertToMb(wstring(1, iWChar));
#else
    char res[MB_CUR_MAX + 1];
    int len = wctomb(res, iWChar);
    if (len == -1)
        return "";
    res[len] = '\0';

    return res;
#endif
}


string truncString(const string &iStr, unsigned int iMaxWidth)
{
    // Heuristic: the width of a character cannot exceed the number of
    // bytes used to represent it (even in UTF-8)
    if (iStr.size() <= iMaxWidth)
        return iStr;
    return truncAndConvert(convertToWc(iStr), iMaxWidth);
}


string truncAndConvert(const wstring &iWstr, unsigned int iMaxWidth)
{
    unsigned int width = 0;
    unsigned int pos;
    for (pos = 0; pos < iWstr.size(); ++pos)
    {
        int n = wcwidth(iWstr[pos]);
        if (n == -1)
        {
            ostringstream ss;
            ss << "truncAndConvert: non printable character: " << iWstr[pos];
            // XXX: Should we throw an exception instead? Just ignore the problem?
            cerr << ss.str() << endl;;
            //throw DicException(ss.str());
            return convertToMb(iWstr);
        }
        if (width + n > iMaxWidth)
            break;
        width += n;
    }

    return convertToMb(iWstr.substr(0, pos));
}


string truncOrPad(const string &iStr, unsigned int iWidth, char iChar)
{
    wstring wstr = convertToWc(iStr);
    unsigned int width = 0;
    unsigned int pos;
    for (pos = 0; pos < wstr.size(); ++pos)
    {
        int n = wcwidth(wstr[pos]);
        if (n == -1)
        {
            ostringstream ss;
            ss << "truncAndConvert: non printable character: " << wstr[pos];
            // XXX: Should we throw an exception instead? Just ignore the problem?
            cerr << ss.str() << endl;;
            //throw DicException(ss.str());
            return convertToMb(wstr);
        }
        if (width + n > iWidth)
            break;
        width += n;
    }

    if (iWidth > width)
        return convertToMb(wstr.substr(0, pos)) + string(iWidth - width, iChar);
    else
        return convertToMb(wstr.substr(0, pos));
}


string padAndConvert(const wstring &iWstr, unsigned int iLength,
                     bool iLeftPad, char c)
{
    int width = 0;
    for (unsigned int i = 0; i < iWstr.size(); ++i)
    {
        int n = wcwidth(iWstr[i]);
        if (n == -1)
        {
            ostringstream ss;
            ss << "padAndConvert: non printable character: " << iWstr[i];
            // XXX: Should we throw an exception instead? Just ignore the problem?
            cerr << ss.str() << endl;;
            //throw DicException(ss.str());
            return convertToMb(iWstr);
        }
        width += n;
    }

    if ((unsigned int)width >= iLength)
        return convertToMb(iWstr);
    else
    {
        // Padding is needed
        string s(iLength - width, c);
        if (iLeftPad)
            return s + convertToMb(iWstr);
        else
            return convertToMb(iWstr) + s;
    }
}


unsigned int readFromUTF8(wchar_t *oString, unsigned int iWideSize,
                          const char *iBuffer, unsigned int iBufSize,
                          const string &iContext)
{
#ifdef WIN32
    int res = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, iBuffer,
                                  iBufSize, oString, iWideSize);
    if (res == 0)
    {
        // Retrieve the system error message for the last-error code
        throw DicException("readFromUTF8: MultiByteToWideChar failed (" +
                           iContext + "): " + GetWin32Error());
    }
    return res;
#else
    iconv_t handle = iconv_open("WCHAR_T", "UTF-8");
    if (handle == (iconv_t)(-1))
        throw DicException("readFromUTF8: iconv_open failed");
    size_t inChars = iBufSize;
    size_t outChars = iWideSize * sizeof(wchar_t);
    // Use the ICONV_CONST trick because the declaration of iconv()
    // differs depending on the implementations...
    ICONV_CONST char *in = const_cast<ICONV_CONST char*>(iBuffer);
    char *out = (char*)oString;
    size_t res = iconv(handle, &in, &inChars, &out, &outChars);
    iconv_close(handle);
    // Problem during encoding conversion?
    if (res == (size_t)(-1))
    {
        throw DicException("readFromUTF8: iconv failed (" +
                           iContext + "): " + string(strerror(errno)));
    }
    return iWideSize - outChars / sizeof(wchar_t);
#endif
}


wstring readFromUTF8(const char *iBuffer, unsigned int iBufSize,
                     const string &iContext)
{
    // Temporary buffer for output
    // We will have at most as many characters as in the UTF-8 string
    wchar_t *wideBuf = new wchar_t[iBufSize];
    unsigned int number;
    try
    {
        number = readFromUTF8(wideBuf, iBufSize, iBuffer, iBufSize, iContext);
    }
    catch (...)
    {
        // Make sure not to leak
        delete[] wideBuf;
        throw;
    }
    // Copy the string
    wstring res(wideBuf, number);
    delete[] wideBuf;
    return res;
}


unsigned int writeInUTF8(const wstring &iWString, char *oBuffer,
                         unsigned int iBufSize, const string &iContext)
{
#ifdef WIN32
    int res = WideCharToMultiByte(CP_UTF8, 0, iWString.c_str(), iWString.size(),
                                  oBuffer, iBufSize, NULL, NULL);
    if (res == 0)
    {
        DWORD dw = GetLastError();
        cerr << dw << endl;
        // Retrieve the system error message for the last-error code
        throw DicException("writeInUTF8: WideCharToMultiByte failed (" +
                           iContext + "): " + GetWin32Error());
    }
    return res;
#else
    iconv_t handle = iconv_open("UTF-8", "WCHAR_T");
    if (handle == (iconv_t)(-1))
        throw DicException("writeInUTF8: iconv_open failed");
    size_t length = iWString.size();
    size_t inChars = sizeof(wchar_t) * length;
    size_t outChars = iBufSize;
    // Use the ICONV_CONST trick because the declaration of iconv()
    // differs depending on the implementations...
    // FIXME: bonus ugliness for doing 2 casts at once, and accessing string
    // internals...
    ICONV_CONST char *in = (ICONV_CONST char*)(&iWString[0]);
    char *out = oBuffer;
    size_t res = iconv(handle, &in, &inChars, &out, &outChars);
    iconv_close(handle);
    // Problem during encoding conversion?
    if (res == (size_t)(-1))
    {
        throw DicException("writeInUTF8: iconv failed (" +
                           iContext + ")" + string(strerror(errno)));
    }
    // Return the number of written bytes
    return iBufSize - outChars;
#endif
}

