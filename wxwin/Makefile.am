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

if BUILD_WXWIDGETS

bin_PROGRAMS = wxeliot

AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" @WX_CPPFLAGS@ -I$(top_srcdir) -I$(top_srcdir)/dic -I$(top_srcdir)/game

wxeliot_SOURCES = \
    configdb.cc configdb.h \
    gfxboard.cc gfxboard.h \
    gfxresult.cc gfxresult.h \
    printout.cc printout.h \
    confdimdlg.cc confdimdlg.h \
    confsearch.cc confsearch.h \
    searchpanel.cc searchpanel.h \
    auxframes.cc auxframes.h  \
    mainframe.cc mainframe.h  \
    main.cc ewx.h

wxeliot_LDADD = @WX_LIBS@ ../game/libgame.a ../dic/libdic.a @LIBINTL@

EXTRA_DIST = \
    eliot.xpm \
    eliot.ico

endif

