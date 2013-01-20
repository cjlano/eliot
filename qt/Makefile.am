# Eliot
# Copyright (C) 2008-2012  Olivier Teulière
# Authors: Olivier Teulière <ipkiss @@ gmail.com>
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

if BUILD_QT

AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" @QT_CFLAGS@ -DQT4LOCALEDIR=\"@QT4LOCALEDIR@\" -I$(top_srcdir) -I../intl -I$(top_srcdir)/dic -I$(top_srcdir)/game @LOG4CXX_CFLAGS@

SUFFIXES=.ui.h .moc.cpp

bin_PROGRAMS = eliot

RESOURCES = \
    images/eliot.xpm \
    images/go-down.png \
    images/go-first.png \
    images/go-jump.png \
    images/go-last.png \
    images/go-next.png \
    images/go-previous.png \
    images/go-up.png \
    images/info_16px.png \
    images/playlist_16px.png \
    images/preferences.png \
    images/print-preview.png \
    images/printer.png \
    images/quit_16px.png

EXTRA_DIST = \
    ui/dic_tools_widget.ui \
    ui/dic_wizard_info_page.ui \
    ui/dic_wizard_letters_def_page.ui \
    ui/dic_wizard_conclusion_page.ui \
    ui/arbitration_widget.ui \
    ui/arbit_assignments.ui \
    ui/bag_widget.ui \
    ui/main_window.ui \
    ui/new_game.ui \
    ui/player_widget.ui \
    ui/prefs_dialog.ui \
    ui/tables_dialog.ui \
    ui/topping_widget.ui \
    ui/training_widget.ui \
    eliot.qrc \
    $(RESOURCES) \
    images/eliot.ico

eliot_SOURCES = \
    qtcommon.h qtcommon.cpp \
    game_signals.h game_signals.cpp \
    play_model.h play_model.cpp \
    tile_widget.cpp tile_widget.h \
    rack_widget.cpp rack_widget.h \
    timer_widget.cpp timer_widget.h \
    tile_layout.cpp tile_layout.h \
    validator_factory.h validator_factory.cpp \
    misc_helpers.cpp misc_helpers.h \
    custom_popup.cpp custom_popup.h \
    arbitration_widget.cpp arbitration_widget.h \
    arbit_assignments.cpp arbit_assignments.h \
    bag_widget.cpp bag_widget.h \
    dic_tools_widget.cpp dic_tools_widget.h \
    players_table_helper.cpp players_table_helper.h \
    fav_players.cpp fav_players.h \
    hints_dialog.cpp hints_dialog.h \
    new_game.cpp new_game.h \
    score_widget.cpp score_widget.h \
    dic_wizard.cpp dic_wizard.h \
    board_widget.cpp board_widget.h \
    history_widget.cpp history_widget.h \
    stats_widget.cpp stats_widget.h \
    play_word_mediator.cpp play_word_mediator.h \
    tables_dialog.cpp tables_dialog.h \
    topping_widget.cpp topping_widget.h \
    training_widget.cpp training_widget.h \
    player_widget.cpp player_widget.h \
    prefs_dialog.cpp prefs_dialog.h \
    update_checker.cpp update_checker.h \
    aux_window.cpp aux_window.h \
    main_window.cpp main_window.h \
    main.cpp

nodist_eliot_SOURCES = \
    ui/main_window.ui.h \
    ui/arbitration_widget.ui.h \
    ui/arbit_assignments.ui.h \
    ui/bag_widget.ui.h \
    ui/new_game.ui.h \
    ui/player_widget.ui.h \
    ui/tables_dialog.ui.h \
    ui/topping_widget.ui.h \
    ui/training_widget.ui.h \
    ui/prefs_dialog.ui.h \
    ui/dic_tools_widget.ui.h \
    ui/dic_wizard_info_page.ui.h \
    ui/dic_wizard_letters_def_page.ui.h \
    ui/dic_wizard_conclusion_page.ui.h \
    game_signals.moc.cpp \
    play_model.moc.cpp \
    tile_widget.moc.cpp \
    rack_widget.moc.cpp \
    timer_widget.moc.cpp \
    tile_layout.moc.cpp \
    validator_factory.moc.cpp \
    misc_helpers.moc.cpp \
    custom_popup.moc.cpp \
    players_table_helper.moc.cpp \
    fav_players.moc.cpp \
    hints_dialog.moc.cpp \
    new_game.moc.cpp \
    dic_tools_widget.moc.cpp \
    arbitration_widget.moc.cpp \
    arbit_assignments.moc.cpp \
    bag_widget.moc.cpp \
    score_widget.moc.cpp \
    dic_wizard.moc.cpp \
    board_widget.moc.cpp \
    history_widget.moc.cpp \
    stats_widget.moc.cpp \
    play_word_mediator.moc.cpp \
    player_widget.moc.cpp \
    tables_dialog.moc.cpp \
    topping_widget.moc.cpp \
    training_widget.moc.cpp \
    prefs_dialog.moc.cpp \
    update_checker.moc.cpp \
    aux_window.moc.cpp \
    main_window.moc.cpp \
    resources.cpp

BUILT_SOURCES = $(nodist_eliot_SOURCES)

MOSTLYCLEANFILES = $(nodist_eliot_SOURCES)

eliot_LDADD = ../game/libgame.a ../dic/libdic.a @QT_LIBS@ @LIBINTL@ @LIBCONFIG_LIBS@ @ARABICA_LIBS@ @EXPAT_LIBS@
# Needed for proper stack trace handling
eliot_LDFLAGS = -rdynamic

if WITH_LOGGING
eliot_LDADD += @LOG4CXX_LIBS@
endif

# Generate a cpp file from the resources
resources.cpp: eliot.qrc $(RESOURCES)
	$(RCC) -o $@ $<

# Generate the implementation file from the header
%.moc.cpp: %.h
	$(MOC) -o $@ $<

# Generate the header from the .ui file
# We want to translate the string with gettext, not Qt own way
%.ui.h: %.ui
	mkdir -p ui
	rm -f $@ $@.tmp
	echo "// Note: Lines below generated by Eliot build system" > $@.tmp
	echo "#include \"qtcommon.h\"" >> $@.tmp
	echo "#define Q_(a,b) _q(a)" >> $@.tmp
	$(UIC) -tr "Q_" $< >> $@.tmp
	sed 's/Q_(\"_(\\\"\(.*\)\\\")"/Q_("\1"/' $@.tmp >$@
	rm -f $@.tmp

# Embed the icon in the win32 binary, see http://doc.trolltech.com/4.4/appicon.html
if WIN32_BUILD
BUILT_SOURCES += win32icon.rc
eliot_SOURCES += win32icon.rc
eliot_LDADD += win32icon.o

win32icon.o: win32icon.rc
	i586-mingw32msvc-windres -o $@ -i $<
endif

# This target is out of the conditional, so that make distcheck is happy
win32icon.rc: images/eliot.ico
	echo "IDI_ICON1 ICON DISCARDABLE \"$(top_srcdir)/qt/images/eliot.ico\"" > $@

endif

