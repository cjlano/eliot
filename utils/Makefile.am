# Eliot
# Copyright (C) 1999  Antoine Fraboulet
# antoine.fraboulet@free.fr
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

localedir = $(datadir)/locale
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" -I$(top_srcdir)/dic -I$(top_srcdir)/game -I../intl -I$(top_srcdir)/intl @LOG4CXX_CFLAGS@

noinst_PROGRAMS =
bin_PROGRAMS =

if BUILD_TEXT
noinst_PROGRAMS += eliottxt
eliottxt_SOURCES = game_io.h game_io.cpp eliottxt.cpp
eliottxt_LDADD = $(top_builddir)/game/libgame.a $(top_builddir)/dic/libdic.a @LIBINTL@ @LIBCONFIG_LIBS@ @ARABICA_LIBS@ @EXPAT_LIBS@

if HAS_READLINE
eliottxt_LDADD += -lreadline
endif
if WITH_LOGGING
eliottxt_LDADD += @LOG4CXX_LIBS@
endif
endif

if BUILD_NCURSES
bin_PROGRAMS += eliotcurses
eliotcurses_SOURCES = curses_intf.cpp curses_intf.h
eliotcurses_LDADD = ../game/libgame.a ../dic/libdic.a @CURSES_LIB@ @LIBINTL@ @LIBCONFIG_LIBS@ @ARABICA_LIBS@ @EXPAT_LIBS@
if WITH_LOGGING
eliotcurses_LDADD += @LOG4CXX_LIBS@
endif

endif

