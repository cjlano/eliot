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

noinst_LIBRARIES = libdic.a

INCLUDES = -I$(top_srcdir)

libdic_a_CFLAGS=$(DEBUGFLAGS)
libdic_a_YFLAGS=-d
libdic_a_SOURCES = 		  \
	er.y			  \
	er.l			  \
	dic_internals.h		  \
	dic_search.c dic_search.h \
	dic.c dic.h		  \
	automaton.c automaton.h   \
	hashtable.h hashtable.c	  \
	regexp.c regexp.h	  \
	alist.h alist.c

nodist_libdic_a_SOURCES=	\
	er.c			\
	scanner.h		\
	libdic_a-er.c 		\
	libdic_a-er.h

BUILT_SOURCES=er.c 	\
	scanner.h 	\
	libdic_a-er.c 	\
	libdic_a-er.h

CLEANFILES=er.c		\
	scanner.h 	\
	libdic_a-er.c	\
	libdic_a-er.h

#####################################
if BUILD_DICTOOLS

bin_PROGRAMS = 			\
	compdic 		\
	listdic 		\
	regexp

compdic_SOURCES=		\
	dic_internals.h		\
	hashtable.c hashtble.h	\
	compdic.c

listdic_SOURCES=		\
	dic_internals.h		\
	dic.c dic.h		\
	listdic.c

regexp_SOURCES=	regexpmain.c
#regexp_CFLAGS=-DDEBUG_RE
regexp_LDADD=libdic.a

endif
