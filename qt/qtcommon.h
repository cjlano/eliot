/*****************************************************************************
 * Eliot
 * Copyright (C) 2008 Olivier Teulière
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

#ifndef QT_COMMON_H_
#define QT_COMMON_H_

#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
// Apparently needed on Windows, where libintl.h defines sprintf
// as libintl_sprintf...
#   undef sprintf
#else
#   define _(String) String
#endif

// Convert to/from utf-8 char*
#define qfu(s) QString::fromUtf8(s)
#define qtu(s) (s).toUtf8().data()
// Convert to/from std::wstring
#define qfw(s) QString::fromStdWString(s)
#define qtw(s) (s).toStdWString().data()
// Convert to/from local encoding
#define qfl(s) QString::fromLocal8Bit(s)
#define qtl(s) (s).toLocal8Bit().data()
// Translation macro to use gettext
#define _q(s) qfu(_(s))

#endif
