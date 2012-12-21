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

#include "config.h"

#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <cerrno>
#include <iconv.h>

#include "encoding.h"
#include "dic_exception.h"

using namespace std;


#if !HAVE_WCWIDTH
// wcwidth replacement (for win32 in particular)
// Inspired from the gnulib package, without some of the refinements
static inline int wcwidth(wchar_t c)
{
    // Assume all the printable characters have width 1
    return c == 0 ? 0 : (iswprint(c) ? 1 : -1);
}
#endif


int wtoi(const wchar_t *iWStr)
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
    (void)maxlen;
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
    (void)ptr;
    return wcstok(wcs, delim);
#else
    return wcstok(wcs, delim, ptr);
#endif
}


#define _MAX_SIZE_FOR_STACK_ 30
wstring convertToWc(const string& iStr)
{
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
}


string convertToMb(const wstring& iWStr)
{
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
}
#undef _MAX_SIZE_FOR_STACK_


string convertToMb(wchar_t iWChar)
{
    return convertToMb(wstring(1, iWChar));
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
            // XXX: Should we throw an exception instead? Just ignore the problem?
#if 0
            ss << "truncAndConvert: non printable character: " << iWstr[pos];
            cerr << ss.str() << endl;;
            //throw DicException(ss.str());
#endif
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
            // XXX: Should we throw an exception instead? Just ignore the problem?
#if 0
            ss << "truncAndConvert: non printable character: " << wstr[pos];
            cerr << ss.str() << endl;;
            //throw DicException(ss.str());
#endif
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
            // XXX: Should we throw an exception instead? Just ignore the problem?
#if 0
            ss << "padAndConvert: non printable character: " << iWstr[i];
            cerr << ss.str() << endl;;
            //throw DicException(ss.str());
#endif
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


string centerAndConvert(const wstring &iWstr, unsigned int iLength, char c)
{
    int width = 0;
    for (unsigned int i = 0; i < iWstr.size(); ++i)
    {
        int n = wcwidth(iWstr[i]);
        if (n == -1)
        {
            ostringstream ss;
            // XXX: Should we throw an exception instead? Just ignore the problem?
#if 0
            ss << "padAndConvert: non printable character: " << iWstr[i];
            cerr << ss.str() << endl;;
            //throw DicException(ss.str());
#endif
            return convertToMb(iWstr);
        }
        width += n;
    }

    if ((unsigned int)width >= iLength)
        return convertToMb(iWstr);
    else
    {
        // Padding is needed
        string s((iLength - width) / 2, c);
        string res = s + convertToMb(iWstr) + s;
        // If the string cannot be centered perfectly, pad again
        // (on the left if iLength is even, on the right otherwise:
        //  this tends to align numbers of 1 or 2 digits in a nice way)
        // Note: if needed, we could add the iLeftPad argument
        if ((iLength - width) % 2)
        {
            if (iLength % 2)
                res.append(1, c);
            else
                res.insert(res.begin(), c);
        }
        return res;
    }
}


wstring toUpper(const wstring &iWstr)
{
    wstring str = iWstr;
    std::transform(str.begin(), str.end(), str.begin(), towupper);
    return str;
}


wstring toLower(const wstring &iWstr)
{
    wstring str = iWstr;
    std::transform(str.begin(), str.end(), str.begin(), towlower);
    return str;
}


unsigned int readFromUTF8(wchar_t *oString, unsigned int iWideSize,
                          const char *iBuffer, unsigned int iBufSize,
                          const string &iContext)
{
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
}


wstring readFromUTF8(const string &iString, const string &iContext)
{
    const int size = iString.size();
    // Temporary buffer for output
    // We will have at most as many characters as in the UTF-8 string
    wchar_t *wideBuf = new wchar_t[size];
    unsigned int number;
    try
    {
        number = readFromUTF8(wideBuf, size, iString.data(), size, iContext);
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
                           iContext + "): " + string(strerror(errno)));
    }
    // Return the number of written bytes
    return iBufSize - outChars;
}


string writeInUTF8(const wstring &iWString, const string &iContext)
{
    // Temporary buffer for output
    // Each character will take at most 4 bytes in the UTF-8 string
    unsigned int bufSize = iWString.size() * 4;
    char *buf = new char[bufSize];
    unsigned int number;
    try
    {
        number = writeInUTF8(iWString, buf, bufSize, iContext);
    }
    catch (...)
    {
        // Make sure not to leak
        delete[] buf;
        throw;
    }
    // Copy the string
    string res(buf, number);
    delete[] buf;
    return res;
}

