/* Eliot                                                                     */
/* Copyright (C) 1999  Antoine Fraboulet                                     */
/* Antoine.Fraboulet@free.fr                                                 */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/* $Id: ewx.h,v 1.3 2005/02/05 11:14:56 ipkiss Exp $ */

#ifndef __EWX__
#define __EWX__

#define DEBUG_

#ifdef DEBUG
  #define debug(x...) { fprintf(stderr,x); }
  #define FRAME_TRACE
#else 
  #define debug(x...) 
#endif

#if defined(__WIN32__) || defined(__WIN95__) || defined(__WXMSW__)
  #define ENABLE_LC_NO_HEADER
  #define INCOMPLETE
#else
  #define ENABLE_SAVE_POSTSCRIPT
  #define ENABLE_LOCALE
  #define INCOMPLETE { cerr << "incomplete " << __FILE__ << " " << __LINE__ << "\n"; }
#endif

#include "config.h"
#define APPNAME "Eliot"
#define DATE "$Date: 2005/02/05 11:14:56 $"
#endif

/* wxU is used to convert ansi/utf8 strings to unicode strings (wchar_t) */
#if defined( ENABLE_NLS ) && defined( ENABLE_UTF8 )
#if wxUSE_UNICODE
#   define wxU(utf8) wxString(utf8, wxConvUTF8)
#else
#   define wxU(utf8) wxString(wxConvUTF8.cMB2WC(utf8), *wxConvCurrent)
#endif

#else // ENABLE_NLS && ENABLE_UTF8
#if wxUSE_UNICODE
#   define wxU(ansi) wxString(ansi, *wxConvCurrent)
#else
#   define wxU(ansi) ansi
#endif

#endif
