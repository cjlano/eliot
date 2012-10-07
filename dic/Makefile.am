# Eliot
# Copyright (C) 1999-2012  Antoine Fraboulet & Olivier Teulière
# Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
#          Olivier Teulière <ipkiss @@ gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

noinst_LIBRARIES = libdic.a

localedir = $(datadir)/locale
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" -I$(top_srcdir) -I../intl -I$(top_srcdir)/intl $(INCICONV) @LOG4CXX_CFLAGS@

libdic_a_SOURCES = \
	logging.h \
	base_exception.cpp base_exception.h \
	dic_exception.cpp dic_exception.h \
	header.cpp header.h \
	dic_internals.h \
	tile.cpp tile.h \
	dic.cpp dic.h \
	dic_search.cpp \
	csv_helper.cpp csv_helper.h \
	encoding.cpp encoding.h \
	stacktrace.cpp stacktrace.h \
	automaton.cpp automaton.h \
	regexp.cpp regexp.h \
	grammar.cpp grammar.h \
	compdic.cpp compdic.h \
	listdic.cpp listdic.h \
	debug.h

#####################################
if BUILD_DICTOOLS

bin_PROGRAMS = \
	compdic \
	listdic \
	regexp

compdic_SOURCES=compdicmain.cpp
compdic_CPPFLAGS=$(AM_CPPFLAGS) @BOOST_CPPFLAGS@
compdic_LDADD=libdic.a @LIBINTL@

listdic_SOURCES=listdicmain.cpp
listdic_LDADD=libdic.a @LIBINTL@

#regexp_CFLAGS=-DDEBUG_RE
regexp_SOURCES=regexpmain.cpp
regexp_LDADD=libdic.a @LIBINTL@

if WITH_LOGGING
compdic_LDADD += @LOG4CXX_LIBS@
listdic_LDADD += @LOG4CXX_LIBS@
regexp_LDADD += @LOG4CXX_LIBS@
endif

endif

