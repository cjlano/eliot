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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# $Id: Makefile.am,v 1.5 2005/02/26 22:57:34 ipkiss Exp $

top_srcdir = @top_srcdir@

INCLUDES = -I$(top_srcdir)/dic -I$(top_srcdir)/game -I$(top_srcdir)/intl
noinst_PROGRAMS =

if BUILD_TEXT
noinst_PROGRAMS += eliottxt
eliottxt_SOURCES = game_io.h game_io.cpp eliottxt.cpp
eliottxt_LDADD = ../dic/libdic.a ../game/libgame.a
endif

if BUILD_NCURSES
noinst_PROGRAMS += eliotcurses
eliotcurses_SOURCES = ncurses.cpp
eliotcurses_LDADD = ../game/libgame.a ../dic/libdic.a -lncurses @LIBINTL@
endif

datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

