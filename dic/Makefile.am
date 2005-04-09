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

# $Id: Makefile.am,v 1.5 2005/04/09 19:16:09 afrab Exp $ 

noinst_LIBRARIES = libdic.a

INCLUDES = -I$(top_srcdir)

libdic_a_SOURCES = 		  \
	dic_internals.h		  \
	dic_search.c dic_search.h \
	dic.c dic.h		  \
	automaton.c automaton.h   \
	regexp.c regexp.h

##################################### 
# if BUILD_DICTOOLS
# automake does not support regexp_YFLAGS being defined conditionally

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

regexp_YFLAGS=-d
regexp_CFLAGS=${REGEXP_FLAGS}

regexp_SOURCES=	                \
	regexpmain.c		\
	regexp.c regexp.h	\
	automaton.h automaton.c	\
	er.y			\
	er.l			

# endif
