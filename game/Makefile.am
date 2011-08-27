# Eliot
# Copyright (C) 1999-2007  Antoine Fraboulet & Olivier Teulière
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

noinst_LIBRARIES = libgame.a

AM_CPPFLAGS = -I$(top_srcdir)/dic -I../intl -I$(top_srcdir)/intl @LIBCONFIG_CFLAGS@ @ARABICA_CFLAGS@ @EXPAT_CFLAGS@ @LOG4CXX_CFLAGS@

libgame_a_SOURCES= \
    game_exception.cpp game_exception.h \
    command.h command.cpp \
    coord.cpp coord.h \
    cross.cpp cross.h \
    rack.cpp rack.h \
    pldrack.cpp pldrack.h \
    round.cpp round.h \
    move.cpp move.h \
    results.cpp results.h \
    bag.cpp bag.h \
    turn.cpp turn.h \
    history.cpp history.h \
    player.cpp player.h \
    player_points_cmd.cpp player_points_cmd.h \
    player_move_cmd.cpp player_move_cmd.h \
    player_rack_cmd.cpp player_rack_cmd.h \
    ai_player.h \
    ai_percent.cpp ai_percent.h \
    game_params.h \
    board.cpp board.h \
    board_cross.cpp \
    matrix.h \
    board_search.cpp board_search.h \
    settings.cpp settings.h \
    navigation.cpp navigation.h \
    game.cpp game.h \
    game_move_cmd.h game_move_cmd.cpp \
    turn_cmd.cpp turn_cmd.h \
    duplicate.cpp duplicate.h \
    mark_played_cmd.h mark_played_cmd.cpp \
    freegame.cpp freegame.h \
    training.cpp training.h \
    public_game.cpp public_game.h \
    game_factory.cpp game_factory.h \
    game_io.cpp \
    xml_writer.cpp xml_writer.h \
    xml_reader.cpp xml_reader.h \
    debug.h

